#include "simdisk/disk-manager.h"

std::shared_ptr<DiskManager> DiskManager::instance;

DiskManager::DiskManager() : m_fat(DiskManager::fat_block_count) {
    m_disk_path = std::string(get_current_dir_name()).append("/vdisk");
}

DiskManager::~DiskManager() {
    shutdown();
}

void DiskManager::start() {
    initDisk();
    boot();
}

DiskBlock DiskManager::readBlock(int id) {
    std::ifstream ifs(m_disk_path, std::ios::binary | std::ios::in);

    if (!ifs.is_open())
        std::cerr << "could not open disk file" << std::endl;

    DiskBlock ret(id);
    std::shared_ptr<char[]> buffer(new char[DiskManager::block_size]);
    ifs.seekg(id * DiskManager::block_size);
    ifs.read(buffer.get(), DiskManager::block_size);
    ifs.close();

    ret.setData(buffer, 0);

    return ret;
}

void DiskManager::writeBlock(int id, DiskBlock &block) {
    std::fstream fs(m_disk_path, std::ios::binary | std::ios::in | std::ios::out);

    if (!fs.is_open())
        std::cerr << "could not open disk file" << std::endl;

    std::shared_ptr<char[]> buffer(new char[DiskManager::block_size]);
    block.getData(buffer, 0);
    fs.seekg(id * DiskManager::block_size);
    fs.write(buffer.get(), DiskManager::block_size);
    fs.close();
}

void DiskManager::initDisk() {
    auto initializor = std::make_unique<Initializor>(m_disk_path, DiskManager::block_size, block_count);
    initializor->setDiskManager(shared_from_this());
    initializor->init();
}

void DiskManager::boot() {
    if (m_is_on) return;
    m_is_on = true;
    std::cout << "[disk_manager]: booting" << std::endl;

    loadSuperBlock();
    loadFAT();
    loadINodeMap();
    loadBlockMap();
    initCWD();
}

void DiskManager::shutdown() {
    if (!m_is_on) return;
    m_is_on = false;
    std::cout << "[disk_manager]: shuting down" << std::endl;

    saveSuperBlock();
    saveFAT();
    saveINodeMap();
    saveBlockMap();
}

void DiskManager::loadSuperBlock() {
    std::shared_ptr<char[]> buffer(new char[DiskManager::block_size]);
    readBlock(DiskManager::super_block_id).getData(buffer, 0);
    m_super_block.load(buffer);
    m_super_block.m_dirt = false;
}

void DiskManager::saveSuperBlock() {
    DiskBlock block;
    block.setData(m_super_block.dump(), 0);
    writeBlock(DiskManager::super_block_id, block);
}

void DiskManager::loadFAT() {
    std::shared_ptr<char[]> buffer(new char[m_super_block.m_fat_size * DiskManager::block_size]);
    for (int i = 0; i < m_super_block.m_fat_size; i++) {
        readBlock(i + m_super_block.m_fat_location).getData(buffer, i * DiskManager::block_size);
    }

    m_fat.load(buffer);
}

void DiskManager::saveFAT() {
    auto buffer = m_fat.dump();
    for (int i = 0; i < m_super_block.m_fat_size; i++) {
        DiskBlock block;
        block.setData(buffer, i * DiskManager::block_size);
        writeBlock(i + m_super_block.m_fat_location, block);
    }
}

void DiskManager::loadINodeMap() {
    std::shared_ptr<char[]> buffer(new char[m_super_block.m_inode_map_size * DiskManager::block_size]);
    for (int i = 0; i < m_super_block.m_inode_map_size; i++) {
        readBlock(i + m_super_block.m_inode_map_location).getData(buffer, i * DiskManager::block_size);
    }
    m_inode_map.load(buffer);
}

void DiskManager::saveINodeMap() {
    auto buffer = m_inode_map.dump();
    for (int i = 0; i < m_super_block.m_inode_map_size; i++) {
        DiskBlock block;
        block.setData(buffer, i * DiskManager::block_size);
        writeBlock(i + m_super_block.m_inode_map_location, block);
    }
}

void DiskManager::loadBlockMap() {
    std::shared_ptr<char[]> buffer(new char[m_super_block.m_block_map_size * DiskManager::block_size]);
    for (int i = 0; i < m_super_block.m_block_map_size; i++) {
        readBlock(i + m_super_block.m_block_map_location).getData(buffer, i * DiskManager::block_size);
    }
    m_block_map.load(buffer);
}

void DiskManager::saveBlockMap() {
    auto buffer = m_block_map.dump();
    for (int i = 0; i < m_super_block.m_inode_map_size; i++) {
        DiskBlock block;
        block.setData(buffer, i * DiskManager::block_size);
        writeBlock(i + m_super_block.m_block_map_location, block);
    }
}

void DiskManager::initCWD() {
    // m_cwd.m_user = 0;    // m_user is const, initialized when constructed
    m_cwd.m_path = "/";
    m_cwd.m_dentry = std::make_shared<DirectoryEntry>(m_super_block.m_root);
    m_cwd.m_inode = getIndexNode(m_super_block.m_root.m_inode);
}

