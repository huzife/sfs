#include "simdisk/disk-manager.h"

static const struct option long_options[] = {
	{nullptr, no_argument, nullptr, 0}};

int DiskManager::del(int argc, char *argv[], int sid, int inode_id, int block_id) {
	if (argc == 0) {
		assert(inode_id != -1 && block_id != -1);
		freeFlieBlock(block_id);
		freeIndexNode(inode_id);
		return 0;
	}

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
		writeOutput("del: missing operand", sid);
		return -1;
	}

	path = argv[optind];

	auto dentry = getDirectoryEntry(path, sid);
	if (dentry == nullptr) return -1;

	auto inode = getIndexNode(dentry->m_inode);
	if (inode->m_type == FileType::DIRECTORY) {
		std::string out("del: cannot delete '" + dentry->m_filename + "': Is a directory");
		writeOutput(out, sid);
		return -1;
	}

	if (m_shells[sid].m_user != 0 && m_shells[sid].m_user != inode->m_owner) {
		writeOutput("del: cannot delete file '" + dentry->m_filename + "': Permission denied", sid);
		return -1;
	}

	freeFlieBlock(inode->m_location);
	freeIndexNode(dentry->m_inode);

	auto parent_dentry = getDirectoryEntry(getPath(m_shells[sid].m_path, path + "/.."), sid);
	if (parent_dentry == nullptr) assert(false); // unreachable;
	
	open(parent_dentry->m_inode, "w", sid);

	auto parent_inode = getIndexNode(parent_dentry->m_inode);

	if (!checkPermission(Permission::WRITE, parent_inode, m_shells[sid].m_user)) {
		writeOutput("del: cannot delete file in '" + parent_dentry->m_filename + "': Permission denied", sid);
		return -1;
	}

	auto parent_file = std::dynamic_pointer_cast<DirFile>(getFile(parent_inode));

	parent_file->m_dirs.erase(dentry->m_filename);
	parent_inode->m_subs--;
	writeOutput("delete " + path, sid);

	writeIndexNode(parent_dentry->m_inode, parent_inode);
	writeFile(parent_inode, parent_file);

	close(parent_dentry->m_inode, "w");

	return 0;
}