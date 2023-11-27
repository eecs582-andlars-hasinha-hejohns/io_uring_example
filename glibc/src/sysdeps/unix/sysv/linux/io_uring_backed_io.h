#ifndef IO_URING_BACKED_IO
#define IO_URING_BACKED_IO

#include "liburing.h"

extern struct io_uring g_io_uring;

void __attribute__((constructor)) io_uring_infra_init(void);

void __attribute__((destructor)) io_uring_infra_deinit(void);

#endif // IO_URING_BACKED_IO
