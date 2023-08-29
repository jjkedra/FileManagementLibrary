//
// Created by szw on 15.05.23.
//

#ifndef FILESYSTEM_REQUEST_H
#define FILESYSTEM_REQUEST_H

#include <unistd.h>

#define FILENAME_LENGTH 256

namespace lfs {

    typedef int fd_type;

    struct create_chmode_args {
        char name[FILENAME_LENGTH];
        long mode;
    };

    struct rename_args {
        char oldname[FILENAME_LENGTH];
        char newname[FILENAME_LENGTH];
    };

    struct unlink_args {
        char name[FILENAME_LENGTH];
    };

    struct open_args {
        char name[FILENAME_LENGTH];
        int flags;
    };

    struct read_write_args {
        fd_type fd;
        unsigned size;
    };

    struct seek_args {
        fd_type fd;
        long offset;
    };

    struct close_notify_unnotify_args {
        fd_type fd;
    };

    enum function {
        create, chmode, rename, unlink, open, read, write, seek, close, notify, unnotify
    };

    union arguments {
        struct create_chmode_args create_chmode_args;
        struct rename_args rename_args;
        struct unlink_args unlink_args;
        struct open_args open_args;
        struct read_write_args read_write_args;
        struct seek_args seek_args;
        struct close_notify_unnotify_args close_notify_unnotify_args;
    };

    struct request {
        pid_t owner;
        function func;
        arguments args;
    };
}

#endif //FILESYSTEM_REQUEST_H