int DiskManager::expandedSize(int size) {
    int cnt = size / DiskManager::block_size;
    if (size % DiskManager::block_size)
        cnt++;

    return cnt * DiskManager::block_size;
}

std::string DiskManager::timeToDate(const std::chrono::system_clock::time_point &time) {
    time_t t = std::chrono::system_clock::to_time_t(time);
    tm p;
    char buffer[256];
    localtime_r(&t, &p);
    strftime(buffer, 256, "%Y-%m-%d %H:%M:%S", &p);

    return std::string(buffer);
}

std::shared_ptr<IndexNode> DiskManager::getIndexNode(int id) {
    assert(id < m_super_block.m_total_inode);
    auto ret = std::make_shared<IndexNode>();
    std::shared_ptr<char[]> buffer(new char[DiskManager::inode_size]);
    int b_id = id / DiskManager::inode_per_block + DiskManager::inode_block_offset;
    int offs = id % DiskManager::inode_per_block * DiskManager::inode_size;
    readBlock(b_id).getData(buffer, offs, DiskManager::inode_size);
    ret->load(buffer);

    return ret;
}

void DiskManager::writeIndexNode(int id, std::shared_ptr<IndexNode> inode) {
    assert(id < m_super_block.m_total_inode);
    int block_id = DiskManager::inode_block_offset + id / DiskManager::inode_per_block;
    int block_offset = id % DiskManager::inode_per_block * DiskManager::inode_size;
    DiskBlock block = readBlock(block_id);
    block.setData(inode->dump(), block_offset, DiskManager::inode_size);
    writeBlock(block_id, block);
}

std::shared_ptr<DirectoryEntry> DiskManager::getDirectoryEntry(std::string path) {
    if (path == "." || path == "./")
        return m_cwd.m_dentry;

    auto dirs = splitPath(path);
    std::shared_ptr<DirectoryEntry> cur_dentry;
    int i;

    if (dirs[0] == "/") {
        cur_dentry = std::make_shared<DirectoryEntry>(m_super_block.m_root);
        i = 1;
    }
    else {
        cur_dentry = m_cwd.m_dentry;
        i = 0;
    }
    while (i < dirs.size()) {
        std::string d = dirs[i];
        auto cur_inode = getIndexNode(cur_dentry->m_inode);
        if (cur_inode->m_type != FileType::DIRECTORY && cur_inode->m_type != FileType::LINK) {
            std::cerr << cur_dentry->m_filename << ": Not a directory" << std::endl;
            return nullptr;
        }
        auto file = std::dynamic_pointer_cast<DirFile>(getFile(cur_inode));
        if (d == ".")
            cur_dentry = std::make_shared<DirectoryEntry>(file->m_current);
        else if (d == "..")
            cur_dentry = std::make_shared<DirectoryEntry>(file->m_parent);
        else {
            bool exist = false;
            for (auto sub : file->m_dirs) {
                if (d == sub.m_filename) {
                    cur_dentry = std::make_shared<DirectoryEntry>(sub);
                    exist = true;
                    break;
                }
            }
            if (!exist) {
                std::cerr << cur_dentry->m_filename << ": No such file or directory" << std::endl;
                return nullptr;
            }
        }
        i++;
    }

    return cur_dentry;
}

std::shared_ptr<File> DiskManager::getFile(std::shared_ptr<IndexNode> inode) {
    std::shared_ptr<File> ret;
    int size = inode->m_blocks * DiskManager::block_size;
    int subs = inode->m_subs;

    if (inode->m_type == FileType::DIRECTORY) {
        ret = std::make_shared<DirFile>(size, subs);
    }
    else {
        ret = std::make_shared<DataFile>(size);
    }

    std::shared_ptr<char[]> buffer(new char[size]);
    int i = 0;
    int cur = inode->m_location + DiskManager::file_block_offset;
    while (cur != -1) {
        readBlock(cur).getData(buffer, i * DiskManager::block_size);
        i++;
        cur = m_fat[cur];
    }
    ret->load(buffer);

    return ret;
}

void DiskManager::writeFile(std::shared_ptr<IndexNode> inode, std::shared_ptr<File> file) {
    std::shared_ptr<char[]> buffer = file->dump();
    int i = 0;
    int cur = inode->m_location + DiskManager::file_block_offset;
    while (cur != -1) {
        DiskBlock block;
        block.setData(buffer, i * DiskManager::block_size);
        writeBlock(cur, block);
        i++;
        cur = m_fat[cur];
    }
}

int DiskManager::getDirSize(std::shared_ptr<IndexNode> inode) {
    auto file = std::dynamic_pointer_cast<DirFile>(getFile(inode));
    int ret = file->m_current.m_rec_len + file->m_parent.m_rec_len;
    for (auto d : file->m_dirs) {
        ret += d.m_rec_len;
    }

    return ret;
}

