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

void catch_sigs(char *file_name, Libfs& libfs_) {
    fork();

    gfile_name = file_name;
    int fd = libfs_.open(file_name, O_RDWR);
    libfs_.notify(fd);

    signal(SIGUSR1, sigusr1Handler);

    std::cout << "Process nr " << getpid() << " opened and subscribed to file " << file_name << std::endl;

    while (true) {}
}

int main() {
    Libfs libfs_ = Libfs();

    std::string fn1 = "test1.txt";
    char *file_name1 = &fn1[0];
    std::string fn2 = "test2.txt";
    char *file_name2 = &fn2[0];

    int fd1 = libfs_.createFile(file_name1, 0777);
    libfs_.unnotify(fd1);

    int fd2 = libfs_.createFile(file_name2, 0777);
    libfs_.unnotify(fd2);

    pid_t pid = fork();
    if (pid == 0 ) {
        catch_sigs(file_name1, libfs_);
    }
    else if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    else {
        pid_t pid2 = fork();
        if (pid2 == 0 ) {
            catch_sigs(file_name2, libfs_);
        }
        else if (pid2 < 0) {
            exit(EXIT_FAILURE);
        }
        else {
            usleep(1000000);

            fd1 = libfs_.open(file_name1, O_RDWR);
            fd2 = libfs_.open(file_name2, O_RDWR);

            std::cout << "Writing to file " << file_name1 << std::endl;
            libfs_.write(fd1, "Test 12345", 10);

            usleep(100000);

            std::cout << "Writing to file " << file_name2 << std::endl;
            libfs_.write(fd2, "Test 12345", 10);

            usleep(50000);

            libfs_.unlink(file_name1);
            libfs_.unlink(file_name2);
        }
    }
}
