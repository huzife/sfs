#include "simdisk/disk-manager.h"

static const struct option long_options[] = {
    {nullptr, no_argument, nullptr, 0}};

int DiskManager::md(int argc, char *argv[]) {
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
        std::cerr << "md: missing operand" << std::endl;
        return -1;
    }

    path = argv[optind];

    int end = path.find_last_not_of('/');
    int pos = path.find_last_of('/', end);
    std::string parent_dir, name;
    if (pos == UINT64_MAX) {
        parent_dir = ".";
        name = path.substr(0, end + 1);
    }
    else {
        parent_dir = path.substr(0, pos);
        name = path.substr(pos + 1, end - pos);
    }

    if (name.size() > m_super_block.m_filename_maxbytes) {
        std::cerr << "md: cannot create directory '" + name + "': File name too long" << std::endl;
        return -1;
    }

    if (name == "." || name == "..") {
        std::cerr << "md: cannot create directory '" + name + "': File exists" << std::endl;
        return -1;
    }
    auto dentry = getDirectoryEntry(parent_dir);
    if (dentry == nullptr) return -1;
    auto inode = getIndexNode(dentry->m_inode);
    auto file = std::dynamic_pointer_cast<DirFile>(getFile(inode));
    for (auto d : file->m_dirs) {
        if (name == d.m_filename) {
            std::cerr << "md: cannot create directory '" + name + "': File exists" << std::endl;
            return -1;
        }
    }

    // create a new dentry
    DirectoryEntry new_dentry;
    new_dentry.m_filename = name;
    new_dentry.m_inode = allocIndexNode();
    new_dentry.m_name_len = name.size();
    new_dentry.m_rec_len = 7 + new_dentry.m_name_len;

    // insert new dir into parent dir and modify parent dir
    int ret = expandFileSize(inode, new_dentry.m_rec_len);
    inode->m_subs++;
    file->m_size = inode->m_size;
    file->m_subs = inode->m_subs;
    file->m_dirs.emplace_back(new_dentry);
    writeIndexNode(dentry->m_inode, inode);
    writeFile(inode, file);

    // create index node
    std::shared_ptr<IndexNode> new_inode = std::make_shared<IndexNode>();
    new_inode->m_type = FileType::DIRECTORY;
    new_inode->m_owner_permission = static_cast<Permission>(7);
    new_inode->m_other_permission = static_cast<Permission>(5);
    new_inode->m_owner = 0;
    new_inode->m_size = DiskManager::block_size;
    new_inode->m_subs = 2;
    new_inode->m_blocks = 1;
    new_inode->m_location = allocFileBlock(1);
    new_inode->m_count = 1;
    auto now = std::chrono::system_clock::now();
    new_inode->m_create_time = now;
    new_inode->m_access_time = now;
    new_inode->m_modify_time = now;
    new_inode->m_change_time = now;
    writeIndexNode(new_dentry.m_inode, new_inode);

    // create file
    auto new_file = std::make_shared<DirFile>(new_inode->m_size, new_inode->m_subs);
    new_file->m_parent.m_inode = dentry->m_inode;
    new_file->m_current.m_inode = new_dentry.m_inode;
    writeFile(new_inode, new_file);

    return 0;
}