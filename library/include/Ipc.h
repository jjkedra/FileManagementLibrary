#ifndef FILESYSTEM_IPC_H
#define FILESYSTEM_IPC_H


#include "Result.h"

#include <iostream>

#define N_SEMS 3
#define SEM_CMD 0
#define SEM_REQ 1
#define SEM_RES 2

class SharedMemory {
public:
    explicit SharedMemory(int flag);

    ~SharedMemory();

    [[nodiscard]] lfs::request *getRequest() const;

    [[nodiscard]] lfs::result *getResult() const;

    [[nodiscard]] char *getBuf() const;

    void *shm_addr;
};

class Semaphores {
public:
    explicit Semaphores(int flag);

    void lock(unsigned short sem_num) const;

    void lockWithUndo(unsigned short sem_num) const;

    void unlock(unsigned short sem_num) const;

private:
    int sems_id;
};

#endif //FILESYSTEM_IPC_H