int DiskManager::expandFileSize(std::shared_ptr<IndexNode> inode, int size) {
    bool is_dir = inode->m_type == FileType::DIRECTORY;
    int origin_size = is_dir ? getDirSize(inode) : inode->m_size;
    int new_size = origin_size + size;
    int need_blocks = (new_size - inode->m_blocks * DiskManager::block_size) / DiskManager::block_size;

    if (need_blocks > 0) {
        expandBlock(inode->m_location, need_blocks);
        inode->m_blocks += need_blocks;
    }
    inode->m_size = is_dir ? inode->m_blocks * DiskManager::block_size : new_size;

    return need_blocks;
}

int DiskManager::getLastBlock(int id) {
    int cur = id;
    int ret;
    while (cur != -1) {
        ret = cur;
        cur = m_fat[cur];
    }

    return ret;
}

int DiskManager::allocIndexNode() {
    for (int i = 0; i < m_inode_map.size(); i++) {
        if (!m_inode_map.test(i)) {
            m_inode_map.set(i);
            m_super_block.m_free_inode--;
            return i;
        }
    }

    return -1; // return -1 if there is no free index node
}

int DiskManager::allocFileBlock(int n) {
    std::vector<int> blocks;
    int cnt = n;
    for (int i = 0; i < m_block_map.size(); i++) {
        if (!m_block_map.test(i)) {
            blocks.emplace_back(i);
            if (--cnt == 0) break;
        }
    }

    if (cnt == 0) {
        for (int j = 0; j < blocks.size() - 1; j++) {
            m_block_map.set(blocks[j]);
            m_fat[blocks[j] + DiskManager::file_block_offset] = blocks[j + 1] + DiskManager::file_block_offset;
        }
        m_block_map.set(blocks.back());
        m_super_block.m_free_block -= n;
        return blocks.front();
    }

    return -1; // return -1 if there is no enough free block
}

void DiskManager::expandBlock(int id, int n) {
    int first_new_block = allocFileBlock(n);
    if (first_new_block == -1) {
        std::cerr << "error: no enough blocks" << std::endl;
        return;
    }
    m_fat[getLastBlock(id + DiskManager::file_block_offset)] = first_new_block + DiskManager::file_block_offset;
}

void DiskManager::freeIndexNode(int id) {
    m_inode_map.reset(id);
    m_super_block.m_free_inode++;
}

void DiskManager::freeFlieBlock(int id) {
    int cur = id + DiskManager::file_block_offset;
    int cnt = 0;
    while (cur != -1) {
        int next = m_fat[cur];
        m_fat[cur] = -1;
        m_block_map.reset(cur);
        cnt++;
        cur = next;
    }
    m_super_block.m_free_block += cnt;
}

int DiskManager::exec(std::string command) {
    if (command.empty()) return 0;

    auto args = splitArgs(command);
    int argc = args.size();
    char **argv = new char *[argc];
    for (int i = 0; i < argc; i++) {
        argv[i] = args[i].data();
    }

    auto func = getFuncPtr(args[0]);
    if (func == nullptr) {
        std::cerr << "command '" << args[0] << "' not found" << std::endl;
        return 127;
    }

    optind = 0;
    return (this->*func)(argc, argv);
}

DiskManager::cfp DiskManager::getFuncPtr(std::string command_name) {
    if (command_name == "info") return &DiskManager::info;
    if (command_name == "cd") return &DiskManager::cd;
    if (command_name == "dir") return &DiskManager::dir;
    if (command_name == "md") return &DiskManager::md;
    // if (command_name == "rd") return &DiskManager::rd;
    if (command_name == "newfile") return &DiskManager::newfile;
    // if (command_name == "cat") return &DiskManager::cat;
    // if (command_name == "copy") return &DiskManager::copy;
    if (command_name == "del") return &DiskManager::del;
    // if (command_name == "check") return &DiskManager::check;

    return nullptr;
}

std::vector<std::string> DiskManager::splitArgs(std::string command) {
    std::vector<std::string> args;
    int l, r = 0;

    while (true) {
        l = command.find_first_not_of(' ', r);
        if (l == -1) break;
        r = command.find_first_of(' ', l);
        args.emplace_back(command.substr(l, r - l));
    }

    return args;
}

std::vector<std::string> DiskManager::splitPath(std::string path) {
    assert(!path.empty());
    std::vector<std::string> dirs;
    int l, r = 0;

    if (path[0] == '/') dirs.emplace_back("/");

    while (true) {
        l = path.find_first_not_of('/', r);
        if (l == -1) break;
        r = path.find_first_of('/', l);
        if (r == -1)
            dirs.emplace_back(path.substr(l));
        else
            dirs.emplace_back(path.substr(l, r - l));
    }

    return dirs;
}

std::string DiskManager::getPath(std::string cur, std::string path) {
    auto dirs = splitPath(path);
    if (dirs[0] != "/") {
        auto temp = splitPath(cur);
        temp.insert(temp.end(), dirs.begin(), dirs.end());
        dirs.swap(temp);
    }

    assert(dirs[0] == "/");
    std::vector<int> index;
    for (int i = 1; i < dirs.size(); i++) {
        std::string d = dirs[i];
        if (d == "..") {
            if (!index.empty())
                index.pop_back();
        }
        else if (d != ".") {
            index.push_back(i);
        }
    }

    std::string ret = "/";
    for (auto i : index) {
        ret.append(dirs[i] + "/");
    }

    return ret;
}
