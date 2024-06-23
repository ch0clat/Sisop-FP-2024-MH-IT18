#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <crypt.h>
#include <time.h> 
#include <openssl/sha.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

#define PORT 8080
#define BUFFER_SIZE 1024

#define MAX_FILENAME_LEN 100
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define SALT_LEN 16

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
void create_channel(char *channel, char *key, int client_socket, char *user);
int find_user (char *username);
void edit_channel(char *old_name, char *new_name, int client_socket);
void delete_channel(char *channel_name, int client_socket);

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
    printf("Listening\n");
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
            } else if (strcmp(entity, "USER") == 0) {
                list_users(client->socket);
            }
        } else if (strcmp(command, "CREATE") == 0) {
            char *entity = strtok(NULL, " ");
            if (strcmp(entity, "CHANNEL") == 0) {
                char *channel = strtok(NULL, " ");
                strtok(NULL, " "); // Skip the "-k"
                char *key = strtok(NULL, " ");
                char *user = strtok(NULL, " ");
                printf("%s", user);
                create_channel(channel, key, client->socket, user);
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
            } else if (strcmp(entity, "CHANNEL") == 0) {
                char *old_name = strtok(NULL, " ");
                strtok(NULL, " "); // Skip "TO"
                char *new_name = strtok(NULL, " ");
                edit_channel(old_name, new_name, client->socket);
            }
        } else if (strcmp(command, "DEL") == 0) {
            char *entity = strtok(NULL, " ");
            if (strcmp(entity, "CHAT") == 0) {
                char *channel = strtok(NULL, " ");
                char *room = strtok(NULL, " ");
                int id = atoi(strtok(NULL, " "));
                delete_chat(command, channel, room, id, client->socket);
            } else if (strcmp(entity, "CHANNEL") == 0) {
                char *channel = strtok(NULL, " ");
                delete_channel(channel, client->socket);
            }
        } else if (strcmp(command, "EXIT") == 0) {
            break;
        } else {
            error_response("Unknown command\n", client->socket);
        }
    }

    close(client->socket);
    free(client);
    return NULL;
}


int extract_id(const char *line) {
    int user_id;
    sscanf(line, "%d,", &user_id);
    return user_id;
}

void register_user(char *username, char *password, int client_socket) {
    int user_id = 1;
    int id;
    int found = 1;
    FILE *fp;
    char filename[MAX_FILENAME_LEN] = "users.csv";
    char line[256];
    char temp_user[MAX_USERNAME_LEN];
    char response[1024];

    
    // Open CSV file for appending
    fp = fopen(filename, "a+");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }


    while (fscanf(fp, "%d,%[^,],%*s", &id, temp_user) == 2) {
        printf("%s", temp_user);
        if (strcmp(temp_user, username) == 0) {
            sprintf(response, "%s sudah terdaftar\n", username);
            write(client_socket, response, strlen(response));
            fclose(fp);
            found = 0;
        }
        if (found == 0){
            return;
        }
    }

    fseek(fp, 0, SEEK_SET);

    while (fgets(line, sizeof(line), fp)) {
        user_id = extract_id(line) + 1;
    }

    // Write new user to the CSV file
    if (user_id == 1){
        fprintf(fp, "%d,%s,%s,Root\n", user_id, username, password);
    } else {
        fprintf(fp, "%d,%s,%s,User\n", user_id, username, password);
    }

    // Close the file
    fclose(fp);

    sprintf(response, "%s berhasil register\n", username);
    write(client_socket, response, strlen(response));
}

void login_user(char *username, char *password, int client_socket) {
    FILE *fp;
    char filename[MAX_FILENAME_LEN] = "users.csv";
    int found = 0;
    int user_id;
    char response[1024];
    char temp_user[MAX_USERNAME_LEN];
    char temp_role[255];
    char temp_password[MAX_PASSWORD_LEN];

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        error_response("Internal server error", client_socket);
        return;
    }

    // Search for the user in the file
    while (fscanf(fp, "%d,%[^,],%[^,],%[^\n]", &user_id, temp_user, temp_password, temp_role) == 4) {
        if (strcmp(temp_user, username) == 0 && strcmp(temp_password, password) == 0) {
            sprintf(response, "%s berhasil login\n", username);
            write(client_socket, response, strlen(response));
            return;
        }
    }


    sprintf(response, "Login gagal");
    write(client_socket, response, strlen(response));
    // Close the file
    fclose(fp);
}

int find_user (char *username){
    int id;
    int temp_id;
    char *temp_user;
    FILE *fp;
    char filename[MAX_FILENAME_LEN] = "users.csv";

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return 0;
    }

    while (fscanf(fp, "%d,%[^,],%*s", &temp_id, temp_user) == 2) {
        if (strcmp(temp_user, username) == 0) {
            id = temp_id;
            fclose(fp);
            return id;
        }
    }

    fclose(fp);
    return 0;
}

void channel_auth() {

}

