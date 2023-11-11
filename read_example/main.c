#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <liburing.h>

//#define ALIGN_MEMORY

struct io_uring g_ring;

const int MAX_BLOCK_SIZE = 256;

void emplace_request(int fd, int num_bytes, int offset)
{
    struct iovec* request = calloc(1, sizeof(struct iovec));
    request->iov_len = num_bytes;
#ifdef ALIGN_MEMORY
    // make sure memory is alligned on a power of 2 
    posix_memalign(&request->iov_base, MAX_BLOCK_SIZE, num_bytes);
#else
    request->iov_base = calloc(num_bytes, sizeof(char));
#endif
    struct io_uring_sqe* sqe = io_uring_get_sqe(&g_ring);
    io_uring_prep_readv(sqe, fd, request, 1, offset);
    io_uring_sqe_set_data(sqe, request); 
    io_uring_submit(&g_ring);
}

struct iovec* read_response()
{
    struct io_uring_cqe *cqe;
    int status = io_uring_wait_cqe(&g_ring, &cqe);
    struct iovec* ret = io_uring_cqe_get_data(cqe);
    io_uring_cqe_seen(&g_ring, cqe);
    return ret;
}

int main(int argc, char* argv[])
{
    // https://man7.org/linux/man-pages/man2/io_uring_setup.2.html
    // SQPOLL requires to run in sudo for kernel <= 5.11
    int flags = IORING_SETUP_SQPOLL; // Create kernel thread
    int ring_fd = io_uring_queue_init(10, &g_ring, flags);
    if (ring_fd < 0)
    {
        printf("Unable to instantiate io_uring w/ error code %d\n", ring_fd);
        return -1;
    }

    int fd = open("sample.txt", O_RDONLY);

    // https://man7.org/linux/man-pages/man2/io_uring_setup.2.html
    // required to register all files when running kernel <= 5.11
    int status = io_uring_register_files(&g_ring, &fd, 1);
    if (status < 0)
    {
        printf("Unable to register file with io_uring w/ error code %d\n", status);
        return -1;
    }

    emplace_request(fd, 187, 0);

    struct iovec* ret = read_response();

    close(fd);

    char* temp = ret->iov_base;
    for(int i = 0; i < ret->iov_len; ++i)
    {
        fputc(temp[i], stdout);
    }
    printf("\n");

    free(ret);

    return 0;
}
