#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <crypt.h>
#include <time.h>
#include "utils.h"  

#define PORT 8080
#define BUFFER_SIZE 1024


typedef struct {
    int socket;
    struct sockaddr_in address;
} client_t;

void handle_client(client_t *client);
void* client_handler(void *arg);
void register_user(char *username, char *password, int client_socket);
void login_user(char *username, char *password, int client_socket);
void list_channels(int client_socket);
void join_channel(char *username, char *channel, char *key, int client_socket);
void join_room(char *username, char *channel, char *room, int client_socket);
void chat_message(char *username, char *channel, char *room, char *message, int client_socket);
void see_chat(char *username, char *channel, char *room, int client_socket);
void edit_chat(char *username, char *channel, char *room, int id, char *new_message, int client_socket);
void delete_chat(char *username, char *channel, char *room, int id, int client_socket);
void error_response(char *message, int client_socket);
void sigint_handler(int sig);

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    pthread_t tid;

    signal(SIGINT, sigint_handler);

   
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

  
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

   
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

   
    if (listen(server_socket, 10) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
      
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        client_t *client = malloc(sizeof(client_t));
        client->socket = client_socket;
        client->address = client_addr;

        
        if (pthread_create(&tid, NULL, client_handler, (void *)client) != 0) {
            perror("Thread creation failed");
            close(client_socket);
            free(client);
        }
    }

    close(server_socket);
    return 0;
}

void* client_handler(void *arg) {
    client_t *client = (client_t *)arg;
    char buffer[BUFFER_SIZE];
    int bytes_read;

    while ((bytes_read = read(client->socket, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';

       
        char *command = strtok(buffer, " ");
        if (strcmp(command, "REGISTER") == 0) {
            char *username = strtok(NULL, " ");
            char *password = strtok(NULL, " ");
            register_user(username, password, client->socket);
        } else if (strcmp(command, "LOGIN") == 0) {
            char *username = strtok(NULL, " ");
            char *password = strtok(NULL, " ");
            login_user(username, password, client->socket);
        } else if (strcmp(command, "LIST") == 0) {
            char *entity = strtok(NULL, " ");
            if (strcmp(entity, "CHANNEL") == 0) {
                list_channels(client->socket);
            }
        } else if (strcmp(command, "JOIN") == 0) {
            char *channel = strtok(NULL, " ");
            char *key = strtok(NULL, " ");
            join_channel(command, channel, key, client->socket);
        } else if (strcmp(command, "CHAT") == 0) {
            char *channel = strtok(NULL, " ");
            char *room = strtok(NULL, " ");
            char *message = strtok(NULL, "\n");
            chat_message(command, channel, room, message, client->socket);
        } else if (strcmp(command, "SEE") == 0) {
            char *entity = strtok(NULL, " ");
            if (strcmp(entity, "CHAT") == 0) {
                char *channel = strtok(NULL, " ");
                char *room = strtok(NULL, " ");
                see_chat(command, channel, room, client->socket);
            }
        } else if (strcmp(command, "EDIT") == 0) {
            char *entity = strtok(NULL, " ");
            if (strcmp(entity, "CHAT") == 0) {
                char *channel = strtok(NULL, " ");
                char *room = strtok(NULL, " ");
                int id = atoi(strtok(NULL, " "));
                char *new_message = strtok(NULL, "\n");
                edit_chat(command, channel, room, id, new_message, client->socket);
            }
        } else if (strcmp(command, "DEL") == 0) {
            char *entity = strtok(NULL, " ");
            if (strcmp(entity, "CHAT") == 0) {
                char *channel = strtok(NULL, " ");
                char *room = strtok(NULL, " ");
                int id = atoi(strtok(NULL, " "));
                delete_chat(command, channel, room, id, client->socket);
            }
        } else if (strcmp(command, "EXIT") == 0) {
            break;
        } else {
            error_response("Unknown command", client->socket);
        }
    }

    close(client->socket);
    free(client);
    return NULL;
}

void register_user(char *username, char *password, int client_socket) {
   
    char *response = "User registered successfully\n";
    write(client_socket, response, strlen(response));
}

void login_user(char *username, char *password, int client_socket) {
    
    char *response = "User logged in successfully\n";
    write(client_socket, response, strlen(response));
}

void list_channels(int client_socket) {
    
    char *response = "Listing channels...\n";
    write(client_socket, response, strlen(response));
}

void join_channel(char *username, char *channel, char *key, int client_socket) {
    
    char *response = "Channel joined successfully\n";
    write(client_socket, response, strlen(response));
}

void join_room(char *username, char *channel, char *room, int client_socket) {
    
    char *response = "Room joined successfully\n";
    write(client_socket, response, strlen(response));
}

void chat_message(char *username, char *channel, char *room, char *message, int client_socket) {
  
    char *response = "Message sent successfully\n";
    write(client_socket, response, strlen(response));
}

void see_chat(char *username, char *channel, char *room, int client_socket) {
    
    char *response = "Displaying chat messages...\n";
    write(client_socket, response, strlen(response));
}

void edit_chat(char *username, char *channel, char *room, int id, char *new_message, int client_socket) {
   
    char *response = "Message edited successfully\n";
    write(client_socket, response, strlen(response));
}

void delete_chat(char *username, char *channel, char *room, int id, int client_socket) {

    char *response = "Message deleted successfully\n";
    write(client_socket, response, strlen(response));
}

void error_response(char *message, int client_socket) {
    
    write(client_socket, message, strlen(message));
}

void sigint_handler(int sig) {
    
    printf("Shutting down server...\n");
    exit(0);
}
