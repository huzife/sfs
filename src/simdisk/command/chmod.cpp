#include "simdisk/disk-manager.h"

static const struct option long_options[] = {
	{nullptr, no_argument, nullptr, 0}};

int DiskManager::chmod(int argc, char *argv[], int sid) {
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
	std::string perm;

	if (optind > argc - 2) {
		writeOutput("chmod: missing operand", sid);
		return -1;
	}

	perm = argv[optind];
	path = argv[optind + 1];

	if (perm.size() != 2 || perm[0] < '0' || perm[0] > '7' || perm[1] < '0' || perm[1] > '7') {
		writeOutput("chmod: invalid permission: " + perm, sid);
		return -1;
	}

	auto dentry = getDirectoryEntry(path, sid);
	if (dentry == nullptr) return -1;

	auto inode = getIndexNode(dentry->m_inode);

	if (m_shells[sid].m_user != 0 && m_shells[sid].m_user != inode->m_owner) {
		writeOutput("chmod: bad ownership", sid);
		return -1;
	}

	inode->m_owner_permission = static_cast<Permission>(perm[0] - '0');
	inode->m_other_permission = static_cast<Permission>(perm[1] - '0');
	writeIndexNode(dentry->m_inode, inode);

	return 0;
}