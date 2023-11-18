#ifndef IO_URING_BACKED_IO
#define IO_URING_BACKED_IO

#include <stdio.h>

#include "liburing.h"

static int g_constructor_has_run = 0;

struct io_uring g_io_uring;

static void __attribute__((constructor)) io_uring_infra_init(void)
{
    if (g_constructor_has_run == 0)
    {
        int flags = IORING_SETUP_SQPOLL;
        int ring_fd = io_uring_queue_init(8, &g_io_uring, flags);
        if (ring_fd < 0)
        {
            printf("failure to init io_uring\n");
        }
        else
        {
            printf("init success!\n");
        }
        g_constructor_has_run = 1;
    }

}

static void __attribute__((destructor)) io_uring_infra_deinit(void)
{
    if (g_constructor_has_run == 1)
    {
        io_uring_queue_exit(&g_io_uring);
        printf("deinit success!\n");
        g_constructor_has_run = 0;
    }
}

#endif // IO_URING_BACKED_IO