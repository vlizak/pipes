#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

#define BUFSIZE 512
#define MAX_EVENTS 2

int main(int argc, char *argv[])
{
    int pipefd[2];
    int nfds, epollfd = epoll_create(3);

    if (epollfd < 0)
    {
        printf("An error occured while creating epoll handler, errno = %d\n", errno);
    }
    char buf;
    char toPrint[] = "ping ";
    struct epoll_event ev, events[MAX_EVENTS]; 

//    char buffer[BUFSIZE];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
       
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = pipefd[0];

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, pipefd[0], &ev) < 0)
    {
        printf("An error occured while connecting read end of the pipe.\n");
    }
//    fcntl(pipefd[1], F_SETFL, O_NONBLOCK);
    fcntl(pipefd[0], F_SETFL, O_NONBLOCK);

    ev.events = EPOLLOUT | EPOLLET;
    ev.data.fd = pipefd[1];

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, pipefd[1], &ev) < 0)
    {
        printf("An error occured while connecting write end of the pipe.\n");
    }

    printf("fd of epollfd = %d\n", epollfd);
    printf("fd of read end of the pipe = %d\n", pipefd[0]);
    printf("fd of write end of the pipe = %d\n", pipefd[1]);

    bool readEndClosed = false, writeEndClosed = false;

    while (!readEndClosed && !writeEndClosed)
    {
        printf("In while\n");
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);    
        printf("nfds = %d\n", nfds);
        int messageSize;
        
        for (int i = 0; i < nfds; ++i)        
        {
            if (events[i].data.fd == pipefd[0])
            {
                printf("Selected fd = %d\n", events[i].data.fd);
                while ( (messageSize = read(pipefd[0], &buf, 1)) > 0)
                {
                    write(STDOUT_FILENO, &buf, 1);
                    printf("Byte written, messageSize = %d\n", messageSize);
                } 
                printf("After readend while\n");
                if (0 == messageSize)
                {
                    printf("No more messages to read.\n");
                    close(pipefd[0]);
                    readEndClosed = true;
                }
            }
            else if (events[i].data.fd == pipefd[1])
            {
                printf("Selected fd = %d\n", events[i].data.fd);
                write(pipefd[1], toPrint, sizeof(toPrint));
                printf("written\n");
                close(pipefd[1]);
                writeEndClosed = true;
            }
        }
        sleep(1);
    }
    return 0;
}