void create_channel(char *channel, char *key, int client_socket, char *user) {
    FILE *fp;
    
    char filename[MAX_FILENAME_LEN] = "channels.csv";
    char line[256];
    char temp_channel[MAX_FILENAME_LEN];
    char response[BUFFER_SIZE];
    char channel_path[MAX_FILENAME_LEN];
    int channel_id;

    // Open CSV file for appending
    fp = fopen(filename, "a+");
    if (fp == NULL) {
        perror("Error opening file");
        error_response("Internal server error", client_socket);
        return;
    }

    // Check if the channel already exists in channels.csv
    while (fgets(line, sizeof(line), fp) != NULL) {
        sscanf(line, "%[^,],%*s", temp_channel);
        if (strcmp(temp_channel, channel) == 0) {
            sprintf(response, "Channel %s sudah ada\n", channel);
            write(client_socket, response, strlen(response));
            fclose(fp);
            return;
        }
    }

    // Close the file and reopen for writing
    fclose(fp);

    // Create directory for the new channel
    snprintf(channel_path, sizeof(channel_path), "%s", channel);
    if (mkdir(channel_path, 0777) == -1) {
        perror("mkdir");
        error_response("Failed to create channel", client_socket);
        return;
    }

    // Create admin directory and auth.csv for the channel
    char admin_path[MAX_FILENAME_LEN];
    snprintf(admin_path, sizeof(admin_path), "%s/admin", channel_path);
    if (mkdir(admin_path, 0777) == -1) {
        perror("mkdir");
        error_response("Failed to create admin directory", client_socket);
        return;
    }

    char auth_path[MAX_FILENAME_LEN];
    snprintf(auth_path, sizeof(auth_path), "%s/auth.csv", admin_path);
    fp = fopen(auth_path, "w");
    if (fp == NULL) {
        perror("fopen");
        error_response("Failed to create auth.csv", client_socket);
        return;
    }

    // Find user id
    int id = find_user(user);

    // Write user information to auth.csv
    fprintf(fp, "%d,%s,ADMIN\n", id, user);
    fclose(fp);

    // Append new channel to channels.csv
    fp = fopen(filename, "a+");
    if (fp == NULL) {
        perror("fopen");
        error_response("Failed to append to channels.csv", client_socket);
        return;
    }

    while (fgets(line, sizeof(line), fp)) {
        channel_id = extract_id(line) + 1;
    }

    fprintf(fp, "%d,%s,%s\n", channel_id,channel, key);
    fclose(fp);

    sprintf(response, "Channel %s dibuat\n", channel);
    write(client_socket, response, strlen(response));
    }

// int remove_directory(const char *path) {
//     DIR *d = opendir(path);
//     size_t path_len = strlen(path);
//     int r = -1;

//     if (d) {
//         struct dirent *p;

//         r = 0;

//         while (!r && (p = readdir(d))) {
//             int r2 = -1;
//             char *buf;
//             size_t len;

//             // Skip "." and ".." directories
//             if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
//                 continue;
//             }

//             len = path_len + strlen(p->d_name) + 2; 
//             buf = malloc(len);

//             if (buf) {
//                 struct stat statbuf;

//                 snprintf(buf, len, "%s/%s", path, p->d_name);

//                 if (!stat(buf, &statbuf)) {
//                     if (S_ISDIR(statbuf.st_mode)) {
//                         r2 = remove_directory(buf);
//                     } else {
//                         r2 = unlink(buf);
//                     }
//                 }

//                 free(buf);
//             }

//             r = r2;
//         }

//         closedir(d);
//     }

//     if (!r) {
//         r = rmdir(path);
//     }

//     return r;
// }

void edit_channel(char *old_name, char *new_name, int client_socket) {
    char old_path[MAX_FILENAME_LEN];
    char new_path[MAX_FILENAME_LEN];
    char response[1024];

    snprintf(old_path, sizeof(old_path), "%s", old_name);
    snprintf(new_path, sizeof(new_path), "%s", new_name);

    struct stat st = {0};
    if (stat(old_path, &st) == 0) {
        if (stat(new_path, &st) == -1) {
            if (rename(old_path, new_path) == -1) {
                perror("rename");
                error_response("Failed to rename channel", client_socket);
                return;
            }
            snprintf(response, sizeof(response), "Channel %s berhasil diubah menjadi %s\n", old_name, new_name);
        } else {
            snprintf(response, sizeof(response), "Channel %s sudah ada\n", new_name);
        }
    } else {
        snprintf(response, sizeof(response), "Channel %s tidak ada\n", old_name);
    }
    write(client_socket, response, strlen(response));

}

void delete_channel(char *channel_name, int client_socket) {
    char channel_path[MAX_FILENAME_LEN];
    char response[1024];

    snprintf(channel_path, sizeof(channel_path), "%s",  channel_name);

    struct stat st = {0};
    if (stat(channel_path, &st) == 0) {
        char command[MAX_FILENAME_LEN];
        snprintf(command, sizeof(command), "rm -r %s", channel_path);
        system(command);

        snprintf(response, sizeof(response), "Channel %s berhasil dihapus\n", channel_name);
    } else {
        snprintf(response, sizeof(response), "Channel %s tidak ada\n", channel_name);
    }
    write(client_socket, response, strlen(response));
}

void list_users(int client_socket) {
    FILE *fp;
    char filename[MAX_FILENAME_LEN] = "users.csv";
    char line[BUFFER_SIZE];
    char response[BUFFER_SIZE] = "Users: ";

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        error_response("Internal server error\n", client_socket);
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *id = strtok(line, ",");
        char *username = strtok(NULL, ",");
        strcat(response, username);
        strcat(response, " ");
    }
    fclose(fp);

    strcat(response, "\n");
    write(client_socket, response, strlen(response));
}

void list_channels(int client_socket) {
    FILE *fp;
    char filename[MAX_FILENAME_LEN] = "channels.csv";
    char line[BUFFER_SIZE];
    char response[BUFFER_SIZE] = "Channels: ";

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        error_response("Internal server error\n", client_socket);
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *channel = strtok(line, ",");
        strcat(response, channel);
        strcat(response, " ");
    }
    fclose(fp);

    strcat(response, "\n");
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
