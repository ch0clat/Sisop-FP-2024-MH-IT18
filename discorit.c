#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

void register_user(int server_socket, char *username, char *password);
void login_user(int server_socket, char *username, char *password);
int send_command(int server_socket, char *command);
int receive_response(int server_socket, char *buffer, size_t buffer_size);

void register_user(int server_socket, char *username, char *password) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "REGISTER %s %s", username, password);
    send_command(server_socket, buffer);
    receive_response(server_socket, buffer, sizeof(buffer));
}

void login_user(int server_socket, char *username, char *password) {
    char buffer[BUFFER_SIZE];
    
    snprintf(buffer, sizeof(buffer), "LOGIN %s %s", username, password);
    send_command(server_socket, buffer);

    if (receive_response(server_socket, buffer, sizeof(buffer))) {
        if (strstr(buffer, "berhasil login") != NULL) {
            printf("[%s] ", username);
            while (1) {
                if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
                    buffer[strcspn(buffer, "\n")] = 0; 
                    if (strcmp(buffer, "exit") == 0) {
                        break;
                    } else if (strstr("CREATE", buffer) == 0){
                        if (strstr("CHANNEL", buffer) == 0){
                            char *channel_name_key = buffer + strlen("CREATE CHANNEL ");
                            char key[BUFFER_SIZE];
                            snprintf(buffer, sizeof(buffer), "CREATE CHANNEL %s %s", channel_name_key, username);
                        send_command(server_socket, buffer);    
                        } else if ("ROOM"){

                        send_command(server_socket, buffer);
                        }
                    }
                    // printf("%s", buffer);
                    receive_response(server_socket, buffer, sizeof(buffer));
                    printf("[%s] ", username);
                }
            }
        } else {
            exit(EXIT_FAILURE);
        }
    }
}

int send_command(int server_socket, char *command) {
    send(server_socket, command, strlen(command), 0);
}

int receive_response(int server_socket, char *buffer, size_t buffer_size) {
    int bytes_received = recv(server_socket, buffer, buffer_size - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
        return 1;
    } else {
        printf("Server closed the connection.\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <COMMAND> <USERNAME> -p <PASSWORD>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *command = argv[1];
    char *username = argv[2];
    char *password = argv[4];

    int server_socket;
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (strcmp(command, "REGISTER") == 0) {
        register_user(server_socket, username, password);
    } else if (strcmp(command, "LOGIN") == 0) {
        login_user(server_socket, username, password);
    } else {
        fprintf(stderr, "Invalid command\n");
    }

    close(server_socket);
    return 0;
}
