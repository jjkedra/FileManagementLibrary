#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <cstring>
#include "../include/LibraryException.h"
#include "../../daemon/include/Payload.h"
#include "../include/Ipc.h"
#include "Daemon.h"


#define N_SEMS 3
#define SEM_CMD 0
#define SEM_REQ 1
#define SEM_RES 2

std::string getRootPath(){
    return std::string(getenv("HOME")) + "/" + std::string(ROOT_DIR);
}
SharedMemory::SharedMemory(int flag) {
    key_t shm_key = ftok(getRootPath().c_str(), 1);
    if (shm_key == (key_t) -1) {
        throw LibraryException("ftok() for shm failed", __FUNCTION__, __LINE__);
    }

    int shm_id = shmget(shm_key, sizeof(lfs::shm_payload), 0666 | flag);
    if (shm_id == -1) {
        throw LibraryException("shmget() failed", __FUNCTION__, __LINE__);
    }

    shm_addr = shmat(shm_id, nullptr, 0);
    if (shm_addr == (void *) -1) {
        throw LibraryException("shmat() failed", __FUNCTION__, __LINE__);
    }
}

lfs::request *SharedMemory::getRequest() const {
    return (lfs::request *) ((char *) shm_addr + offsetof(lfs::shm_payload, req));
}

lfs::result * SharedMemory::getResult() const {
    return (lfs::result *) ((char *) shm_addr + offsetof(lfs::shm_payload, res));
}

char* SharedMemory::getBuf() const {
    return (char *) shm_addr + offsetof(lfs::shm_payload, buf);
}

SharedMemory::~SharedMemory() {
    if (shmdt(shm_addr) == -1) {
        throw LibraryException("shmdt() failed", __FUNCTION__, __LINE__);
    }
}

Semaphores::Semaphores(int flag) {
    key_t sems_key = ftok(getRootPath().c_str(), 1);
    if (sems_key == (key_t) -1) {
        throw LibraryException("ftok() for sems failed", __FUNCTION__, __LINE__);
    }
    sems_id = semget(sems_key, N_SEMS, 0666 | flag);
    if (sems_id == -1) {
        throw LibraryException("semget() failed", __FUNCTION__, __LINE__);
    }
    if(flag & IPC_CREAT){
        unsigned short initial_sem_values[3] = {1, 0, 0};
        semctl(sems_id, 0, SETALL, initial_sem_values);
    }
}

void addToSemaphore(int sems_id, unsigned short sem_num, short diff, short flag) {
    struct sembuf semops[N_SEMS] = {sem_num, diff, flag};
    if (semop(sems_id, semops, 1) == -1) {
        throw LibraryException("semop() failed", __FUNCTION__, __LINE__);
    }
}

void Semaphores::lock(unsigned short sem_num) const {
    addToSemaphore(sems_id, sem_num, -1, 0);
}

void Semaphores::lockWithUndo(unsigned short sem_num) const {
    addToSemaphore(sems_id, sem_num, -1, SEM_UNDO);
}

void Semaphores::unlock(unsigned short sem_num) const {
    addToSemaphore(sems_id, sem_num, 1, SEM_UNDO);
}
