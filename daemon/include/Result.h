//
// Created by szw on 15.05.23.
//

#ifndef FILESYSTEM_RESULT_H
#define FILESYSTEM_RESULT_H

namespace lfs {
    struct result {
        int ret;
        int lfs_errno;
    };
}
#endif //FILESYSTEM_RESULT_H
