#include <sys/ipc.h>
#include <sys/shm.h>
#include "gtest/gtest.h"
#include "../library/include/Libfs.h"
#include "../daemon/include/Payload.h"
#include "Daemon.h"
#include <iostream>
#include <fcntl.h>

TEST(GeneralSuite, TestGTest) {
    EXPECT_EQ(2 + 2, 4);
}

class DaemonSuite : public ::testing::Test {
protected:
    void SetUp() override {
        std::cout << "Starting setup\n";
        libfs_ = new Libfs();
        shm_id = shmget(shm_key, sizeof(lfs::shm_payload), 0666);
        std::cout << "Setup ended\n";
    }

    void TearDown() override {
        delete libfs_;
    }

    Libfs *libfs_;
    std::string fn = "test.txt";
    char *file_name = &fn[0];
    std::string fn2 = "test2.txt";
    char *file_name2 = &fn2[0];
    std::string ne = "nonexistent";
    char *nonexistent = &ne[0];
    std::string tl = "toolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongnametoolongname";
    char *too_long = &tl[0];
    int mode = 0777;
    int mode2 = 0776;
    std::string fc = "Test 12345";
    char *file_content = &fc[0];
    key_t shm_key = ftok((std::string(getenv("HOME")) + "/" + std::string(ROOT_DIR)).c_str(), 1);
    int shm_id;
};

TEST_F(DaemonSuite, TestCreateFileUnlink) {
    int ret = libfs_->createFile(file_name, mode);
    EXPECT_NE(ret, -1);
    std::cout << "Executed create request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_NE(payload.req.owner, 0);
    EXPECT_EQ(payload.req.func, lfs::function::create);
    EXPECT_STREQ(payload.req.args.create_chmode_args.name, file_name);
    EXPECT_EQ(payload.req.args.create_chmode_args.mode, mode);

    EXPECT_EQ(payload.res.ret, ret);

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";

    payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_NE(payload.req.owner, 0);
    EXPECT_EQ(payload.req.func, lfs::function::unlink);
    EXPECT_STREQ(payload.req.args.create_chmode_args.name, file_name);

    EXPECT_EQ(payload.res.ret, 0);
}

