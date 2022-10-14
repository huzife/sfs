#include "initializor/initializor.h"

// Initializor::Initializor(std::string path) m_path(path) {
//     m_path = getcwd(nullptr, 0);
//     // m_path = get_current_dir_name();
//     std::cout << "path: " << m_path << std::endl;
// }

Initializor::Initializor(std::string path) : m_path(path) {}

void Initializor::init() {
    if (this->exist())
        std::cout << "file system already exists" << std::endl;
    else {
        std::cout << "file system does not exist, creating..." << std::endl;
        create();
    }
}

bool Initializor::exist() {
    std::ifstream ifs(m_path);
    return ifs.is_open();
}

void Initializor::create() {
    std::ofstream ofs(m_path, std::ios::out | std::ios::binary);
    if (ofs.is_open()) {
        ofs.write(nullptr, DISK_SIZE);
    }
    else {
        std::cerr << "could not open disk file" << std::endl;
    }
    ofs.close();
}