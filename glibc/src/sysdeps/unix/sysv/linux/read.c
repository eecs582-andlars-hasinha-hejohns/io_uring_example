/* Linux read syscall implementation.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <unistd.h>
#include <sysdep-cancel.h>

#include "io_uring_backed_io.h"

/* Read NBYTES into BUF from FD.  Return the number read or -1.  */
ssize_t
__libc_read (int fd, void *buf, size_t nbytes)
{
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
  if (cqe->res < 0)
  {
    ret = cqe->res;
  }
  io_uring_cqe_seen(&g_io_uring, cqe);

  return ret;
}
libc_hidden_def (__libc_read)

libc_hidden_def (__read)
weak_alias (__libc_read, __read)
libc_hidden_def (read)
weak_alias (__libc_read, read)
