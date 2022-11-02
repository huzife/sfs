#include "simdisk/disk-manager.h"
#include <getopt.h>

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
        std::cerr << "missing operand" << std::endl;
        return -1;
    }

    path = argv[optind];

    int start = path.back() == '/' ? path.size() - 2 : path.size() - 1;
    int pos = path.find_last_of('/', start);
    std::string parent_dir, new_dir;
    if (pos == UINT64_MAX) {
        parent_dir = ".";
        new_dir = path;
    }
    else {
        parent_dir = path.substr(0, pos);
        new_dir = path.substr(pos + 1, start - pos + 1);
    }

    if (new_dir.size() > m_super_block.m_filename_maxbytes) {
        std::cerr << "md: cannot create directory '" + new_dir + "': File name too long" << std::endl;
        return -1;
    }

    if (new_dir == "." || new_dir == "..") {
        std::cerr << "md: cannot create directory '" + new_dir + "': File exists" << std::endl;
        return -1;
    }

    auto dentry = getDirectoryEntry(parent_dir);
    if (dentry == nullptr) return -1;
    auto inode = getIndexNode(dentry->m_inode);
    auto file = std::dynamic_pointer_cast<DirFile>(getFile(inode));
    for (auto d : file->m_dirs) {
        if (new_dir == d.m_filename) {
            std::cerr << "md: cannot create directory '" + new_dir + "': File exists" << std::endl;
            return -1;
        }
    }

    // create a new dentry
    DirectoryEntry dir;
    dir.m_filename = new_dir;
    dir.m_inode = allocIndexNode();
    dir.m_name_len = new_dir.size();
    dir.m_rec_len = 7 + dir.m_name_len;

    // insert new dir into parent dir and modify parent dir
    int ret = expandFileSize(inode, dir.m_rec_len);
    inode->m_size += ret * DiskManager::block_size;
    inode->m_subs++;
    file->m_size = inode->m_size;
    file->m_subs = inode->m_subs;
    file->m_dirs.emplace_back(dir);
    writeIndexNode(dentry->m_inode, inode);
    writeFile(inode, file);

    // create index node
    std::shared_ptr<IndexNode> new_inode = std::make_shared<IndexNode>();
    new_inode->m_type = FileType::DIRECTORY;
    new_inode->m_owner_permission = static_cast<Permission>(7);
    new_inode->m_other_permission = static_cast<Permission>(5);
    new_inode->m_owner = 0;
    new_inode->m_size = DiskManager::block_size;
    new_inode->m_location = allocFileBlock(1);
    new_inode->m_count = 1;
    new_inode->m_subs = 2;
    auto now = std::chrono::system_clock::now();
    new_inode->m_create_time = now;
    new_inode->m_access_time = now;
    new_inode->m_modify_time = now;
    new_inode->m_change_time = now;
    writeIndexNode(dir.m_inode, new_inode);

    // create file
    auto new_file = std::make_shared<DirFile>(new_inode->m_size, new_inode->m_subs);
    new_file->m_parent = *dentry;
    new_file->m_current = dir;
    writeFile(new_inode, new_file);

    return 0;
}