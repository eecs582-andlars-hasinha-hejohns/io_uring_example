#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <sys/wait.h>
#endif
#include "monkey.h"
#include <liburing.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <cstddef>

// for some reason dynamic userspace probes (uprobe) isn't playing well, so
// we're using static userspace (usdt) probes
#include <sys/sdt.h>

// C++
#include <iostream>
#include <mutex>

// for open
# define __CHECK_OPEN_NEEDS_MODE(oflag) (((oflag) & O_CREAT) != 0 || ((oflag) & __O_TMPFILE) == __O_TMPFILE)

static bool monkey_initialized = false; // need to catch the first person who uses a uring
static int monkey_shared_wq_fd; // see IORING_SETUP_ATTACH_WQ
// io_uring interface is not thread safe
// and axboe strongly advises using 1 ring/thread
thread_local static bool monkey_thread_initialized = 0;
thread_local static struct io_uring monkey_thread_uring;

// called when shared library is unloaded
void io_uring_infra_deinit(void)
{
    // NOTE: this segfaults, so let's leave it out indefinitely
    //io_uring_queue_exit(&g_io_uring);
}

int io_uring_infra_init(void)
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
        if(monkey_initialized){
            p.wq_fd = monkey_shared_wq_fd;
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
        int succ = io_uring_queue_init_params(1<<10, &monkey_thread_uring, &p);
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
            if(!monkey_initialized){
                monkey_initialized = true;
                monkey_shared_wq_fd = monkey_thread_uring.ring_fd;
            }
            monkey_thread_initialized = 1;
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

// need to initialize the thread_local uring if it's the thread's first time
// through monkey
int
monkey_init_thread_if_needed(){
  if(!monkey_thread_initialized){
    int res = io_uring_infra_init();
    if(res){
      // errno should've been set by io_uring_infra_init
      return res;
    }
  }
  return 0;
}

/* Open FILE with access OFLAG.  If O_CREAT or O_TMPFILE is in OFLAG,
   a third argument is the file protection.  */
int
open (const char *file, int oflag, ...)
{
    va_list args;
    va_start(args, oflag);
    int mode = 0;
    if (__CHECK_OPEN_NEEDS_MODE (oflag))
    {
        mode = va_arg (args, int);
    }
    DTRACE_PROBE3(monkey, open_entry, file, oflag, mode);
    auto ret = monkey_open(file, oflag, mode);
    DTRACE_PROBE3(monkey, open_exit, file, oflag, mode);
    va_end(args);
    return ret;
}


// wait for completion
int wait_for_completion(){
  struct io_uring_cqe *cqe;
  int res = io_uring_wait_cqe(&monkey_thread_uring, &cqe);
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

    io_uring_cqe_seen(&monkey_thread_uring, cqe);

    return ret;
  }
}

int
monkey_open(const char *file, int oflag, int mode){
    {
        auto ret = monkey_init_thread_if_needed();
        if(ret){
            return -1;
        }
    }

  // emplace request
  struct io_uring_sqe* sqe = io_uring_get_sqe(&monkey_thread_uring);
  assert(sqe != NULL);

  io_uring_prep_openat(sqe, AT_FDCWD, file, oflag | O_LARGEFILE, mode);
  int res = io_uring_submit(&monkey_thread_uring);
  assert(res > 0);
  return wait_for_completion();
}

/* Read NBYTES into BUF from FD.  Return the number read or -1.  */
ssize_t
read (int fd, void *buf, size_t nbytes)
{
    DTRACE_PROBE3(monkey, read_enter, fd, buf, nbytes);
    auto ret = monkey_read(fd, buf, nbytes);
    DTRACE_PROBE3(monkey, read_exit, fd, buf, nbytes);
    return ret;
}

// to attach a uprobe to, except it doesn't really work with bpftrace and LD_PRELOAD as is
ssize_t
monkey_read(int fd, void *buf, size_t nbytes)
{
    {
        auto ret = monkey_init_thread_if_needed();
        if(ret){
            return -1;
        }
    }
  ssize_t ret = nbytes;

  // emplace request
  struct io_uring_sqe* sqe = io_uring_get_sqe(&monkey_thread_uring);
  io_uring_prep_read(sqe, fd, buf, nbytes, -1);
  int res = io_uring_submit(&monkey_thread_uring);
  assert(res >= 0);
  return wait_for_completion();
}

/* Write NBYTES of BUF to FD.  Return the number written, or -1.  */
ssize_t
write (int fd, const void *buf, size_t nbytes)
{
    DTRACE_PROBE3(monkey, write_enter, fd, buf, nbytes);
    auto ret = monkey_write(fd, buf, nbytes);
    DTRACE_PROBE3(monkey, write_exit, fd, buf, nbytes);
    return ret;
}

ssize_t
monkey_write(int fd, const void *buf, size_t nbytes){
    {
        auto ret = monkey_init_thread_if_needed();
        if(ret){
            return -1;
        }
    }
  ssize_t ret = nbytes;

  // emplace request
  struct iovec request;
  request.iov_base = (void*)buf;
  request.iov_len  = nbytes;
  struct io_uring_sqe* sqe = io_uring_get_sqe(&monkey_thread_uring);
  io_uring_prep_writev(sqe, fd, &request, 1, -1);
  int res = io_uring_submit(&monkey_thread_uring);
  assert(res >= 0);
  return wait_for_completion();
}

/* Make all changes done to FD actually appear on disk.  */
int
fsync (int fd)
{
    DTRACE_PROBE1(monkey, fsync_enter, fd);
    auto ret = monkey_fsync(fd);
    DTRACE_PROBE1(monkey, fsync_exit, fd);
    return ret;
}

int
monkey_fsync(int fd){
    {
        auto ret = monkey_init_thread_if_needed();
        if(ret){
            return -1;
        }
    }
  // emplace request
  struct io_uring_sqe* sqe = io_uring_get_sqe(&monkey_thread_uring);
  io_uring_prep_fsync(sqe, fd, 0);
  int res = io_uring_submit(&monkey_thread_uring);
  assert(res >= 0);
  return wait_for_completion();
}


/* Close the file descriptor FD.  */
int
close (int fd)
{
    DTRACE_PROBE1(monkey, close_enter, fd);
    auto ret = monkey_close(fd);
    DTRACE_PROBE1(monkey, close_exit, fd);
    return ret;
}

int
monkey_close(int fd){
    {
        auto ret = monkey_init_thread_if_needed();
        if(ret){
            return -1;
        }
    }
  // emplace request
  struct io_uring_sqe* sqe = io_uring_get_sqe(&monkey_thread_uring);
  io_uring_prep_close(sqe, fd);
  int res = io_uring_submit(&monkey_thread_uring);
  return wait_for_completion();
}
