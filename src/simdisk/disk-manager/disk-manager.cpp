#include "disk-manager/disk-manager.h"

std::unique_ptr<DiskManager> DiskManager::instance;

DiskManager::DiskManager() {
    m_disk_path = std::string(get_current_dir_name()).append("/vdisk");
}

std::unique_ptr<DiskManager>& DiskManager::getInstance() {
    if (instance == nullptr) {
        instance = std::make_unique<DiskManager>();
    }
    return instance;
}

void DiskManager::initDisk() {
    Initializor initializor(m_disk_path);
    initializor.init();
}