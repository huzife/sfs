#include "simdisk/disk-manager.h"

static const struct option long_options[] = {
	{"sub-directories", no_argument, nullptr, 's'},
	{nullptr, no_argument, nullptr, 0}};

int DiskManager::dir(int argc, char *argv[], int sid) {
	optind = 0;
	// get options
	int ch;
	bool sub_directories = false;
	while ((ch = getopt_long(argc, argv, "s", long_options, nullptr)) != -1) {
		switch (ch) {
		case 's': // -s: show all sub directories
			sub_directories = true;
			break;
		default:
			std::string out = "invalid option '" + std::string(argv[optind - 1]) + "'\n";
			if (sid != 0)
				writeOutput(out, sid);
			return 1;
		}
	}

	std::string path;
	std::shared_ptr<DirectoryEntry> dentry;
	if (optind == argc) {
		path = m_shells[sid].m_path;
		dentry = m_shells[sid].m_dentry;
	}
	else {
		path = argv[optind];
		dentry = getDirectoryEntry(path, sid);
	}

	if (dentry == nullptr) return -1;

	open(dentry->m_inode, "r", sid);

	auto inode = getIndexNode(dentry->m_inode);

	if (!checkPermission(Permission::READ, inode, m_shells[sid].m_user)) {
		writeOutput("dir: cannot open directory '" + dentry->m_filename + "': Permission denied", sid);
		close(dentry->m_inode, "r");
		return -1;
	}

	std::vector<std::shared_ptr<IndexNode>> dirs;
	std::vector<std::string> names;

	// get the index nodes of dirs
	if (sub_directories) {
		auto file = std::dynamic_pointer_cast<DirFile>(getFile(inode));
		dirs.emplace_back(getIndexNode(file->m_current.m_inode));
		names.emplace_back(".");
		dirs.emplace_back(getIndexNode(file->m_parent.m_inode));
		names.emplace_back("..");
		for (auto &[name, d] : file->m_dirs) {
			dirs.emplace_back(getIndexNode(d.m_inode));
			names.emplace_back(d.m_filename);
		}
	}
	else {
		dirs.emplace_back(inode);
		names.emplace_back(path);
	}

	close(dentry->m_inode, "r");

	// format output
	// get max length of m_subs
	int max_subs_len =
		std::to_string((*std::max_element(dirs.begin(), dirs.end(),
										  [](std::shared_ptr<IndexNode> a, std::shared_ptr<IndexNode> b) {
											  return a->m_subs < b->m_subs;
										  }))
						   ->m_subs)
			.size()
		+ 1;

	// get max length of m_owner
	int max_owner_len =
		getUserName((*std::max_element(dirs.begin(), dirs.end(),
										  [this](std::shared_ptr<IndexNode> a, std::shared_ptr<IndexNode> b) {
											  return getUserName(a->m_owner).size() < getUserName(b->m_owner).size();
										  }))
						   ->m_owner)
			.size()
		+ 1;

	// get max length of m_size
	int max_size_len =
		std::to_string((*std::max_element(dirs.begin(), dirs.end(),
										  [](std::shared_ptr<IndexNode> a, std::shared_ptr<IndexNode> b) {
											  return a->m_size < b->m_size;
										  }))
						   ->m_size)
			.size()
		+ 1;

	// print
	std::string out;
	for (int i = 0; i < dirs.size(); i++) {
		out += IndexNode::FileTypeToStr(dirs[i]->m_type);
		out += IndexNode::PermissionToStr(dirs[i]->m_owner_permission);
		out += IndexNode::PermissionToStr(dirs[i]->m_other_permission);
		out += fill(std::to_string(dirs[i]->m_subs), max_subs_len, 'l');
		out += fill(getUserName(dirs[i]->m_owner), max_owner_len, 'l');
		out += fill(std::to_string(dirs[i]->m_size), max_size_len, 'l');
		out += " " + timeToDate(dirs[i]->m_modify_time);
		out += " " + names[i];
		out += "\n";
	}
	writeOutput(out, sid);

	return 0;
}