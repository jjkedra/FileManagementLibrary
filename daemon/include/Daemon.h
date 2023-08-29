//
// Created by Jan Kędra on 12/05/2023.
//

#ifndef FILESYSTEM_DAEMON_H
#define FILESYSTEM_DAEMON_H

//#include <FileHandles.h>
#include <sys/ipc.h>
#include "FileHandles.h"
#include "../../library/include/Ipc.h"
#include <iostream>

#define ROOT_DIR ".libfs"

using fd_type = int;

class Daemon {
public:
    Daemon();
    void await_and_process_request();

private:

    void process_request();

    fd_type libfs_create(const char* name, long mode);
    void execute_create();

    int libfs_chmode(const char* name, long mode);
    void execute_chmode();

    int libfs_rename(const char* oldName, const char* newName);
    void execute_rename();

    int libfs_unlink(const char* name);
    void execute_unlink();

    fd_type libfs_open(const char* name, int flags);
    void execute_open();

    int libfs_read(fd_type fd, void* buf, unsigned int size);
    void execute_read();

    int libfs_write(fd_type fd, const void* buf, unsigned int size);
    void execute_write();

    int libfs_seek(fd_type fd, long int offset);
    void execute_seek();

    int libfs_close(fd_type fd);
    void execute_close();

    int libfs_notify(fd_type fd);
    void execute_notify();

    int libfs_unnotify(fd_type fd);
    void execute_unnotify();

    void send_notification(char *fname);

    void clean_buf();

    FileHandles fileHandles = FileHandles();
    SharedMemory sharedMemory = SharedMemory(0666 | IPC_CREAT);
    Semaphores semaphores = Semaphores(0666 | IPC_CREAT);

    lfs::request* req;
    lfs::result* res;
    char* buf;
};


#endif //FILESYSTEM_DAEMON_H
