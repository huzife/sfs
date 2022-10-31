#include "simdisk/disk-manager.h"
#include <getopt.h>

static const struct option long_options[] = {
    {nullptr, no_argument, nullptr, 0}};

int DiskManager::cd(int argc, char *argv[]) {
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
    else
        path = argv[optind];
    
    m_cwd.m_dentry = getDirectoryEntry(path);
    m_cwd.m_inode = getIndexNode(m_cwd.m_dentry->m_inode);

    return 0;
}