#ifndef __INITIALIZOR_H
#define __INITIALIZOR_H

#include <iostream>
#include <fstream>
#include <string>

#define DISK_SIZE 104857600

class Initializor {
private:
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