#include "disk-manager/disk-manager.h"

std::unique_ptr<DiskManager> DiskManager::instance;

DiskManager::DiskManager() {
    m_disk_path = std::string(get_current_dir_name()).append("/vdisk");
    m_initializor = std::make_unique<Initializor>(m_disk_path);
}

std::unique_ptr<DiskManager>& DiskManager::getInstance() {
    if (instance == nullptr) {
        instance = std::make_unique<DiskManager>();
    }
    return instance;
}

std::unique_ptr<Initializor>& DiskManager::getInitializor() {
    return m_initializor;
}