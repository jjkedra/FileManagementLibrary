#ifndef FILESYSTEM_PAYLOAD_H
#define FILESYSTEM_PAYLOAD_H

#include "Request.h"
#include "Result.h"

#define BUF_SIZE 65536

namespace lfs {
    struct shm_payload {
        request req;
        result res;
        char buf[BUF_SIZE];
    };
}

#endif //FILESYSTEM_PAYLOAD_H