TEST_F(DaemonSuite, TestOpenClose) {
    int fd = libfs_->createFile(file_name, mode);
    EXPECT_NE(fd, -1);
    std::cout << "Executed create request\n";

    int ret = libfs_->close(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed close request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_NE(payload.req.owner, 0);
    EXPECT_EQ(payload.req.func, lfs::function::close);
    EXPECT_EQ(payload.req.args.close_notify_unnotify_args.fd, fd);

    EXPECT_EQ(payload.res.ret, 0);

    fd = libfs_->open(file_name, O_RDONLY);
    EXPECT_NE(fd, -1);
    std::cout << "Executed open request\n";

    payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_NE(payload.req.owner, 0);
    EXPECT_EQ(payload.req.func, lfs::function::open);
    EXPECT_STREQ(payload.req.args.open_args.name, file_name);
    EXPECT_EQ(payload.req.args.open_args.flags, O_RDONLY);

    EXPECT_EQ(payload.res.ret, fd);

    ret = libfs_->close(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed close request\n";

    payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_NE(payload.req.owner, 0);
    EXPECT_EQ(payload.req.func, lfs::function::close);
    EXPECT_EQ(payload.req.args.close_notify_unnotify_args.fd, fd);

    EXPECT_EQ(payload.res.ret, 0);

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestReadEmpty) {
    int fd = libfs_->createFile(file_name, mode);
    EXPECT_NE(fd, -1);
    std::cout << "Executed create request\n";

    fd = libfs_->open(file_name, O_RDONLY);
    EXPECT_NE(fd, -1);
    std::cout << "Executed open request\n";

    char buf[10];
    int ret = libfs_->read(fd, buf, 10);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed read request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_NE(payload.req.owner, 0);
    EXPECT_EQ(payload.req.func, lfs::function::read);
    EXPECT_EQ(payload.req.args.read_write_args.fd, fd);
    EXPECT_EQ(payload.req.args.read_write_args.size, 10);

    EXPECT_EQ(payload.res.ret, 0);
    EXPECT_EQ(memcmp("", payload.buf, ret), 0);

    ret = libfs_->close(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed close request\n";

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestWrite) {
    int fd = libfs_->createFile(file_name, mode);
    EXPECT_NE(fd, -1);
    std::cout << "Executed create request\n";

    fd = libfs_->open(file_name, O_WRONLY);
    EXPECT_NE(fd, -1);
    std::cout << "Executed open request\n";

    int ret = libfs_->write(fd, file_content, 10);
    EXPECT_EQ(ret, 10);
    std::cout << "Executed write request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_NE(payload.req.owner, 0);
    EXPECT_EQ(payload.req.func, lfs::function::write);
    EXPECT_EQ(payload.req.args.read_write_args.fd, fd);
    EXPECT_EQ(payload.req.args.read_write_args.size, 10);

    EXPECT_EQ(payload.res.ret, 10);
    EXPECT_EQ(memcmp(file_content, payload.buf, ret), 0);

    ret = libfs_->close(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed close request\n";

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestReadContent) {
    int fd = libfs_->createFile(file_name, mode);
    EXPECT_NE(fd, -1);
    std::cout << "Executed create request\n";

    fd = libfs_->open(file_name, O_RDWR);
    EXPECT_NE(fd, -1);
    std::cout << "Executed open request\n";

    int ret = libfs_->write(fd, file_content, 10);
    EXPECT_EQ(ret, 10);
    std::cout << "Executed write request\n";

    ret = libfs_->close(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed close request\n";

    fd = libfs_->open(file_name, O_RDWR);
    EXPECT_NE(fd, -1);
    std::cout << "Executed open request\n";

    char buf[10];
    ret = libfs_->read(fd, buf, 10);
    EXPECT_EQ(ret, 10);
    std::cout << "Executed read request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_NE(payload.req.owner, 0);
    EXPECT_EQ(payload.req.func, lfs::function::read);
    EXPECT_EQ(payload.req.args.read_write_args.fd, fd);
    EXPECT_EQ(payload.req.args.read_write_args.size, 10);

    EXPECT_EQ(payload.res.ret, 10);
    EXPECT_EQ(memcmp(file_content, payload.buf, ret), 0);

    ret = libfs_->close(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed close request\n";

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestNotify) {
    int fd = libfs_->createFile(file_name, mode);
    EXPECT_NE(fd, -1);
    std::cout << "Executed create request\n";

    int ret = libfs_->notify(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed notify request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_NE(payload.req.owner, 0);
    EXPECT_EQ(payload.req.func, lfs::function::notify);
    EXPECT_EQ(payload.req.args.close_notify_unnotify_args.fd, fd);

    EXPECT_EQ(payload.res.ret, 0);

    ret = libfs_->close(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed close request\n";

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestUnnotify) {
    int fd = libfs_->createFile(file_name, mode);
    EXPECT_NE(fd, -1);
    std::cout << "Executed create request\n";

    int ret_notify = libfs_->notify(fd);
    EXPECT_EQ(ret_notify, 0);
    std::cout << "Executed notify request\n";

    int ret_unnotify = libfs_->unnotify(fd);
    EXPECT_EQ(ret_unnotify, 0);
    std::cout << "Executed unnotify request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_NE(payload.req.owner, 0);
    EXPECT_EQ(payload.req.func, lfs::function::unnotify);
    EXPECT_EQ(payload.req.args.close_notify_unnotify_args.fd, fd);

    EXPECT_EQ(payload.res.ret, 0);

    int ret = libfs_->close(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed close request\n";

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestSeek) {
    int fd = libfs_->createFile(file_name, mode);
    EXPECT_NE(fd, -1);
    std::cout << "Executed create request\n";

    fd = libfs_->open(file_name, O_RDWR);
    EXPECT_NE(fd, -1);
    std::cout << "Executed open request\n";

    int ret = libfs_->write(fd, file_content, 10);
    EXPECT_EQ(ret, 10);
    std::cout << "Executed write request\n";

    ret = libfs_->seek(fd, 5);
    EXPECT_NE(ret, -1);
    std::cout << "Executed seek request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_NE(payload.req.owner, 0);
    EXPECT_EQ(payload.req.func, lfs::function::seek);
    EXPECT_EQ(payload.req.args.seek_args.fd, fd);
    EXPECT_EQ(payload.req.args.seek_args.offset, 5);

    EXPECT_NE(payload.res.ret, -1);

    char buf[10];
    ret = libfs_->read(fd, buf, 10);
    EXPECT_EQ(ret, 5);
    std::cout << "Executed read request\n";

    payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_EQ(payload.res.ret, 5);
    EXPECT_EQ(memcmp(file_content + 5, payload.buf, ret), 0);

    ret = libfs_->close(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed close request\n";

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestRename) {
    int fd = libfs_->createFile(file_name, mode);
    EXPECT_NE(fd, -1);
    std::cout << "Executed create request\n";

    int ret = libfs_->rename(file_name, file_name2);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed rename request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_NE(payload.req.owner, 0);
    EXPECT_EQ(payload.req.func, lfs::function::rename);
    EXPECT_STREQ(payload.req.args.rename_args.oldname, file_name);
    EXPECT_STREQ(payload.req.args.rename_args.newname, file_name2);

    EXPECT_EQ(payload.res.ret, 0);

    ret = libfs_->unlink(file_name2);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestChmode) {
    int fd = libfs_->createFile(file_name, mode);
    EXPECT_NE(fd, -1);
    std::cout << "Executed create request\n";

    int ret = libfs_->chmode(file_name, mode2);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed chmode request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_NE(payload.req.owner, 0);
    EXPECT_EQ(payload.req.func, lfs::function::chmode);
    EXPECT_STREQ(payload.req.args.create_chmode_args.name, file_name);
    EXPECT_EQ(payload.req.args.create_chmode_args.mode, mode2);

    EXPECT_EQ(payload.res.ret, 0);

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestCreateExisting) {
    int ret = libfs_->createFile(file_name, mode);
    EXPECT_NE(ret, -1);
    std::cout << "Executed first create request\n";

    ret = libfs_->createFile(file_name, mode);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, EEXIST);
    std::cout << "Executed second create request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_EQ(payload.res.ret, -1);
    EXPECT_EQ(payload.res.lfs_errno, EEXIST);

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestCreateNameTooLong) {
    int ret = libfs_->createFile(too_long, mode);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, ENAMETOOLONG);
    std::cout << "Executed create request\n";
}

TEST_F(DaemonSuite, TestOpenNonExistent) {
    int ret = libfs_->open(nonexistent, O_RDWR);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, ENOENT);
    std::cout << "Executed open request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_EQ(payload.res.ret, -1);
    EXPECT_EQ(payload.res.lfs_errno, ENOENT);
}

TEST_F(DaemonSuite, TestOpenNotAllowed) {
    int ret = libfs_->createFile(file_name, 0000);
    EXPECT_NE(ret, -1);
    std::cout << "Executed create request\n";

    ret = libfs_->open(file_name, O_RDWR);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, EACCES);
    std::cout << "Executed open request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_EQ(payload.res.ret, -1);
    EXPECT_EQ(payload.res.lfs_errno, EACCES);

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestOpenTooLong) {
    int ret = libfs_->open(too_long, O_RDWR);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, ENAMETOOLONG);
    std::cout << "Executed open request\n";
}

TEST_F(DaemonSuite, TestCloseFDNonExistent) {
    int ret = libfs_->close(-1);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, EBADF);
    std::cout << "Executed close request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_EQ(payload.res.ret, -1);
    EXPECT_EQ(payload.res.lfs_errno, EBADF);
}

TEST_F(DaemonSuite, TestUnlinkNonExistent) {
    int ret = libfs_->unlink(nonexistent);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, ENOENT);
    std::cout << "Executed unlink request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_EQ(payload.res.ret, -1);
    EXPECT_EQ(payload.res.lfs_errno, ENOENT);
}

TEST_F(DaemonSuite, TestReadFDNonExistent) {
    char buf[10];
    int ret = libfs_->read(-1, buf, 10);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, EBADF);
    std::cout << "Executed read request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_EQ(payload.res.ret, -1);
    EXPECT_EQ(payload.res.lfs_errno, EBADF);
}

TEST_F(DaemonSuite, TestReadNotOpen) {
    int fd = libfs_->createFile(file_name, mode);
    EXPECT_NE(fd, -1);
    std::cout << "Executed create request\n";

    int ret = libfs_->close(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed close request\n";

    char buf[10];
    ret = libfs_->read(fd, buf, 10);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, EBADF);
    std::cout << "Executed read request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_EQ(payload.res.ret, -1);
    EXPECT_EQ(payload.res.lfs_errno, EBADF);

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestReadWronly) {
    int fd = libfs_->createFile(file_name, mode);
    EXPECT_NE(fd, -1);
    std::cout << "Executed create request\n";

    fd = libfs_->open(file_name, O_WRONLY);
    EXPECT_NE(fd, -1);
    std::cout << "Executed open request\n";

    char buf[10];
    int ret = libfs_->read(fd, buf, 10);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, EBADF);
    std::cout << "Executed read request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_EQ(payload.res.ret, -1);
    EXPECT_EQ(payload.res.lfs_errno, EBADF);

    ret = libfs_->close(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed close request\n";

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestWriteFDNonExistent) {
    int ret = libfs_->write(-1, file_content, 10);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, EBADF);
    std::cout << "Executed write request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_EQ(payload.res.ret, -1);
    EXPECT_EQ(payload.res.lfs_errno, EBADF);
}

TEST_F(DaemonSuite, TestWriteNotOpen) {
    int fd = libfs_->createFile(file_name, mode);
    EXPECT_NE(fd, -1);
    std::cout << "Executed create request\n";

    int ret = libfs_->close(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed close request\n";

    ret = libfs_->write(fd, file_content, 10);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, EBADF);
    std::cout << "Executed write request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_EQ(payload.res.ret, -1);
    EXPECT_EQ(payload.res.lfs_errno, EBADF);

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestWriteRdonly) {
    int fd = libfs_->createFile(file_name, mode);
    EXPECT_NE(fd, -1);
    std::cout << "Executed create request\n";

    fd = libfs_->open(file_name, O_RDONLY);
    EXPECT_NE(fd, -1);
    std::cout << "Executed open request\n";

    int ret = libfs_->write(fd, file_content, 10);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, EBADF);
    std::cout << "Executed write request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_EQ(payload.res.ret, -1);
    EXPECT_EQ(payload.res.lfs_errno, EBADF);

    ret = libfs_->close(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed close request\n";

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestSeekFDNonExistent) {
    int ret = libfs_->seek(-1, 5);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, EBADF);
    std::cout << "Executed seek request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_EQ(payload.res.ret, -1);
    EXPECT_EQ(payload.res.lfs_errno, EBADF);
}

