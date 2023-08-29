#include <cstring>
#include "../include/Libfs.h"
#include "../include/LibraryException.h"
#include "../../daemon/include/Result.h"
#include "../include/Ipc.h"

int Libfs::createFile(const char *name, long mode) {
    if (strlen(name) > FILENAME_LENGTH) {
        lfs_errno = ENAMETOOLONG;
        return -1;
    }
    lfs::arguments args = {.create_chmode_args = {.mode = mode}};
    snprintf(args.create_chmode_args.name, FILENAME_LENGTH, "%s", name);
    return performRequest(lfs::function::create, &args);
}

int Libfs::chmode(const char *name, long mode) {
    if (strlen(name) > FILENAME_LENGTH) {
        lfs_errno = ENAMETOOLONG;
        return -1;
    }
    lfs::arguments args = {.create_chmode_args = {.mode = mode}};
    snprintf(args.create_chmode_args.name, FILENAME_LENGTH, "%s", name);
    return performRequest(lfs::function::chmode, &args);
}

int Libfs::rename(const char *oldName, const char *newName) {
    if (strlen(oldName) > FILENAME_LENGTH || strlen(newName) > FILENAME_LENGTH) {
        lfs_errno = ENAMETOOLONG;
        return -1;
    }
    lfs::arguments args = {};
    snprintf(args.rename_args.oldname, FILENAME_LENGTH, "%s", oldName);
    snprintf(args.rename_args.newname, FILENAME_LENGTH, "%s", newName);
    return performRequest(lfs::function::rename, &args);
}

int Libfs::unlink(const char *name) {
    if (strlen(name) > FILENAME_LENGTH) {
        lfs_errno = ENAMETOOLONG;
        return -1;
    }
    lfs::arguments args = {};
    snprintf(args.unlink_args.name, FILENAME_LENGTH, "%s", name);
    return performRequest(lfs::function::unlink, &args);
}

int Libfs::open(const char *name, int flags) {
    if (strlen(name) > FILENAME_LENGTH) {
        lfs_errno = ENAMETOOLONG;
        return -1;
    }
    lfs::arguments args = {.open_args = {.flags = flags}};
    snprintf(args.open_args.name, FILENAME_LENGTH, "%s", name);
    return performRequest(lfs::function::open, &args);
}

int Libfs::read(int fd, char *buf, unsigned int size) {
    lfs::arguments args = {.read_write_args = {.fd = fd, .size = size}};
    return performRequest(lfs::function::read, &args, nullptr, 0, buf);
}

int Libfs::write(int fd, const char *buf, unsigned int size) {
    lfs::arguments args = {.read_write_args = {.fd = fd, .size = size}};
    return performRequest(lfs::function::write, &args, buf, size);
}

int Libfs::seek(int fd, int offset) {
    lfs::arguments args = {.seek_args = {.fd = fd, .offset = offset}};
    return performRequest(lfs::function::seek, &args);
}

int Libfs::close(int fd) {
    lfs::arguments args = {.close_notify_unnotify_args = {.fd = fd}};
    return performRequest(lfs::function::close, &args);
}

int Libfs::notify(int fd) {
    lfs::arguments args = {.close_notify_unnotify_args = {.fd = fd}};
    return performRequest(lfs::function::notify, &args);
}

int Libfs::unnotify(int fd) {
    lfs::arguments args = {.close_notify_unnotify_args = {.fd = fd}};
    return performRequest(lfs::function::unnotify, &args);
}

int Libfs::performRequest(lfs::function func, lfs::arguments *args,
                          const char *inBuf, unsigned int inSize, char *outBuf) {
    try {
        SharedMemory shm(0);
        Semaphores sems(0);

        // start command
        sems.lockWithUndo(SEM_CMD);

        // write request
        auto req = shm.getRequest();
        req->owner = getpid();
        req->func = func;
        req->args = *args;
        if (inBuf != nullptr) {
            memcpy(shm.getBuf(), inBuf, inSize);
        }
        sems.unlock(SEM_REQ);

        // read response
        sems.lock(SEM_RES);
        lfs::result *res = shm.getResult();
        int ret = res->ret;
        if (ret < 0) {
            lfs_errno = res->lfs_errno;
        } else if (outBuf != nullptr) {
            memcpy(outBuf, shm.getBuf(), ret);
        }

        // end command
        sems.unlock(SEM_CMD);

        return ret;
    } catch (const LibraryException &e) {
        lfs_errno = EDAEMON;
        return -1;
    }
}
