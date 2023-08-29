#include <FileHandles.h>
#include <cstring>

FileHandles::FileHandles() {
    handles_p.fill(nullptr);
}

FileHandles::~FileHandles() {
    for(auto & hp : handles_p){
        if(hp != nullptr){
            delete hp;
        }
    }
}

void FileHandles::add_handle(fd_type fd, pid_t owner, int flags, char* name) {
    for(auto & hp : handles_p){
        if(hp == nullptr){
            hp = new handle;
            hp->fd = fd;
            hp->owner = owner;
            hp->position = 0;
            hp->flags = flags;
            hp->flags &= ~NOTIFY_FLAG;
            std::strcpy(hp->name, name);
            break;
        }
    }
}

void FileHandles::remove_handle(fd_type fd) {
    handle* h = get_handle_by_fd(fd);
    if (h != nullptr){
        delete h;
        h = nullptr;
    }
}

void FileHandles::add_notification(fd_type fd) {
    auto h = get_handle_by_fd(fd);
    h->flags |= NOTIFY_FLAG;
}

void FileHandles::remove_notification(fd_type fd) {
    auto h = get_handle_by_fd(fd);
    h->flags &= ~NOTIFY_FLAG;
}

void FileHandles::modify_offset(fd_type fd, long offset) {
    auto h = get_handle_by_fd(fd);
    if(h->position + offset >= 0){
        h->position += offset;
    }
}

handle* FileHandles::get_handle_by_fd(fd_type fd) {
    for(auto & hp : handles_p){
        if(hp != nullptr && hp->fd == fd){
            return hp;
        }
    }
    handle* nullHandle = nullptr;
    return nullHandle;
}

std::vector<handle*> FileHandles::getHandles() {
    std::vector<handle*> handles;
    for(auto & hp : handles_p){
        if(hp != nullptr){
            handles.push_back(hp);
        }
    }
    return handles;
}