TEST_F(DaemonSuite, TestSeekNegativePosition) {
    int fd = libfs_->createFile(file_name, mode);
    EXPECT_NE(fd, -1);
    std::cout << "Executed create request\n";

    int ret = libfs_->seek(fd, -5);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, EINVAL);
    std::cout << "Executed seek request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_EQ(payload.res.ret, -1);
    EXPECT_EQ(payload.res.lfs_errno, EINVAL);

    ret = libfs_->close(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed close request\n";

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestRenameNonExistent) {
    int ret = libfs_->rename(nonexistent, file_name);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, ENOENT);
    std::cout << "Executed rename request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_EQ(payload.res.ret, -1);
    EXPECT_EQ(payload.res.lfs_errno, ENOENT);
}

TEST_F(DaemonSuite, TestRenameToTooLong) {
    int fd = libfs_->createFile(file_name, mode);
    EXPECT_NE(fd, -1);
    std::cout << "Executed create request\n";

    int ret = libfs_->rename(file_name, too_long);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, ENAMETOOLONG);
    std::cout << "Executed rename request\n";

    ret = libfs_->close(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed close request\n";

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestRenameFromTooLong) {
    int ret = libfs_->rename(too_long, file_name);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, ENAMETOOLONG);
    std::cout << "Executed rename request\n";
}

