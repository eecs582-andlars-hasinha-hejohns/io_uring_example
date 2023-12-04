#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <liburing/io_uring.h>
#include <unistd.h>
#endif
//#include <liburing/io_uring.h>

#include <stdio.h>
#include <stdarg.h>
#include <liburing.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <iostream>

#include <mutex>

# define __CHECK_OPEN_NEEDS_MODE(oflag) (((oflag) & O_CREAT) != 0 || ((oflag) & __O_TMPFILE) == __O_TMPFILE)

static bool has_anyone_initialized_a_uring = 0;
static int if_someone_has_this_is_their_fd;
thread_local static bool g_io_uring_initialized = 0;
thread_local static struct io_uring g_io_uring;

// called when shared library is unloaded
extern "C" void io_uring_infra_deinit(void)
{
    // NOTE: this segfaults
    //io_uring_queue_exit(&g_io_uring);
}

extern "C" int io_uring_infra_init(void)
{
    struct io_uring_params p;

    // The resv field must be zeroed.    
    assert(memset(&p, 0, sizeof(p)));

    // This doesn't do anything if we never look at io_sq_ring.
    p.sq_thread_idle = 1<<30;

    p.flags |= IORING_SETUP_SQPOLL;
    static std::mutex m;
    {
        std::lock_guard<std::mutex> l(m);
        if(has_anyone_initialized_a_uring){
            p.wq_fd = if_someone_has_this_is_their_fd;
#ifndef NDEBUG
#ifdef DEBUG
            std::cout << "p.wq_fd" << p.wq_fd << std::endl;
#endif
#endif
            p.flags |= IORING_SETUP_ATTACH_WQ;
#ifndef NDEBUG
#ifdef DEBUG
            std::cout << "got here: " << __LINE__ << std::endl;
#endif
#endif
        }
#ifndef NDEBUG
#ifdef DEBUG
        std::cout << "&g_io_uring: " << &g_io_uring << std::endl;
#endif
#endif
        int succ = io_uring_queue_init_params(1<<10, &g_io_uring, &p);
        if (succ)
        {
            if (succ == -EINTR){
                // User's problem.
                return succ;
            }
            else {
                printf("failure to init io_uring\n");
                printf("exiting...\n");
                printf("The failure code was %d\n", succ);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            if(!has_anyone_initialized_a_uring){
                has_anyone_initialized_a_uring = true;
                if_someone_has_this_is_their_fd = g_io_uring.ring_fd;
            }
            g_io_uring_initialized = 1;
            if(atexit(io_uring_infra_deinit)){
                perror("atexit");
                exit(1);
            }
            //syscall();
#ifndef NDEBUG
#ifdef DEBUG
            std::cout << __FUNCTION__ << ": thread: " << &g_io_uring << std::endl;;
#endif
#endif
            return 0;
        }
    }
}

/* Open FILE with access OFLAG.  If O_CREAT or O_TMPFILE is in OFLAG,
   a third argument is the file protection.  */
extern "C" int
open (const char *file, int oflag, ...)
{
  if(!g_io_uring_initialized){
    int res = io_uring_infra_init();
    if (res == -EINTR) {
        // User's problem.
        return res;
    }
  }
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
  assert(sqe != NULL);

  io_uring_prep_openat(sqe, AT_FDCWD, file, oflag | O_LARGEFILE, mode);
  int res = io_uring_submit(&g_io_uring);
  assert(res > 0);

  // wait for completion
  struct io_uring_cqe *cqe;
  res = io_uring_wait_cqe(&g_io_uring, &cqe);
  if (res == -EINTR) {
    // If the syscall was interrupted, the user needs to deal with it.
    return res;
  }
  else if (res != 0) {
    // If the syscall fails for other reasons, it's our problem. 
    exit(res);
  }
  else {
    int ret = cqe->res;

    io_uring_cqe_seen(&g_io_uring, cqe);

    return ret;
  }
}

/* Read NBYTES into BUF from FD.  Return the number read or -1.  */
extern "C" ssize_t
read (int fd, void *buf, size_t nbytes)
{
  if(!g_io_uring_initialized){
    int res = io_uring_infra_init();
    if (res == -EINTR) {
        // User's problem.
        return res;
    }
  }
  ssize_t ret = nbytes;

  // emplace request
  struct io_uring_sqe* sqe = io_uring_get_sqe(&g_io_uring);
  io_uring_prep_read(sqe, fd, buf, nbytes, -1);
  int res = io_uring_submit(&g_io_uring);
  assert(res >= 0);

  // wait for completion
  struct io_uring_cqe *cqe;
  res = io_uring_wait_cqe(&g_io_uring, &cqe);
  if (res == -EINTR) {
    // If the syscall was interrupted, the user needs to deal with it.
    return res;
  }
  else if (res != 0) {
    // If the syscall fails for other reasons, it's our problem. 
    exit(res);
  }
  else {
    ret = cqe->res;

    io_uring_cqe_seen(&g_io_uring, cqe);

    return ret;
  }
}

/* Write NBYTES of BUF to FD.  Return the number written, or -1.  */
ssize_t
write (int fd, const void *buf, size_t nbytes)
{
  if(!g_io_uring_initialized){
    int res = io_uring_infra_init();
    if (res == -EINTR) {
        // User's problem.
        return res;
    }
  }
  ssize_t ret = nbytes;

  // emplace request
  struct iovec request;
  request.iov_base = (void*)buf;
  request.iov_len  = nbytes;
  struct io_uring_sqe* sqe = io_uring_get_sqe(&g_io_uring);
  io_uring_prep_writev(sqe, fd, &request, 1, -1);
  int res = io_uring_submit(&g_io_uring);
  assert(res >= 0);

  // wait for completion
  struct io_uring_cqe *cqe;
  res = io_uring_wait_cqe(&g_io_uring, &cqe);
  if (res == -EINTR) {
    // If the syscall was interrupted, the user needs to deal with it.
    return res;
  }
  else if (res != 0) {
    // If the syscall fails for other reasons, it's our problem. 
    exit(res);
  }
  else {
    ret = cqe->res;

    io_uring_cqe_seen(&g_io_uring, cqe);

    return ret;
  }
}

/* Make all changes done to FD actually appear on disk.  */
int
fsync (int fd)
{
  if(!g_io_uring_initialized){
    int res = io_uring_infra_init();
    if (res == -EINTR) {
        // User's problem.
        return res;
    }
  }
  // emplace request
  struct io_uring_sqe* sqe = io_uring_get_sqe(&g_io_uring);
  io_uring_prep_fsync(sqe, fd, 0);
  int res = io_uring_submit(&g_io_uring);
  assert(res >= 0);

  // wait for completion
  struct io_uring_cqe *cqe;
  res = io_uring_wait_cqe(&g_io_uring, &cqe);
  if (res == -EINTR) {
    // If the syscall was interrupted, the user needs to deal with it.
    return res;
  }
  else if (res != 0) {
    // If the syscall fails for other reasons, it's our problem. 
    exit(res);
  }
  else {
    int ret = cqe->res;

    io_uring_cqe_seen(&g_io_uring, cqe);

    return ret;
  }
}


/* Close the file descriptor FD.  */
int
close (int fd)
{
  if(!g_io_uring_initialized){
    int res = io_uring_infra_init();
    if (res == -EINTR) {
        // User's problem.
        return res;
    }
  }

  // emplace request
  struct io_uring_sqe* sqe = io_uring_get_sqe(&g_io_uring);
  io_uring_prep_close(sqe, fd);
  int res = io_uring_submit(&g_io_uring);

  // wait for completion
  struct io_uring_cqe *cqe;
  res = io_uring_wait_cqe(&g_io_uring, &cqe);
  if (res == -EINTR) {
    // If the syscall was interrupted, the user needs to deal with it.
    return res;
  }
  else if (res != 0) {
    // If the syscall fails for other reasons, it's our problem. 
    exit(res);
  }
  else {
    int ret = cqe->res;

    io_uring_cqe_seen(&g_io_uring, cqe);

    return ret;
  }
}
