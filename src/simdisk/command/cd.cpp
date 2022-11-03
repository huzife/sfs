#include "simdisk/disk-manager.h"

static const struct option long_options[] = {
	{nullptr, no_argument, nullptr, 0}};

int DiskManager::cd(int argc, char *argv[]) {
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
		std::cerr << "cd: missing operand" << std::endl;
		return -1;
	}

	path = argv[optind];

	auto dentry = getDirectoryEntry(path);
	if (dentry == nullptr) return -1;

	auto inode = getIndexNode(dentry->m_inode);
	if (inode->m_type != FileType::DIRECTORY && inode->m_type != FileType::LINK) {
		std::cerr << dentry->m_filename << ": Not a directory" << std::endl;
		return -1;
	}

	m_cwd.m_path = getPath(m_cwd.m_path, path);
	m_cwd.m_dentry = dentry;
	m_cwd.m_inode = inode;
	std::cout << "current directory: " << m_cwd.m_path << std::endl;

	return 0;
}