TEST_F(DaemonSuite, TestChmodeNonExistent) {
    int ret = libfs_->chmode(nonexistent, mode);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, ENOENT);
    std::cout << "Executed chmode request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_EQ(payload.res.ret, -1);
    EXPECT_EQ(payload.res.lfs_errno, ENOENT);
}

TEST_F(DaemonSuite, TestChmodeNameTooLong) {
    int ret = libfs_->chmode(too_long, mode);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, ENAMETOOLONG);
    std::cout << "Executed chmode request\n";
}

TEST_F(DaemonSuite, TestNotifyAlreadyDone) {
    int fd = libfs_->createFile(file_name, mode);
    EXPECT_NE(fd, -1);
    std::cout << "Executed create request\n";

    int ret = libfs_->notify(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed first notify request\n";

    ret = libfs_->notify(fd);
    EXPECT_EQ(ret, 1);
    std::cout << "Executed second notify request\n";

    ret = libfs_->close(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed close request\n";

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestNotifyFDNonExistent) {
    int ret = libfs_->notify(-1);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, EBADF);
    std::cout << "Executed notify request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_EQ(payload.res.ret, -1);
    EXPECT_EQ(payload.res.lfs_errno, EBADF);
}

TEST_F(DaemonSuite, TestUnnotifyAlreadyDone) {
    int fd = libfs_->createFile(file_name, mode);
    EXPECT_NE(fd, -1);
    std::cout << "Executed create request\n";

    int ret = libfs_->notify(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed first unnotify request\n";

    ret = libfs_->unnotify(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed first unnotify request\n";

    ret = libfs_->unnotify(fd);
    EXPECT_EQ(ret, 1);
    std::cout << "Executed second unnotify request\n";

    ret = libfs_->close(fd);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed close request\n";

    ret = libfs_->unlink(file_name);
    EXPECT_EQ(ret, 0);
    std::cout << "Executed unlink request\n";
}

TEST_F(DaemonSuite, TestUnnotifyFDNonExistent) {
    int ret = libfs_->unnotify(-1);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(libfs_->lfs_errno, EBADF);
    std::cout << "Executed unnotify request\n";

    void *shm_addr = shmat(shm_id, nullptr, 0);
    lfs::shm_payload payload = *(lfs::shm_payload *) shm_addr;

    EXPECT_EQ(payload.res.ret, -1);
    EXPECT_EQ(payload.res.lfs_errno, EBADF);
}
