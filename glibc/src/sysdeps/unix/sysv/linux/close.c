/* Linux close syscall implementation.
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
#include <not-cancel.h>

#include "io_uring_backed_io.h"

/* Close the file descriptor FD.  */
int
__close (int fd)
{
  int ret = 0;

  // emplace request
  struct io_uring_sqe* sqe = io_uring_get_sqe(&g_io_uring);
  io_uring_prep_close(sqe, fd);
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
libc_hidden_def (__close)
strong_alias (__close, __libc_close)
weak_alias (__close, close)
