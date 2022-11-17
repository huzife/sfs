#include "simdisk/disk-manager.h"

int DiskManager::info(int argc, char *argv[], int sid) {
	std::string out;
	out += fill("FAT location:", 22, 'r') + std::to_string(m_super_block.m_fat_location) + "\n";
	out += fill("FAT size:", 22, 'r') + std::to_string(m_super_block.m_fat_size) + "\n";
	out += fill("Inode map location:", 22, 'r') + std::to_string(m_super_block.m_inode_map_location) + "\n";
	out += fill("Inode map size:", 22, 'r') + std::to_string(m_super_block.m_inode_map_size) + "\n";
	out += fill("Block map location:", 22, 'r') + std::to_string(m_super_block.m_block_map_location) + "\n";
	out += fill("Block map size:", 22, 'r') + std::to_string(m_super_block.m_block_map_size) + "\n";
	out += fill("Inode count:", 22, 'r') + std::to_string(m_super_block.m_total_inode) + "\n";
	out += fill("Block count:", 22, 'r') + std::to_string(m_super_block.m_total_block) + "\n";
	out += fill("Free inode:", 22, 'r') + std::to_string(m_super_block.m_free_inode) + "\n";
	out += fill("Free block:", 22, 'r') + std::to_string(m_super_block.m_free_block) + "\n";
	out += fill("Block size:", 22, 'r') + std::to_string(m_super_block.m_block_size) + "\n";
	writeOutput(out, sid);

	// std::cout << std::setiosflags(std::ios::left);
	// std::cout << std::setw(22) << "FAT location:" << m_super_block.m_fat_location << std::endl;
	// std::cout << std::setw(22) << "FAT size:" << m_super_block.m_fat_size << std::endl;
	// std::cout << std::setw(22) << "Inode map location:" << m_super_block.m_inode_map_location << std::endl;
	// std::cout << std::setw(22) << "Inode map size:" << m_super_block.m_inode_map_size << std::endl;
	// std::cout << std::setw(22) << "Block map location:" << m_super_block.m_block_map_location << std::endl;
	// std::cout << std::setw(22) << "Block map size:" << m_super_block.m_block_map_size << std::endl;
	// std::cout << std::setw(22) << "Inode count:" << m_super_block.m_total_inode << std::endl;
	// std::cout << std::setw(22) << "Block count:" << m_super_block.m_total_block << std::endl;
	// std::cout << std::setw(22) << "Free inode:" << m_super_block.m_free_inode << std::endl;
	// std::cout << std::setw(22) << "Free block:" << m_super_block.m_free_block << std::endl;
	// std::cout << std::setw(22) << "Block size:" << m_super_block.m_block_size << std::endl;
	// std::cout << std::resetiosflags(std::ios::left);
	return 0;
}