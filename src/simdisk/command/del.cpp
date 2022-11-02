#include "simdisk/disk-manager.h"

static const struct option long_options[] = {
    {nullptr, no_argument, nullptr, 0}};

int DiskManager::del(int argc, char *argv[]) {
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

    auto p_dentry = getDirectoryEntry(getPath(getPath(m_cwd.m_path, path), ".."));
    auto p_inode = getIndexNode(p_dentry->m_inode);
    auto p_file = std::dynamic_pointer_cast<DirFile>(getFile(p_inode));

    for (auto it = p_file->m_dirs.begin(); it != p_file->m_dirs.end(); it++) {
        if (it->m_filename == dentry->m_filename) {
            p_file->m_dirs.erase(it);
            p_inode->m_subs--;
            std::cout << "delete " << path << std::endl;
            break;
        }
    }
    writeIndexNode(p_dentry->m_inode, p_inode);
    writeFile(p_inode, p_file);

    return 0;
}