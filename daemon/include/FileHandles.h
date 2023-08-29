#ifndef DAEMON_FILEHANDLES_H
#define DAEMON_FILEHANDLES_H

#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <csignal>
#include <sys/fcntl.h>
#include "Request.h"
#include <array>
#include <vector>

#define MAX_FILE_HANDLES 128

#define NOTIFY_FLAG 4

using fd_type = lfs::fd_type;

struct handle {

    fd_type fd;
    pid_t owner;
    unsigned int position;
    int flags;
    char name[FILENAME_LENGTH];
};

class FileHandles{
public:
    FileHandles();
    ~FileHandles();
    void add_handle(fd_type fd, pid_t owner, int flags, char* name);
    void remove_handle(fd_type fd);
    void add_notification(fd_type fd);
    void remove_notification(fd_type fd);
    void modify_offset(fd_type fd, long offset);
    handle* get_handle_by_fd(fd_type fd);
    std::vector<handle*> getHandles();
private:
    std::array<handle*, MAX_FILE_HANDLES> handles_p;
};

#endif //DAEMON_FILEHANDLES_H
