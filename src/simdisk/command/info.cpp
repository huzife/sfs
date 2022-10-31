#include "simdisk/disk-manager.h"

int DiskManager::info() {
    std::cout << std::setiosflags(std::ios::left);
    std::cout << std::setw(22) << "FAT location:" << m_super_block.m_fat_location << std::endl;
    std::cout << std::setw(22) << "FAT size:" << m_super_block.m_fat_size << std::endl;
    std::cout << std::setw(22) << "Inode map location:" << m_super_block.m_inode_map_location << std::endl;
    std::cout << std::setw(22) << "Inode map size:" << m_super_block.m_inode_map_size << std::endl;
    std::cout << std::setw(22) << "Block map location:" << m_super_block.m_block_map_location << std::endl;
    std::cout << std::setw(22) << "Block map size:" << m_super_block.m_block_map_size << std::endl;
    std::cout << std::setw(22) << "Inode count:" << m_super_block.m_total_inode << std::endl;
    std::cout << std::setw(22) << "Block count:" << m_super_block.m_total_block << std::endl;
    std::cout << std::setw(22) << "Free inode:" << m_super_block.m_free_inode << std::endl;
    std::cout << std::setw(22) << "Free block:" << m_super_block.m_free_block << std::endl;
    std::cout << std::setw(22) << "Block size:" << m_super_block.m_block_size << std::endl;
}