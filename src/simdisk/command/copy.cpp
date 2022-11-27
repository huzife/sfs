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

	std::string src_text;
	std::string src_name;

	std::shared_ptr<DataFile> src_file;
	std::shared_ptr<DirFile> dst_file;

	if (host_src && host_dst) {
		writeOutput("copy: source and destination cannot both be a host path", sid);
		return -1;
	}
	else if (host_src) {
		std::ifstream ifs(src.substr(6), std::ios::in);
		if (!ifs.is_open()) {
			writeOutput("copy: cannot open file '" + src + "'", sid);
			return -1;
		}

		src_text = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
		src_name = src.substr(src.find_last_of('/') + 1);

		ifs.close();

		// read dst dir
		auto dst_dentry = getDirectoryEntry(dst, sid);
		if (dst_dentry == nullptr) return -1;
		open(dst_dentry->m_inode, "w", sid);
		auto dst_inode = getIndexNode(dst_dentry->m_inode);

		if (dst_inode->m_type != FileType::DIRECTORY && dst_inode->m_type != FileType::LINK) {
			std::string out("copy: cannot copy to '" + dst_dentry->m_filename + "': Not a directory");
			writeOutput(out, sid);
			close(dst_dentry->m_inode, "w");
			return -1;
		}

		if (!checkPermission(Permission::WRITE, dst_inode, m_shells[sid].m_user)) {
			writeOutput("copy: cannot create file in '" + dst_dentry->m_filename + "': Permission denied", sid);
			close(dst_dentry->m_inode, "w");
			return -1;
		}

		dst_file = std::dynamic_pointer_cast<DirFile>(getFile(dst_inode));

		if (dst_file->m_dirs.find(src_name) != dst_file->m_dirs.end()) {
			std::string out("copy: cannot copy to '" + src_name + "': File exist");
			writeOutput(out, sid);
			close(dst_dentry->m_inode, "w");
			return -1;
		}

		// create a new dentry
		DirectoryEntry new_dentry;
		new_dentry.m_inode = allocIndexNode();
		new_dentry.m_filename = src_name;
		new_dentry.m_name_len = src_name.size();
		new_dentry.m_rec_len = 7 + new_dentry.m_name_len;

		// insert new dir into parent dir and modify parent dir
		int ret = expandFileSize(dst_inode, new_dentry.m_rec_len);
		if (ret == -1) {
			writeOutput("error: no enough blocks", sid);
			close(dst_dentry->m_inode, "w");
			return -1;
		}

		dst_inode->m_subs++;
		dst_file->m_size = dst_inode->m_size;
		dst_file->m_subs = dst_inode->m_subs;
		dst_file->m_dirs[new_dentry.m_filename] = new_dentry;
		writeIndexNode(dst_dentry->m_inode, dst_inode);
		writeFile(dst_inode, dst_file);

		// create index node
		int size = src_text.size();
		int blocks = (size - 1) / DiskManager::block_size + 1;
		int loc = allocFileBlock(blocks);
		std::shared_ptr<IndexNode> new_inode = std::make_shared<IndexNode>(FileType::NORMAL, m_shells[sid].m_user, loc, size, blocks);
		writeIndexNode(new_dentry.m_inode, new_inode);

		// create file
		std::shared_ptr<char[]> buffer(new char[blocks * DiskManager::block_size]);
		memcpy(buffer.get(), src_text.data(), size);
		src_file = std::make_shared<DataFile>(blocks * DiskManager::block_size);
		src_file->load(buffer);
		writeFile(new_inode, src_file);

		close(dst_dentry->m_inode, "w");
	}
	else if (host_dst) {
		// copy from simdisk to simdisk
		// read src file

		auto src_dentry = getDirectoryEntry(src, sid);
		if (src_dentry == nullptr) return -1;
		open(src_dentry->m_inode, "r", sid);
		auto src_inode = getIndexNode(src_dentry->m_inode);

		if (src_inode->m_type == FileType::DIRECTORY) {
			writeOutput("copy: cannot copy '" + src_dentry->m_filename + "': Is a directory", sid);
			close(src_dentry->m_inode, "r");
			return -1;
		}

		if (!checkPermission(Permission::READ, src_inode, m_shells[sid].m_user)) {
			writeOutput("copy: cannot open file '" + src_dentry->m_filename + "': Permission denied", sid);
			close(src_dentry->m_inode, "r");
			return -1;
		}

		src_file = std::dynamic_pointer_cast<DataFile>(getFile(src_inode));
		src_text = std::string(src_file->dump().get(), src_inode->m_size);

		std::string path = dst.substr(6) + "/" + src_dentry->m_filename;

		std::ofstream ofs(path, std::ios::out);
		if (!ofs.is_open()) {
			writeOutput("copy: cannot copy file to '" + src + "'", sid);
			close(src_dentry->m_inode, "r");
			return -1;
		}
		ofs << src_text;
		ofs.close();

		close(src_dentry->m_inode, "r");
	}
	else {
		// copy from simdisk to simdisk
		// read src file

		auto src_dentry = getDirectoryEntry(src, sid);
		if (src_dentry == nullptr) return -1;
		open(src_dentry->m_inode, "r", sid);
		auto src_inode = getIndexNode(src_dentry->m_inode);

		if (src_inode->m_type == FileType::DIRECTORY) {
			std::string out("copy: cannot copy '" + src_dentry->m_filename + "': Is a directory");
			writeOutput(out, sid);
			close(src_dentry->m_inode, "r");
			return -1;
		}

		if (!checkPermission(Permission::READ, src_inode, m_shells[sid].m_user)) {
			writeOutput("copy: cannot open file '" + src_dentry->m_filename + "': Permission denied", sid);
			close(src_dentry->m_inode, "r");
			return -1;
		}

		src_file = std::dynamic_pointer_cast<DataFile>(getFile(src_inode));

		// read dst dir
		auto dst_dentry = getDirectoryEntry(dst, sid);
		if (dst_dentry == nullptr) return -1;
		open(dst_dentry->m_inode, "w", sid);
		auto dst_inode = getIndexNode(dst_dentry->m_inode);

		if (dst_inode->m_type != FileType::DIRECTORY && dst_inode->m_type != FileType::LINK) {
			std::string out("copy: cannot copy to '" + dst_dentry->m_filename + "': Not a directory");
			writeOutput(out, sid);
			close(src_dentry->m_inode, "r");
			close(dst_dentry->m_inode, "w");
			return -1;
		}

		if (!checkPermission(Permission::WRITE, dst_inode, m_shells[sid].m_user)) {
			writeOutput("copy: cannot create file in '" + dst_dentry->m_filename + "': Permission denied", sid);
			close(src_dentry->m_inode, "r");
			close(dst_dentry->m_inode, "w");
			return -1;
		}

		dst_file = std::dynamic_pointer_cast<DirFile>(getFile(dst_inode));

		if (dst_file->m_dirs.find(src_dentry->m_filename) != dst_file->m_dirs.end()) {
			std::string out("copy: cannot copy to '" + dst_dentry->m_filename + "': File exist");
			close(src_dentry->m_inode, "r");
			close(dst_dentry->m_inode, "w");
			writeOutput(out, sid);
			return -1;
		}

		//
		//
		// create a new dentry
		DirectoryEntry new_dentry(*src_dentry);
		new_dentry.m_inode = allocIndexNode();

		// insert new dir into parent dir and modify parent dir
		int ret = expandFileSize(dst_inode, new_dentry.m_rec_len);
		if (ret == -1) {
			writeOutput("error: no enough blocks", sid);
			close(src_dentry->m_inode, "r");
			close(dst_dentry->m_inode, "w");
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
		new_inode->m_location = allocFileBlock(new_inode->m_blocks);
		auto now = std::chrono::system_clock::now();
		new_inode->m_create_time = now;
		new_inode->m_access_time = now;
		new_inode->m_modify_time = now;
		new_inode->m_change_time = now;
		writeIndexNode(new_dentry.m_inode, new_inode);

		// create file
		writeFile(new_inode, src_file);

		close(src_dentry->m_inode, "r");
		close(dst_dentry->m_inode, "w");
	}
	return 0;
}