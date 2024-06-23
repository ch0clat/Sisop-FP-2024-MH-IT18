#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    char channel[BUFFER_SIZE];
    char room[BUFFER_SIZE];
} monitor_args_t;

void* monitor_chat(void *args);

int main(int argc, char *argv[]) {
    if (argc < 6) {
        fprintf(stderr, "Usage: %s <USERNAME> -channel <CHANNEL_NAME> -room <ROOM_NAME>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

char *username = argv[1];
    char *channel = argv[3];
    char *room = argv[5];

    int server_socket;
    struct sockaddr_in server_addr;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Define server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to the server
    if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Log in to the server
    char login_command[BUFFER_SIZE];
    snprintf(login_command, sizeof(login_command), "LOGIN %s", username);
    send(server_socket, login_command, strlen(login_command), 0);
char response[BUFFER_SIZE];
    int bytes_received = recv(server_socket, response, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        response[bytes_received] = '\0';
        printf("%s\n", response);
    }

    // Start monitoring the chat
    monitor_args_t monitor_args;
    monitor_args.socket = server_socket;
    strcpy(monitor_args.channel, channel);
    strcpy(monitor_args.room, room);

    pthread_t monitor_thread;
    pthread_create(&monitor_thread, NULL, monitor_chat, &monitor_args);
    pthread_join(monitor_thread, NULL);

    close(server_socket);
    return 0;
}

void* monitor_chat(void *args) {
    monitor_args_t *monitor_args = (monitor_args_t *)args;
    int server_socket = monitor_args->socket;
    char *channel = monitor_args->channel;
    char *room = monitor_args->room;

    char command[BUFFER_SIZE];
    snprintf(command, sizeof(command), "SEE CHAT %s %s", channel, room);
send(server_socket, command, strlen(command), 0);

    char buffer[BUFFER_SIZE];
    while (1) {
        int bytes_received = recv(server_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("%s\n", buffer);
        } else {
            break;
        }
    }

    return NULL;
}
