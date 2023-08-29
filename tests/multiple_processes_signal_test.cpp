//
// Created by Lidia on 6/9/2023.
//

#include <iostream>
#include <csignal>
#include "../library/include/Libfs.h"
#include <fcntl.h>

char *gfile_name;

void sigusr1Handler(int signal) {
    if(signal == SIGUSR1) {}
    std::cout << "Process nr " << getpid() << " received SIGUSR1. File was " << gfile_name << std::endl;
    exit(EXIT_SUCCESS);
}

void catch_sigs(char *file_name, Libfs &libfs_) {
    fork();
    fork();

    gfile_name = file_name;
    int fd = libfs_.open(file_name, O_RDWR);
    libfs_.notify(fd);

    std::cout << "Process nr " << getpid() << " opened and subscribed to file " << file_name << std::endl;

    while (true) {signal(SIGUSR1, sigusr1Handler);}
}

int main() {
    Libfs libfs_ = Libfs();

    std::string fn = "test.txt";
    char *file_name = &fn[0];

    int fd = libfs_.createFile(file_name, 0777);
    libfs_.unnotify(fd);

    pid_t pid = fork();
    if (pid == 0 ) {
        catch_sigs(file_name, libfs_);
    }
    else if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    else {
        usleep(500000);

        std::cout << "Writing to file " << file_name << std::endl;
        libfs_.write(fd, "Test 12345", 10);

        usleep(200000);

        libfs_.unlink(file_name);
    }
}
