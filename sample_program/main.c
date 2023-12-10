#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

int main(int argc, char* argv[])
{
    pid_t pid = getpid();

    const char* filename = "sample.txt";
    remove(filename);

    int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC);

    // generate a random number to write
    char* text = "random number generated for run = ";
    srand(time(NULL));
    int random = rand();
    char write_buffer[100] = { 0 };
    memcpy(write_buffer, text, strlen(text));
    sprintf(&write_buffer[strlen(text)], "%d", random);
    write_buffer[strlen(write_buffer)] = '\n';

    write(fd, write_buffer, strlen(write_buffer));

    fsync(fd);

    lseek(fd, 0, SEEK_SET);

    char buffer[100] = { 0 };
    read(fd, buffer, sizeof(buffer) - 1);

    printf("Read from file: %s\n", buffer);

    close(fd);

    return 0;
}
