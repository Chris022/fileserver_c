#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#define PORT 8080

void sendHeaderData(int socket, int code)
{
    char header[1024];
    snprintf(header, 1024, "HTTP/2 %i \n\r\n", code);
    send(socket, header, strlen(header), 0);
}

void *handleConnection(void *socket)
{
    int new_socket = *((int *)socket);

    char buffer[4096] = {0};
    char readBuffer[4096] = {0};

    read(new_socket, buffer, sizeof(buffer));

    char *command = strtok(buffer, " ");
    printf("Command is %s\n", command);

    if (strcmp(command, "GET") == 0)
    {
        char *path = strtok(NULL, " ");
        // File Öffnen
        FILE *f = fopen(path + 1, "r");

        if (f == NULL)
        {
           sendHeaderData(new_socket,404);
        }
        else
        {
            // File Zeile für Zeile Lesen und an Client senden
            do
            {
                memchr(readBuffer, 0, sizeof(readBuffer));
                char *ret = fgets(readBuffer, sizeof(readBuffer), f);
                send(new_socket, readBuffer, strlen(readBuffer), 0);
            } while (!feof(f));
        }
    }
    close(new_socket);
    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &address.sin_addr);
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    pthread_t tid[50];
    int i = 0;
    int new_socket;
    while (1)
    {

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                 (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        pthread_create(&tid[i], NULL, &handleConnection, &new_socket);
        pthread_join(tid[i++], NULL);
    }

    return 0;
}