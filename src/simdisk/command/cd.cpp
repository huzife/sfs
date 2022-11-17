#include "simdisk/disk-manager.h"

static const struct option long_options[] = {
	{nullptr, no_argument, nullptr, 0}};

int DiskManager::cd(int argc, char *argv[], int sid) {
	optind = 0;
	// get options
	int ch;
	while ((ch = getopt_long(argc, argv, "", long_options, nullptr)) != -1) {
		switch (ch) {
		default:
			return -1;
		}
	}

	std::string path;
	if (optind == argc) {
		writeOutput("cd: missing operand", sid);
		return -1;
	}

	path = argv[optind];

	auto dentry = getDirectoryEntry(path, sid);
	if (dentry == nullptr) return -1;

	auto inode = getIndexNode(dentry->m_inode);
	if (inode->m_type != FileType::DIRECTORY && inode->m_type != FileType::LINK) {
		writeOutput(dentry->m_filename + ": Not a directory", sid);
		return -1;
	}

	auto &shell = m_shells[sid];
	shell.m_path = getPath(shell.m_path, path);
	shell.m_dentry = dentry;
	shell.m_inode = inode;
	writeOutput("current directory: " + shell.m_path, sid);

	return 0;
}