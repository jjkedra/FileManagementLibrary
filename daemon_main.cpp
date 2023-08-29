#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <csignal>
#include <Daemon.h>
#include "daemon/include/DaemonException.h"
#include <filesystem>

#undef DEBUG
// #define DEBUG

void create_dir(const std::string &root_path) {
    if (!std::filesystem::exists(root_path)) {
        std::filesystem::create_directory(root_path);
    }
}

void change_dir(const std::string &root_path) {
    if ((chdir(root_path.c_str())) < 0) {
        std::cout << "chdir\n";
        exit(EXIT_FAILURE);
    }
}

void sigusr1Handler(int signal) {
    // Do nothing - ignore the signal
}

int main() {
    std::string root_path = std::string(getenv("HOME")) + "/" + std::string(ROOT_DIR);
    create_dir(root_path);
    change_dir(root_path);

    auto daemon = Daemon();

    
#ifdef DEBUG
    while (true) {
        signal(SIGUSR1, sigusr1Handler);
        daemon.await_and_process_request();
    }
#else
    pid_t pid, sid;
    pid = fork();

    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // If we want daemon to be able to write to files, we need to change umask
    umask(0);

    sid = setsid();
    if (sid < 0) {
        std::cout << "sid\n";
        exit(EXIT_FAILURE);
    }
    // Close standard file descriptors as daemon does not interact with terminal
    // they are a hazard if left open
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // The loop daemon do be in work here
    while (true) {
        signal(SIGUSR1, sigusr1Handler);
        daemon.await_and_process_request();
    }
#endif
}