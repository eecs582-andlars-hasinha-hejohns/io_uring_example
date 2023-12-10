#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


int main(int argc, char* argv[])
{
    for (int i = 0; i < 1<<22; ++i)
    {
        int fd = open("sample.txt", O_RDONLY);
	close(fd);
    }

    return 0;
}
