

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

/* Socket API headers */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void do_job(int fd)
{
    int length, rcnt;
    char recvbuf[512], bmsg[512];
    int recvbuflen = 512;

    // Receive until the peer shuts down the connection
    do
    {
        rcnt = recv(fd, recvbuf, recvbuflen, 0);
        if (rcnt > 0)
        {
            printf("Bytes received: %d\n", rcnt);

            // Echo the buffer back to the sender
            rcnt = send(fd, recvbuf, rcnt, 0);
            if (rcnt < 0)
            {
                printf("Send failed:\n");
                close(fd);
                break;
            }
            printf("Bytes sent: %d\n", rcnt);
        }
        else if (rcnt == 0)
            printf("Connection closing...\n");
        else
        {
            printf("Receive failed:\n");
            close(fd);
            break;
        }
    } while (rcnt > 0);
}

int main(int argc, char *argv[])
{
    unsigned short cmdOffset = 1;
    char *homeDirectory = "./";
    unsigned int port;
    char *passwordFile;

    int server, client;
    struct sockaddr_in local_addr;
    struct sockaddr_in remote_addr;
    int length, fd, rcnt, optval;
    pid_t pid;
    if (argc < 7)
    {
        printf("Usage: ./ftpserver -d <directory> -p <port> -u <password file>");
    }
    else
    {
        port = atoi(argv[4]);
        homeDirectory = argv[2];
        passwordFile = argv[6];
        int sockets;

        if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("Can't create socket!");
            return (1);
        }

        memset(&local_addr, 0, sizeof(local_addr));
        memset(&remote_addr, 0, sizeof(remote_addr));

        /* Set values to local_addr structure */
        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        local_addr.sin_port = htons(port);

        // set SO_REUSEADDR on a socket to true (1):
        optval = 1;
        // printf("%s\n",local_addr.sin_addr.s_addr);

        setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

        if (bind(server, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0)
        {
            /* could not start server */
            perror("Bind error");
            return (1);
        }

        if (listen(server, SOMAXCONN) < 0)
        {
            perror("listen");
            exit(1);
        }

        printf("Concurrent  socket server now starting on port %d\n", port);
        printf("Wait for connection\n");
        while (1)
        {
            length = sizeof remote_addr;
            if ((fd = accept(server, (struct sockaddr *)&remote_addr, &length)) == -1)
            {
                perror("Accept Problem!");
                continue;
            }
            printf("Server: got connection from %s\n",
                   inet_ntoa(remote_addr.sin_addr));
            if ((pid = fork()) == 0)
            {
                close(server);
                do_job(fd);
                printf("Child finished their job!\n");
                close(fd);
                exit(0);
            }
        }

        close(server);
    }
}
