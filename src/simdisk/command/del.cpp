#include "simdisk/disk-manager.h"

static const struct option long_options[] = {
    {nullptr, no_argument, nullptr, 0}};

int DiskManager::del(int argc, char *argv[], int inode_id, int block_id) {
    if (argc == 0) {
        assert(inode_id != -1 && block_id != -1);
        freeFlieBlock(block_id);
        freeIndexNode(inode_id);
        return 0;
    }

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
        std::cerr << "del: missing operand" << std::endl;
        return -1;
    }

    path = argv[optind];

    auto dentry = getDirectoryEntry(path);
    if (dentry == nullptr) return -1;

    auto inode = getIndexNode(dentry->m_inode);
    if (inode->m_type == FileType::DIRECTORY) {
        std::cerr << "del: cannot delete '" << dentry->m_filename << "': Is a directory" << std::endl;
        return -1;
    }

    freeFlieBlock(inode->m_location);
    freeIndexNode(dentry->m_inode);

    auto parent_dentry = getDirectoryEntry(getPath(m_cwd.m_path, path + "/.."));
    auto parent_inode = getIndexNode(parent_dentry->m_inode);
    auto parent_file = std::dynamic_pointer_cast<DirFile>(getFile(parent_inode));

    parent_file->m_dirs.erase(dentry->m_filename);
    parent_inode->m_subs--;
    std::cout << "delete " << path << std::endl;
    
    writeIndexNode(parent_dentry->m_inode, parent_inode);
    writeFile(parent_inode, parent_file);

    return 0;
}