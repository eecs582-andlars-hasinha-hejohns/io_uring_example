#include <stdio.h>
#include <liburing.h>

int main(int argc, char* argv[])
{
    struct io_uring ring;

    io_uring_queue_init(10, &ring, 0);

    return 0;
}
