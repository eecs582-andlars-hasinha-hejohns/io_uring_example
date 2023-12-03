#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <liburing.h>

# define __CHECK_OPEN_NEEDS_MODE(oflag) \
  (((oflag) & O_CREAT) != 0 || ((oflag) & __O_TMPFILE) == __O_TMPFILE)

static int g_io_uring_initialized;
struct io_uring g_io_uring;

void io_uring_infra_init(void)
{
    int flags = IORING_SETUP_SQPOLL;
    struct io_uring_params p;
    p.sq_thread_idle = 1<<30;
    p.flags = flags;
    int ring_fd = io_uring_queue_init_params(8, &g_io_uring, &p);
    if (ring_fd < 0)
    {
        printf("failure to init io_uring\n");
    }
    else
    {
        printf("init success!\n");
    }
    g_io_uring_initialized = 1;
}

//__attribute__((at_exit))
void io_uring_infra_deinit(void)
{
    io_uring_queue_exit(&g_io_uring);
    printf("deinit success!\n");
}

/* Open FILE with access OFLAG.  If O_CREAT or O_TMPFILE is in OFLAG,
   a third argument is the file protection.  */
int
open64 (const char *file, int oflag, ...)
{
  int mode = 0;

  if (__CHECK_OPEN_NEEDS_MODE (oflag))
    {
      va_list arg;
      va_start (arg, oflag);
      mode = va_arg (arg, int);
      va_end (arg);
    }

  // emplace request
  struct io_uring_sqe* sqe = io_uring_get_sqe(&g_io_uring);
  io_uring_prep_openat(sqe, AT_FDCWD, file, oflag | O_LARGEFILE, mode);
  io_uring_submit(&g_io_uring);

  // wait for completion
  struct io_uring_cqe *cqe;
  io_uring_wait_cqe(&g_io_uring, &cqe);
  int ret = cqe->res;
  io_uring_cqe_seen(&g_io_uring, cqe);

  return ret;
}

/* Read NBYTES into BUF from FD.  Return the number read or -1.  */
ssize_t
read (int fd, void *buf, size_t nbytes)
{
  if(!g_io_uring_initialized){
      io_uring_infra_init();
  }
  ssize_t ret = nbytes;

  // emplace request
  struct iovec request;
  request.iov_base = buf;
  request.iov_len  = nbytes;
  struct io_uring_sqe* sqe = io_uring_get_sqe(&g_io_uring);
  io_uring_prep_readv(sqe, fd, &request, 1, -1);
  io_uring_submit(&g_io_uring);

  // wait for completion
  struct io_uring_cqe *cqe;
  io_uring_wait_cqe(&g_io_uring, &cqe);
  ret = cqe->res;

  io_uring_cqe_seen(&g_io_uring, cqe);
  //char buf2[ret + 1];
  //memset(buf2, 0, ret + 1);
  //memcpy(buf2, buf, ret);
  //printf("[straceish]: read {fd = %d, buf = [%s]} = %ld\n", fd, buf2, ret);
  //printf("[read] {fd = %d } = %ld\n", fd, ret);

  return ret;
}

/* Write NBYTES of BUF to FD.  Return the number written, or -1.  */
ssize_t
write (int fd, const void *buf, size_t nbytes)
{
  if(!g_io_uring_initialized){
      io_uring_infra_init();
  }
  ssize_t ret = nbytes;

  // emplace request
  struct iovec request;
  request.iov_base = (void*)buf;
  request.iov_len  = nbytes;
  struct io_uring_sqe* sqe = io_uring_get_sqe(&g_io_uring);
  io_uring_prep_writev(sqe, fd, &request, 1, -1);
  io_uring_submit(&g_io_uring);

  // wait for completion
  struct io_uring_cqe *cqe;
  io_uring_wait_cqe(&g_io_uring, &cqe);
  ret = cqe->res;
  io_uring_cqe_seen(&g_io_uring, cqe);

  //char buf2[ret + 1];
  //memset(buf2, 0, ret + 1);
  //memcpy(buf2, buf, ret);
  //printf("[straceish]: write {fd = %d, buf = [%s]} = %ld\n", fd, buf2, ret);

  return ret;
}

/* Make all changes done to FD actually appear on disk.  */
int
fsync (int fd)
{
  // emplace request
  struct io_uring_sqe* sqe = io_uring_get_sqe(&g_io_uring);
  io_uring_prep_fsync(sqe, fd, 0);
  io_uring_submit(&g_io_uring);

  // wait for completion
  struct io_uring_cqe *cqe;
  io_uring_wait_cqe(&g_io_uring, &cqe);
  int ret = cqe->res;
  io_uring_cqe_seen(&g_io_uring, cqe);

  return ret;
}


/* Close the file descriptor FD.  */
int
close (int fd)
{
  // emplace request
  struct io_uring_sqe* sqe = io_uring_get_sqe(&g_io_uring);
  io_uring_prep_close(sqe, fd);
  io_uring_submit(&g_io_uring);

  // wait for completion
  struct io_uring_cqe *cqe;
  io_uring_wait_cqe(&g_io_uring, &cqe);
  int ret = cqe->res;
  io_uring_cqe_seen(&g_io_uring, cqe);

  return ret;
}
