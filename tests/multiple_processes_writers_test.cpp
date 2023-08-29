#include <iostream>
#include <csignal>
#include "../library/include/Libfs.h"
#include <fcntl.h>

char *gfile_name;
int gfd;

void sigusr1Handler(int signal) {
    if(signal == SIGUSR1) {}
    std::cout << "Process nr " << getpid() << " received SIGUSR1 from " << gfile_name << std::endl;
}

void write_file(Libfs &libfs_) {
    std::string text = std::to_string(getpid());
    gfd = libfs_.open(gfile_name, O_RDWR);
    libfs_.notify(gfd);
    libfs_.write(gfd, &text[0], text.length());
    std::cout << "Process nr " << getpid() << " done writing to " << gfile_name << std::endl;
    exit(EXIT_SUCCESS);
}

void catch_sigs(char *file_name, Libfs &libfs_) {
    fork();
    fork();

    gfile_name = file_name;
    signal(SIGUSR1, sigusr1Handler);

    write_file(libfs_);
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
        usleep(1000000);

        libfs_.close(fd);
        fd = libfs_.open(file_name, O_RDONLY);

        char buf[30];
        libfs_.read(fd, buf, 30);
        std::cout << "File content: " << buf << std::endl;

        usleep(50000);

        libfs_.unlink(file_name);
    }
}
