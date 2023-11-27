#include "io_uring_backed_io.h"

#include <stdio.h>

struct io_uring g_io_uring;

void io_uring_infra_init(void)
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
}

void io_uring_infra_deinit(void)
{
    io_uring_queue_exit(&g_io_uring);
    printf("deinit success!\n");
}
