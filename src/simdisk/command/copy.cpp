#include "simdisk/disk-manager.h"

static const struct option long_options[] = {
	{nullptr, no_argument, nullptr, 0}};

int DiskManager::copy(int argc, char *argv[], int sid) {
	// get options
	int ch;
	optind = 0;
	while ((ch = getopt_long(argc, argv, "", long_options, nullptr)) != -1) {
		switch (ch) {
		default:
			std::string out = "invalid option '" + std::string(argv[optind - 1]) + "'\n";
			if (sid != 0)
				writeOutput(out, sid);
			return -1;
		}
	}

	if (optind == argc) {
		writeOutput("copy: missing file operand", sid);
		return -1;
	}
	else if (optind == argc - 1) {
		std::string out = "copy: missing destination file operand after '";
		out += argv[optind];
		out += "'";
		writeOutput(out, sid);
		return -1;
	}

	std::string src = argv[optind];		// source file
	std::string dst = argv[optind + 1]; // destination

	// TODO: implements cross-filesystem copy between simdisk and host
	bool host_src = src.substr(0, 6) == "<host>";
	bool host_dst = dst.substr(0, 6) == "<host>";

	// copy from simdisk to simdisk
	auto src_dentry = getDirectoryEntry(src, sid);
	auto dst_dentry = getDirectoryEntry(dst, sid);
	if (src_dentry == nullptr || dst_dentry == nullptr) return -1;

	auto src_inode = getIndexNode(src_dentry->m_inode);
	auto dst_inode = getIndexNode(dst_dentry->m_inode);
	if (src_inode->m_type == FileType::DIRECTORY) {
		std::string out("copy: cannot copy '" + src_dentry->m_filename + "': Is a directory");
		writeOutput(out, sid);
		return -1;
	}
	else if (dst_inode->m_type != FileType::DIRECTORY && dst_inode->m_type != FileType::LINK) {
		std::string out("copy: cannot copy to '" + dst_dentry->m_filename + "': Not a directory");
		writeOutput(out, sid);
		return -1;
	}

	auto src_file = std::dynamic_pointer_cast<DataFile>(getFile(src_inode));
	auto dst_file = std::dynamic_pointer_cast<DirFile>(getFile(dst_inode));

	if (dst_file->m_dirs.find(src_dentry->m_filename) != dst_file->m_dirs.end()) {
		std::string out("copy: cannot copy to '" + dst_dentry->m_filename + "': File exist");
		writeOutput(out, sid);
		return -1;
	}

	// create a new dentry
	DirectoryEntry new_dentry(*src_dentry);
	new_dentry.m_inode = allocIndexNode();

	// insert new dir into parent dir and modify parent dir
	int ret = expandFileSize(dst_inode, new_dentry.m_rec_len);
	if (ret == -1) {
		writeOutput("error: no enough blocks", sid);
		return -1;
	}

	dst_inode->m_subs++;
	dst_file->m_size = dst_inode->m_size;
	dst_file->m_subs = dst_inode->m_subs;
	dst_file->m_dirs[new_dentry.m_filename] = new_dentry;
	writeIndexNode(dst_dentry->m_inode, dst_inode);
	writeFile(dst_inode, dst_file);

	// create index node
	std::shared_ptr<IndexNode> new_inode = std::make_shared<IndexNode>(*src_inode);
	// new_inode->m_owner = 0;  // owner should be changed to the new owner
	new_inode->m_location = allocFileBlock(new_inode->m_blocks);
	auto now = std::chrono::system_clock::now();
	new_inode->m_create_time = now;
	new_inode->m_access_time = now;
	new_inode->m_modify_time = now;
	new_inode->m_change_time = now;
	writeIndexNode(new_dentry.m_inode, new_inode);

	// create file
	writeFile(new_inode, src_file);

	return 0;
}