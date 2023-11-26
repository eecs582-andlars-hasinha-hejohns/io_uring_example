#ifndef IO_URING_BACKED_IO
#define IO_URING_BACKED_IO

#include <stdio.h>

#include "liburing.h"

extern struct io_uring g_io_uring;

void io_uring_infra_init_impl(void);

void io_uring_infra_deinit_impl(void);

static void __attribute__((constructor)) io_uring_infra_init(void)
{
    io_uring_infra_init_impl();
}

static void __attribute__((destructor)) io_uring_infra_deinit(void)
{
    io_uring_infra_deinit_impl();
}

#endif // IO_URING_BACKED_IO
