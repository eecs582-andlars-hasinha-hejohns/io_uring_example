#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char* argv[])
{
    pid_t pid = getpid();

    int fd = open("a_sample.txt", O_RDWR | O_CREAT);
    
    const char* text = "hello, world\n";
    write(fd, text, strlen(text));
    
    fsync(fd);
    
    lseek(fd, 0, SEEK_SET); 
    
    char buffer[100] = { 0 };
    read(fd, buffer, sizeof(buffer) - 1);

    printf("Read from file: %s\n", buffer);
    
    close(fd);

    return 0;
}
