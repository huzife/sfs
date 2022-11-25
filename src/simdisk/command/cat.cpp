#include "simdisk/disk-manager.h"

static const struct option long_options[] = {
	{nullptr, no_argument, nullptr, 0}};

int DiskManager::cat(int argc, char *argv[], int sid) {
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
		writeOutput("cat: missing operand", sid);
		return -1;
	}

	path = argv[optind];

	auto dentry = getDirectoryEntry(path, sid);
	if (dentry == nullptr) return -1;

	open(dentry->m_inode, "r", sid);
	std::this_thread::sleep_for(std::chrono::seconds(3)); // for debug

	auto inode = getIndexNode(dentry->m_inode);
	if (inode->m_type == FileType::DIRECTORY || inode->m_type == FileType::LINK) {
		writeOutput("cat: " + dentry->m_filename + ": Is a directory", sid);
		close(dentry->m_inode, "r");
		return -1;
	}

	if (!checkPermission(Permission::READ, inode, m_shells[sid].m_user)) {
		writeOutput("cat: cannot open file '" + dentry->m_filename + "': Permission denied", sid);
		close(dentry->m_inode, "r");
		return -1;
	}

	auto file = std::dynamic_pointer_cast<DataFile>(getFile(inode));
	std::string out(file->m_data.get(), inode->m_size);
	writeOutput(out, sid);

	close(dentry->m_inode, "r");

	return 0;
}