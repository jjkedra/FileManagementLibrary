#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <csignal>
#include <sys/fcntl.h>
#include "../include/Daemon.h"
#include "../include/DaemonException.h"
#include "Payload.h"

Daemon::Daemon() : fileHandles{}, sharedMemory{0666 | IPC_CREAT}, semaphores(0666 | IPC_CREAT) {
    res = sharedMemory.getResult();
    req = sharedMemory.getRequest();
    buf = sharedMemory.getBuf();
    clean_buf();
}

fd_type Daemon::libfs_create(const char *name, long mode) {
    struct stat st;
    if (stat(name, &st) == 0) {
        // File already exists
        errno = EEXIST;
        throw DaemonException("File already exists.", errno, __FUNCTION__, __LINE__);
    }

    fd_type fileDescriptor = open(name, O_CREAT | O_WRONLY, mode);
    if (fileDescriptor == -1) {
        throw DaemonException("Error creating the file.", errno, __FUNCTION__, __LINE__);
    }
    return fileDescriptor;
}

void Daemon::execute_create() {
    auto pid = req->owner;
    auto args = req->args.create_chmode_args;
    try{
        res->ret = libfs_create(args.name, args.mode);
    }
    catch(DaemonException){
        res->lfs_errno = errno;
        res->ret = -1;
    }
    if(res->ret != -1){
        fileHandles.add_handle(res->ret, pid, args.mode, args.name);
    }
}

int Daemon::libfs_chmode(const char *name, long mode) {
    int result = chmod(name, mode);
    if (result == -1) {
        throw DaemonException("Error changing file mode.", errno, __FUNCTION__, __LINE__);
    }
    return result;
}

void Daemon::execute_chmode() {
    auto args = req->args.create_chmode_args;
    try{
        res->ret = libfs_chmode(args.name, args.mode);
    }
    catch(DaemonException){
        res->ret = -1;
        res->lfs_errno = errno;
    }
}

fd_type Daemon::libfs_rename(const char *oldName, const char *newName) {
    int result = std::rename(oldName, newName);
    if (result == -1) {
        throw DaemonException("Error renaming the file.", errno, __FUNCTION__, __LINE__);
    }
    return result;
}

void Daemon::execute_rename() {
    auto args = req->args.rename_args;
    try{
        res->ret = libfs_rename(args.oldname, args.newname);
    }
    catch(DaemonException){
        res->ret = -1;
        res->lfs_errno = errno;
    }
}

int Daemon::libfs_unlink(const char* name) {
    int result = std::remove(name);
    if (result == -1) {
        throw DaemonException("Error unlinking the file.", errno, __FUNCTION__, __LINE__);
    }

    return result;
}

void Daemon::execute_unlink() {
    auto args = req->args.unlink_args;
    try{
        res->ret = libfs_unlink(args.name);
        send_notification(args.name);
    }
    catch(DaemonException){
        res->ret = -1;
        res->lfs_errno = errno;
    }
}

fd_type Daemon::libfs_open(const char* name, int flags) {
    int fileDescriptor = open(name, flags);
    if (fileDescriptor == -1) {
        throw DaemonException("Error opening the file.", errno, __FUNCTION__, __LINE__);
    }

    return fileDescriptor;
}

void Daemon::execute_open() {
    auto args = req->args.open_args;
    try{
        res->ret = libfs_open(args.name, args.flags);
        lseek(res->ret, 0, SEEK_SET);
        fileHandles.add_handle(res->ret, req->owner, args.flags, args.name);
    }
    catch(DaemonException){
        res->ret = -1;
        res->lfs_errno = errno;
    }

}

int Daemon::libfs_read(fd_type fd, void *buf, unsigned int size) {
    std::cout << size << " bytes to read\n";
    ssize_t bytesRead = read(fd, buf, size);
    if (bytesRead == -1) {
        throw DaemonException("Error reading from the file.", errno, __FUNCTION__, __LINE__);
    }

    return bytesRead;
}

void Daemon::execute_read() {
    auto args = req->args.read_write_args;
    try{
        res->ret = libfs_read(args.fd, buf, args.size);
        fileHandles.modify_offset(args.fd, res->ret);
    }
    catch(DaemonException){
        res->ret = -1;
        res->lfs_errno = errno;
    }
}

int Daemon::libfs_write(fd_type fd, const void *buf, unsigned int size) {
    ssize_t bytesWritten = write(fd, buf, size);
    if (bytesWritten == -1) {
        throw DaemonException("Error writing to the file: " + std::string(strerror(errno)), errno, __FUNCTION__, __LINE__);
    }
    return bytesWritten;
}

