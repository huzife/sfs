#include "simdisk/disk-manager.h"

static const struct option long_options[] = {
	{nullptr, no_argument, nullptr, 0}};

int DiskManager::md(int argc, char *argv[], int sid) {
	optind = 0;
	// get options
	int ch;
	while ((ch = getopt_long(argc, argv, "", long_options, nullptr)) != -1) {
		switch (ch) {
		default:
			std::string out = "invalid option '" + std::string(argv[optind - 1]) + "'\n";
			if (sid != 0)
				writeOutput(out, sid);
			return -1;
		}
	}

	std::string path;
	if (optind == argc) {
		writeOutput("md: missing operand", sid);
		return -1;
	}

	path = argv[optind++];

	// get the names of parent directory and new directory to be created
	int end = path.find_last_not_of('/');
	int pos = path.find_last_of('/', end);
	std::string parent_dir, name;
	if (pos == UINT64_MAX) {
		parent_dir = ".";
		name = path.substr(0, end + 1);
	}
	else {
		parent_dir = pos == 0 ? "/" : path.substr(0, pos);
		name = path.substr(pos + 1, end - pos);
	}

	// check filename size, maxsize == 255 byte
	if (name.size() > m_super_block.m_filename_maxbytes) {
		std::string out("md: cannot create directory '" + name + "': File name too long");
		writeOutput(out, sid);
		return -1;
	}

	if (name == "." || name == "..") {
		std::string out("md: cannot create directory '" + name + "': File exists");
		writeOutput(out, sid);
		return -1;
	}

	// get parent dir
	auto dentry = getDirectoryEntry(parent_dir, sid);
	if (dentry == nullptr) return -1;

	open(dentry->m_inode, "w", sid);

	auto inode = getIndexNode(dentry->m_inode);

	if (!checkPermission(Permission::WRITE, inode, m_shells[sid].m_user)) {
		writeOutput("md: cannot create directory in '" + dentry->m_filename + "': Permission denied", sid);
		close(dentry->m_inode, "w");
		return -1;
	}

	auto file = std::dynamic_pointer_cast<DirFile>(getFile(inode));

	// check if 'name' is exist
	if (file->m_dirs.find(name) != file->m_dirs.end()) {
		std::string out("md: cannot create directory '" + name + "': File exists");
		writeOutput(out, sid);
		close(dentry->m_inode, "w");
		return -1;
	}

	// create a new dentry
	DirectoryEntry new_dentry;
	new_dentry.m_filename = name;
	new_dentry.m_inode = allocIndexNode();
	new_dentry.m_name_len = name.size();
	new_dentry.m_rec_len = 7 + new_dentry.m_name_len;

	// insert new dir into parent dir and modify parent dir
	int ret = expandFileSize(inode, new_dentry.m_rec_len);
	if (ret == -1) {
		writeOutput("error: no enough blocks", sid);
		close(dentry->m_inode, "w");
		return -1;
	}

	inode->m_subs++;
	file->m_size = inode->m_size;
	file->m_subs = inode->m_subs;
	file->m_dirs[new_dentry.m_filename] = new_dentry;
	writeIndexNode(dentry->m_inode, inode);
	writeFile(inode, file);

	// create index node
	std::shared_ptr<IndexNode> new_inode = std::make_shared<IndexNode>(FileType::DIRECTORY, m_shells[sid].m_user, allocFileBlock(1));
	writeIndexNode(new_dentry.m_inode, new_inode);

	// create file
	auto new_file = std::make_shared<DirFile>(new_inode->m_size, new_inode->m_subs);
	new_file->m_parent.m_inode = dentry->m_inode;
	new_file->m_current.m_inode = new_dentry.m_inode;
	writeFile(new_inode, new_file);

	close(dentry->m_inode, "w");

	return 0;
}