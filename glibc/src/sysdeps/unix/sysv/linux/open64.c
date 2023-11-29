/* Linux open syscall implementation, LFS.
   Copyright (C) 1991-2023 Free Software Foundation, Inc.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sysdep-cancel.h>
#include <shlib-compat.h>

#include "io_uring_backed_io.h"

/* Open FILE with access OFLAG.  If O_CREAT or O_TMPFILE is in OFLAG,
   a third argument is the file protection.  */
int
__libc_open64 (const char *file, int oflag, ...)
{
  int mode = 0;

  if (__OPEN_NEEDS_MODE (oflag))
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

strong_alias (__libc_open64, __open64)
libc_hidden_weak (__open64)
weak_alias (__libc_open64, open64)

#ifdef __OFF_T_MATCHES_OFF64_T
strong_alias (__libc_open64, __libc_open)
strong_alias (__libc_open64, __open)
libc_hidden_weak (__open)
weak_alias (__libc_open64, open)
#endif

#if OTHER_SHLIB_COMPAT (libpthread, GLIBC_2_1, GLIBC_2_2)
compat_symbol (libc, __libc_open64, open64, GLIBC_2_2);
#endif