void Daemon::execute_write() {
    auto args = req->args.read_write_args;
    try{
        res->ret = libfs_write(args.fd, buf, args.size);
        char* name = fileHandles.get_handle_by_fd(args.fd)->name;
        send_notification(name);
    }
    catch(DaemonException){
        res->ret = -1;
        res->lfs_errno = errno;
    }
}

int Daemon::libfs_seek(fd_type fd, long offset) {
    off_t result = lseek(fd, offset, SEEK_SET);
    if (result == -1) {
        int error = errno; // Store the errno value before it changes
        throw DaemonException("Error seeking the file.", errno, __FUNCTION__, __LINE__);
    }

    return result;
}

void Daemon::execute_seek() {
    auto args = req->args.seek_args;
    try{
        handle* handle = fileHandles.get_handle_by_fd(args.fd);
        if (handle == nullptr) {
            errno = EBADF;
            throw DaemonException("Error seeking the file.", EBADF, __FUNCTION__, __LINE__);
        }

        auto new_position = handle->position + args.offset;
        res->ret = libfs_seek(args.fd, new_position);
        fileHandles.modify_offset(args.fd, new_position);
    }
    catch(DaemonException){
        res->ret = -1;
        res->lfs_errno = errno;
    }
}

int Daemon::libfs_close(fd_type fd) {
    int result = close(fd);
    if (result == -1) {
        throw DaemonException("Error closing the file.", errno, __FUNCTION__, __LINE__);
    }

    return result;
}

void Daemon::execute_close() {
    auto args = req->args.close_notify_unnotify_args;
    try{
        res->ret = libfs_close(args.fd);
        fileHandles.remove_handle(args.fd);
        std::cout << "Removing handle\n";
    }
    catch(DaemonException){
        res->ret = -1;
        res->lfs_errno = errno;
    }
}

int Daemon::libfs_notify(fd_type fd) {
    auto handle = fileHandles.get_handle_by_fd(fd);
    if (handle == nullptr) {
        errno = EBADF;
        throw DaemonException("Error notifying the file.", errno, __FUNCTION__, __LINE__);
    } else if (handle->flags & NOTIFY_FLAG) {
        return 1;
    }
    fileHandles.add_notification(fd);
    return 0;
}

void Daemon::execute_notify() {
    auto fd = req->args.close_notify_unnotify_args.fd;
    try{
        res->ret = libfs_notify(fd);
    }
    catch(DaemonException){
        res->ret = -1;
        res->lfs_errno = errno;
    }
}

int Daemon::libfs_unnotify(fd_type fd) {
    auto handle = fileHandles.get_handle_by_fd(fd);
    if (handle == nullptr) {
        errno = EBADF;
        throw DaemonException("Error unnotifying the file.", errno, __FUNCTION__, __LINE__);
    } else if (!(handle->flags & NOTIFY_FLAG)) {
        return 1;
    }
    fileHandles.remove_notification(fd);
    return 0;
}

void Daemon::execute_unnotify() {
    auto fd = req->args.close_notify_unnotify_args.fd;
    try{
        res->ret = libfs_unnotify(fd);
    }
    catch(DaemonException){
        res->ret = -1;
        res->lfs_errno = errno;
    }
}

/*
 * If client used libfs_notify, then the daemon will send a notification to the client if the file was modified.
 */
void Daemon::send_notification(char *fname) {
    std::vector<pid_t> owners;

    for (const auto& handle : this->fileHandles.getHandles()) {
        if ((handle != nullptr) && (handle->flags & NOTIFY_FLAG) && (std::strcmp(handle->name, fname) == 0)) {
            owners.push_back(handle->owner);
        }
    }

    for (pid_t owner : owners) {
        // Send the SIGUSR1 signal to each owner
        kill(owner, SIGUSR1);
    }
}

void Daemon::await_and_process_request() {
    semaphores.lock(SEM_REQ);
    process_request();
    semaphores.unlock(SEM_RES);
}

void Daemon::process_request() {
    switch (req->func) {
        case lfs::function::create:
            execute_create();
            break;
        case lfs::function::chmode:
            execute_chmode();
            break;
        case lfs::function::rename:
            execute_rename();
            break;
        case lfs::function::unlink:
            execute_unlink();
            break;
        case lfs::function::open:
            execute_open();
            break;
        case lfs::function::read:
            execute_read();
            break;
        case lfs::function::write:
            execute_write();
            break;
        case lfs::function::seek:
            execute_seek();
            break;
        case lfs::function::close:
            execute_close();
            break;
        case lfs::function::notify:
            execute_notify();
            break;
        case lfs::function::unnotify:
            execute_unnotify();
            break;
    }
}

void Daemon::clean_buf() {
    for(int i=0; i<BUF_SIZE; ++i){
        buf[i] = '\0';
    }
}
