#include "simdisk/disk-manager.h"

static const struct option long_options[] = {
    {"sub-directories", no_argument, nullptr, 's'},
    {nullptr, no_argument, nullptr, 0}};

int DiskManager::dir(int argc, char *argv[]) {
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
            return 1;
        }
    }

    std::string path;
    std::shared_ptr<DirectoryEntry> dentry;
    if (optind == argc) {
        path = m_cwd.m_dentry->m_filename;
        dentry = m_cwd.m_dentry;
    }
    else {
        path = argv[optind];
        dentry = getDirectoryEntry(path);
    }

    if (dentry == nullptr) return -1;
    auto inode = getIndexNode(dentry->m_inode);
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
        std::to_string((*std::max_element(dirs.begin(), dirs.end(),
                                          [](std::shared_ptr<IndexNode> a, std::shared_ptr<IndexNode> b) {
                                              return a->m_owner < b->m_owner;
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
    for (int i = 0; i < dirs.size(); i++) {
        std::cout << IndexNode::FileTypeToStr(dirs[i]->m_type)
                  << IndexNode::PermissionToStr(dirs[i]->m_owner_permission)
                  << IndexNode::PermissionToStr(dirs[i]->m_other_permission)
                  << std::setiosflags(std::ios::right)
                  << std::setw(max_subs_len) << dirs[i]->m_subs
                  << std::setw(max_owner_len) << dirs[i]->m_owner
                  << std::setw(max_size_len) << dirs[i]->m_size
                  << std::resetiosflags(std::ios::right)
                  << ' ' << timeToDate(dirs[i]->m_modify_time)
                  << ' ' << names[i]
                  << std::endl;
    }

    return 0;
}