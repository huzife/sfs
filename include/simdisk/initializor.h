#ifndef __INITIALIZOR_H
#define __INITIALIZOR_H

#include <iostream>
#include <fstream>
#include <string>

class Initializor {
private:
    static constexpr int disk_size = 100 * 1024 * 1024; // 100 MiB space

    std::string m_path;

public:
    // Initializor();

    Initializor(std::string path);

    void init();        // initialized file system

private:
    bool exist();       // check if the file system is exists

    void create();      // create file system

};

#endif  // __INITIALIZOR_H