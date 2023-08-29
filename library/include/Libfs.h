#ifndef FILESYSTEM_LIBFS_H
#define FILESYSTEM_LIBFS_H

#include "../../daemon/include/Request.h"

#define EDAEMON 99

class Libfs {
public:
    int createFile(const char *name, long mode);

    int chmode(const char *name, long mode);

    int rename(const char *oldName, const char *newName);

    int unlink(const char *name);

    int open(const char *name, int flags);

    int read(int fd, char *buf, unsigned int size);

    int write(int fd, const char *buf, unsigned int size);

    int seek(int fd, int offset);

    int close(int fd);

    int notify(int fd);

    int unnotify(int fd);

    int lfs_errno;
private:

    int performRequest(lfs::function func, lfs::arguments *args,
                       const char *inBuf = nullptr, unsigned int inSize = 0,
                       char *outBuf = nullptr);
};


#endif //FILESYSTEM_LIBFS_H
