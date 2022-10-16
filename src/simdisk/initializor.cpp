#include "simdisk/initializor.h"

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

    // create an empty file and write 100 MiB
    // warn: must use loop instead of a 100 MiB char array!!!
    if (!ofs.is_open())
        std::cerr << "could not open disk file" << std::endl;

    char buffer[1024 * 1024];
    for (int i = 0; i < 100; i++) {
        ofs.write(buffer, sizeof(buffer));
    }
    ofs.close();
}