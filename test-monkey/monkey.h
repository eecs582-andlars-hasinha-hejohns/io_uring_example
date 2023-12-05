extern "C" {
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>
    // put the most important things here
    int open (const char *file, int oflag, ...);
    ssize_t read (int fd, void *buf, size_t nbytes);
    ssize_t write (int fd, const void *buf, size_t nbytes);
    int fsync (int fd);
    int close (int fd);

    // optional
    int monkey_open(const char *file, int oflag, int mode);
    ssize_t monkey_read(int fd, void *buf, size_t nbytes);
    ssize_t monkey_write(int, const void *, size_t);
    int monkey_fsync(int fd);
    int monkey_close(int fd);

    void io_uring_infra_deinit(void);
    int io_uring_infra_init(void);
    int monkey_init_thread_if_needed();
}
