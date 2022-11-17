#include "simdisk/disk-manager.h"

static const struct option long_options[] = {
	{nullptr, no_argument, nullptr, 0}};

int DiskManager::rd(int argc, char *argv[], int sid, int inode_id, int block_id) {
	std::string path, filename;
	std::shared_ptr<IndexNode> inode;
	// get options
	if (argc != 0) {
		assert(inode_id == -1 && block_id == -1);
		optind = 0;
		int ch;
		while ((ch = getopt_long(argc, argv, "", long_options, nullptr)) != -1) {
			switch (ch) {
			default:
				return -1;
			}
		}

		if (optind == argc) {
			writeOutput("rd: missing operand", sid);
			return -1;
		}
		path = argv[optind];

		auto dentry = getDirectoryEntry(path, sid);
		if (dentry == nullptr) return -1;
		inode_id = dentry->m_inode;
		filename = dentry->m_filename;

		inode = getIndexNode(inode_id);
		if (inode->m_type != FileType::DIRECTORY) {
			std::string out("rd: cannot delete '" + filename + "': Not a directory");
			writeOutput(out, sid);
			return -1;
		}
		block_id = inode->m_location;
	}

	// remove all sub-directories and files recursively
	if (inode == nullptr) inode = getIndexNode(inode_id);
	auto file = std::dynamic_pointer_cast<DirFile>(getFile(inode));
	for (auto &[name, d] : file->m_dirs) {
		auto sub_inode = getIndexNode(d.m_inode);
		if (sub_inode->m_type == FileType::DIRECTORY)
			rd(0, nullptr, sid, d.m_inode, sub_inode->m_location);
		else
			del(0, nullptr, sid, d.m_inode, sub_inode->m_location);
	}

	freeFlieBlock(block_id);
	freeIndexNode(inode_id);

	// if it's recursion of rd(), we don't have to update the infomation of parent directory
	if (argc == 0) return 0;

	auto parent_dentry = getDirectoryEntry(getPath(m_shells[sid].m_path, path + "/.."), sid);
	if (parent_dentry == nullptr) assert(false); // unreachable;	
	auto parent_inode = getIndexNode(parent_dentry->m_inode);
	auto parent_file = std::dynamic_pointer_cast<DirFile>(getFile(parent_inode));

	parent_file->m_dirs.erase(filename);
	parent_inode->m_subs--;
	writeOutput("delete " + path, sid);

	writeIndexNode(parent_dentry->m_inode, parent_inode);
	writeFile(parent_inode, parent_file);

	return 0;
}