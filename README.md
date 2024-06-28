# Sisop-FP-2024-MH-IT18

## Anggota kelompok
   ### Callista Meyra Azizah 5027231060
   ### Abhirama Triadyatma Hermawan 5027231061
   ### Adi Satria Pangestu 5027231043

## Penjelasan

### Server.c

#### Header dan Makro
```c
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
```
Bagian ini mengimpor berbagai header yang diperlukan dan mendefinisikan beberapa makro untuk ukuran buffer dan panjang maksimum untuk berbagai elemen seperti nama file, nama pengguna, dan kata sandi.

#### Struktur Client
```c
typedef struct {
    int socket;
    struct sockaddr_in address;
} client_t;
```
`client_t` adalah struktur yang menyimpan informasi mengenai client, termasuk socket dan alamat mereka.

#### Deklarasi Fungsi
```c
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
```
Deklarasi berbagai fungsi yang akan digunakan dalam server, seperti `register_user`, `login_user`, `list_channels`, dan lain-lain.

#### Fungsi Utama
```c
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
```
Fungsi `main` adalah inti dari server. Ini mencakup pembuatan socket, pengaturan opsi socket, binding, listening, dan loop untuk menerima koneksi klien. Setiap koneksi klien yang diterima akan ditangani oleh thread baru yang menjalankan fungsi `client_handler`.

#### Handler untuk Client
```c
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
```
Berikut ini adalah penjelasan dari masing-masing fungsi dalam bagian kode yang diberikan:

#### Fungsi `extract_id`
```c
int extract_id(const char *line) {
    int user_id;
    sscanf(line, "%d,", &user_id);
    return user_id;
}
```
- **Deskripsi**: Fungsi ini mengambil ID pengguna dari baris teks yang diformat dalam bentuk CSV (Comma Separated Values).
- **Parameter**: `line` adalah baris teks yang mengandung ID.
- **Keluaran**: ID pengguna sebagai bilangan bulat.

#### Fungsi `register_user`
```c
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
        if (strcmp(temp_user, username) == 0) {
            sprintf(response, "%s sudah terdaftar\n", username);
            write(client_socket, response, strlen(response));
            fclose(fp);
            found = 0;
        }
        if (found == 0) {
            return;
        }
    }

    fseek(fp, 0, SEEK_SET);

    while (fgets(line, sizeof(line), fp)) {
        user_id = extract_id(line) + 1;
    }

    // Write new user to the CSV file
    if (user_id == 1) {
        fprintf(fp, "%d,%s,%s,Root\n", user_id, username, password);
    } else {
        fprintf(fp, "%d,%s,%s,User\n", user_id, username, password);
    }

    // Close the file
    fclose(fp);

    sprintf(response, "%s berhasil register\n", username);
    write(client_socket, response, strlen(response));
}
```
- **Deskripsi**: Fungsi ini mendaftarkan pengguna baru dengan menambahkan informasi pengguna ke file CSV.
- **Parameter**: `username`, `password`, dan `client_socket`.
- **Proses**:
  - Membuka file `users.csv`.
  - Mengecek apakah `username` sudah ada dalam file.
  - Jika tidak ada, menambahkan `username` dengan `password` dan ID baru.
- **Keluaran**: Mengirimkan pesan ke client socket apakah pendaftaran berhasil atau gagal.

#### Fungsi `login_user`
```c
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
```
- **Deskripsi**: Fungsi ini memverifikasi `username` dan `password` pengguna dengan mencocokkannya dengan data dalam file CSV.
- **Parameter**: `username`, `password`, dan `client_socket`.
- **Proses**:
  - Membuka file `users.csv`.
  - Mencari kecocokan antara `username` dan `password`.
- **Keluaran**: Mengirimkan pesan ke client socket apakah login berhasil atau gagal.

#### Fungsi `find_user`
```c
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
```
- **Deskripsi**: Fungsi ini mencari ID pengguna berdasarkan `username` dalam file `users.csv`.
- **Parameter**: `username`.
- **Keluaran**: ID pengguna sebagai bilangan bulat jika ditemukan, atau 0 jika tidak ditemukan.

#### Fungsi `create_channel`
```c
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
```
- **Deskripsi**: Fungsi ini membuat saluran baru, menambahkan entri ke `channels.csv`, dan membuat direktori serta file yang diperlukan untuk saluran tersebut.
- **Parameter**: `channel`, `key`, `client_socket`, dan `user`.
- **Proses**:
  - Memeriksa apakah saluran sudah ada.
  - Membuat direktori untuk saluran.
  - Menambahkan informasi saluran ke `channels.csv`.
  - Membuat direktori admin dan file `auth.csv`.
- **Keluaran**: Mengirimkan pesan ke client socket apakah pembuatan saluran berhasil atau gagal.

#### Fungsi `edit_channel`
```c
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
            snprintf(response,

 sizeof(response), "Channel %s renamed to %s\n", old_name, new_name);
        } else {
            snprintf(response, sizeof(response), "Nama channel %s sudah digunakan\n", new_name);
        }
    } else {
        snprintf(response, sizeof(response), "Channel %s tidak ditemukan\n", old_name);
    }

    write(client_socket, response, strlen(response));
}
```
- **Deskripsi**: Fungsi ini mengganti nama saluran jika saluran lama ada dan nama baru belum digunakan.
- **Parameter**: `old_name`, `new_name`, dan `client_socket`.
- **Proses**:
  - Memeriksa apakah saluran lama ada.
  - Memeriksa apakah nama baru belum digunakan.
  - Mengganti nama saluran.
- **Keluaran**: Mengirimkan pesan ke client socket apakah penggantian nama berhasil atau gagal.

#### Fungsi `delete_channel`
```c
void delete_channel(char *channel_name, int client_socket) {
    char channel_path[MAX_FILENAME_LEN];
    char response[1024];

    snprintf(channel_path, sizeof(channel_path), "%s", channel_name);

    struct stat st = {0};
    if (stat(channel_path, &st) == 0) {
        if (remove(channel_path) == 0) {
            snprintf(response, sizeof(response), "Channel %s berhasil dihapus\n", channel_name);
        } else {
            perror("remove");
            snprintf(response, sizeof(response), "Failed to delete channel %s\n", channel_name);
        }
    } else {
        snprintf(response, sizeof(response), "Channel %s tidak ditemukan\n", channel_name);
    }

    write(client_socket, response, strlen(response));
}
```
- **Deskripsi**: Fungsi ini menghapus saluran jika saluran ada.
- **Parameter**: `channel_name` dan `client_socket`.
- **Proses**:
  - Memeriksa apakah saluran ada.
  - Menghapus saluran.
- **Keluaran**: Mengirimkan pesan ke client socket apakah penghapusan saluran berhasil atau gagal.

#### Fungsi `list_channel`
```c
void list_channel(int client_socket) {
    FILE *fp;
    char filename[MAX_FILENAME_LEN] = "channels.csv";
    char line[256];
    char channel[256];
    char response[BUFFER_SIZE];

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        error_response("Internal server error", client_socket);
        return;
    }

    sprintf(response, "Channel yang ada:\n");
    while (fgets(line, sizeof(line), fp) != NULL) {
        sscanf(line, "%[^,],%*s", channel);
        strcat(response, channel);
        strcat(response, "\n");
    }
    fclose(fp);

    write(client_socket, response, strlen(response));
}
```
- **Deskripsi**: Fungsi ini mencetak daftar saluran yang ada.
- **Parameter**: `client_socket`.
- **Proses**:
  - Membuka file `channels.csv`.
  - Membaca dan mencetak daftar saluran.
- **Keluaran**: Mengirimkan daftar saluran ke client socket.

### Fungsi `join_channel`
```c
void join_channel(char *username, char *channel, int client_socket) {
    FILE *fp;
    char filename[MAX_FILENAME_LEN] = "channels.csv";
    char line[256];
    char response[BUFFER_SIZE];
    char user_path[MAX_FILENAME_LEN];
    char *temp_channel = (char *)malloc(256);

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        error_response("Internal server error", client_socket);
        return;
    }

    // Search for the channel in channels.csv
    while (fgets(line, sizeof(line), fp) != NULL) {
        sscanf(line, "%[^,],%*s", temp_channel);
        if (strcmp(temp_channel, channel) == 0) {
            fclose(fp);

            snprintf(user_path, sizeof(user_path), "%s/auth.csv", channel);
            fp = fopen(user_path, "a");
            if (fp == NULL) {
                perror("Error opening file");
                error_response("Internal server error", client_socket);
                return;
            }

            int id = find_user(username);
            fprintf(fp, "%d,%s,USER\n", id, username);
            fclose(fp);

            sprintf(response, "%s berhasil join ke channel %s\n", username, channel);
            write(client_socket, response, strlen(response));
            return;
        }
    }

    fclose(fp);

    sprintf(response, "Channel %s tidak ditemukan\n", channel);
    write(client_socket, response, strlen(response));
}
```
- **Deskripsi**: Fungsi ini memungkinkan pengguna untuk bergabung ke saluran yang ada.
- **Parameter**: `username`, `channel`, dan `client_socket`.
- **Proses**:
  - Memeriksa apakah saluran ada di `channels.csv`.
  - Jika ada, menambahkan pengguna ke `auth.csv` dalam direktori saluran.
- **Keluaran**: Mengirimkan pesan ke client socket apakah bergabung ke saluran berhasil atau gagal.

#### Fungsi `list_user_in_channel`
```c
void list_user_in_channel(char *channel, int client_socket) {
    FILE *fp;
    char filename[MAX_FILENAME_LEN];
    char line[256];
    char response[BUFFER_SIZE];
    char temp_user[MAX_USERNAME_LEN];
    char temp_role[255];
    int user_id;

    snprintf(filename, sizeof(filename), "%s/auth.csv", channel);

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        error_response("Channel not found", client_socket);
        return;
    }

    sprintf(response, "Pengguna dalam channel %s:\n", channel);
    while (fscanf(fp, "%d,%[^,],%[^\n]", &user_id, temp_user, temp_role) == 3) {
        strcat(response, temp_user);
        strcat(response, "\n");
    }

    fclose(fp);
    write(client_socket, response, strlen(response));
}
```
- **Deskripsi**: Fungsi ini mencetak daftar pengguna dalam saluran tertentu.
- **Parameter**: `channel` dan `client_socket`.
- **Proses**:
  - Membuka file `auth.csv` dalam direktori saluran.
  - Membaca dan mencetak daftar pengguna.
- **Keluaran**: Mengirimkan daftar pengguna dalam saluran ke client socket.

#### Fungsi `leave_channel`
```c
void leave_channel(char *username, char *channel, int client_socket) {
    FILE *fp;
    FILE *temp_fp;
    char filename[MAX_FILENAME_LEN];
    char temp_filename[MAX_FILENAME_LEN];
    char line[256];
    char response[BUFFER_SIZE];
    int found = 0;

    snprintf(filename, sizeof(filename), "%s/auth.csv", channel);
    snprintf(temp_filename, sizeof(temp_filename), "%s/temp_auth.csv", channel);

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        error_response("Internal server error", client_socket);
        return;
    }

    temp_fp = fopen(temp_filename, "w");
    if (temp_fp == NULL) {
        perror("Error opening file");
        error_response("Internal server error", client_socket);
        fclose(fp);
        return;
    }

    // Copy all lines except the one with the given username
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, username) == NULL) {
            fputs(line, temp_fp);
        } else {
            found = 1;
        }
    }

    fclose(fp);
    fclose(temp_fp);

    // Replace the original file with the temporary file
    if (found) {
        remove(filename);
        rename(temp_filename, filename);
        sprintf(response, "%s berhasil keluar dari channel %s\n", username, channel);
    } else {
        remove(temp_filename);
        sprintf(response, "User %s tidak ditemukan dalam channel %s\n", username, channel);
    }

    write(client_socket, response, strlen(response));
}
```
- **Deskripsi**: Fungsi ini memungkinkan pengguna untuk keluar dari saluran tertentu.
- **Parameter**: `username`, `channel`, dan `client_socket`.
- **Proses**:
  - Membuka file `auth.csv` dan file sementara `temp_auth.csv`.
  - Menyalin semua baris kecuali baris dengan `username` ke file sementara.
  - Mengganti file asli dengan file sementara jika pengguna ditemukan.
- **Keluaran**: Mengirimkan pesan ke client socket apakah pengguna berhasil keluar dari saluran atau tidak ditemukan.

#### Fungsi `remove_user`
```c
void remove_user(char *username, char *channel, int client_socket) {
    leave_channel(username, channel, client_socket);
}
```
- **Deskripsi**: Fungsi ini menghapus pengguna dari saluran tertentu dengan memanggil fungsi `leave_channel`.
- **Parameter**: `username`, `channel`, dan `client_socket`.
- **Proses**:
  - Memanggil fungsi `leave_channel`.
- **Keluaran**: Mengirimkan pesan ke client socket apakah pengguna berhasil dihapus dari saluran atau tidak.

Fungsi-fungsi ini bertujuan untuk mengelola pengguna dan saluran dalam sistem berbasis socket, mengelola data pengguna dalam file CSV, serta mengelola direktori dan file yang terkait dengan saluran.

### Discorit

#### Header dan Makro
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
```
Bagian ini mengimpor berbagai header yang diperlukan dan mendefinisikan beberapa makro untuk alamat IP server, port server, dan ukuran buffer.

#### Deklarasi Fungsi
```c
void register_user(int server_socket, char *username, char *password);
void login_user(int server_socket, char *username, char *password);
int send_command(int server_socket, char *command);
int receive_response(int server_socket, char *buffer, size_t buffer_size);
```
Deklarasi berbagai fungsi yang akan digunakan dalam client, seperti `register_user`, `login_user`, `send_command`, dan `receive_response`.

#### Fungsi Register User
```c
void register_user(int server_socket, char *username, char *password) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "REGISTER %s %s", username, password);
    send_command(server_socket, buffer);
    receive_response(server_socket, buffer, sizeof(buffer));
}
```
Fungsi ini mengirimkan perintah `REGISTER` ke server dengan username dan password yang diberikan, kemudian menerima respons dari server.

#### Fungsi Login User
```c
void login_user(int server_socket, char *username, char *password) {
    char buffer[BUFFER_SIZE];
    
    snprintf(buffer, sizeof(buffer), "LOGIN %s %s", username, password);
    send_command(server_socket, buffer);

    if (receive_response(server_socket, buffer, sizeof(buffer))) {
        if (strstr(buffer, "berhasil login") != NULL) {
            printf("%s\n[%s] ", buffer, username);
            while (1) {
                char input_buffer[BUFFER_SIZE];
                if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
                    input_buffer[strcspn(input_buffer, "\n")] = 0; 
                    if (strcmp(input_buffer, "exit") == 0) {
                        break;
                    } else if (strncmp(input_buffer, "CREATE CHANNEL", 14) == 0) {
                        char *channel_name_key = input_buffer + strlen("CREATE CHANNEL ");
                        snprintf(buffer, sizeof(buffer), "CREATE CHANNEL %s %s", channel_name_key, username);
                        send_command(server_socket, buffer);
                    } else if (strncmp(input_buffer, "EDIT CHANNEL", 12) == 0) {
                        char *old_channel = strtok(input_buffer + 13, " ");
                        strtok(NULL, " ");
                        char *new_channel = strtok(NULL, " ");
                        snprintf(buffer, sizeof(buffer), "EDIT CHANNEL %s %s", old_channel, new_channel);
                        send_command(server_socket, buffer); 
                    } else if (strncmp(input_buffer, "DEL CHANNEL", 11) == 0) {
                        char *channel_name = input_buffer + 12;
                        snprintf(buffer, sizeof(buffer), "DEL CHANNEL %s", channel_name);
                        send_command(server_socket, buffer);
                    } else if (strncmp(input_buffer, "LIST CHANNEL", 12) == 0) {
                        send_command(server_socket, input_buffer);
                    }
                    // Clear the buffer before receiving the response
                    memset(buffer, 0, sizeof(buffer));
                    receive_response(server_socket, buffer, sizeof(buffer));
                    printf("[%s] ", username);
                }
            }
        } else {
            exit(EXIT_FAILURE);
        }
    }
}
```


#### Fungsi untuk Mengirim Perintah ke Server
```c
int send_command(int server_socket, char *command) {
    send(server_socket, command, strlen(command), 0);
    return 0; // Return a value to avoid compiler warning
}
```
Fungsi ini mengirimkan perintah ke server melalui socket.

#### Fungsi untuk Menerima Respons dari Server
```c
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
```
Fungsi ini menerima respons dari server dan mencetaknya. Jika server menutup koneksi, fungsi ini mengakhiri program.

#### Fungsi Utama
```c
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
```
Fungsi `main` memeriksa argumen yang diberikan saat menjalankan program. Kemudian membuat socket, menghubungkan ke server, dan memanggil fungsi yang sesuai (`register_user` atau `login_user`) berdasarkan perintah yang diberikan.

Fungsi `client_handler` membaca perintah dari klien dan memanggil fungsi yang sesuai berdasarkan perintah tersebut, seperti `register_user`, `login_user`, `list_channels`, dan lain-lain.

Saya akan lanjutkan penjelasan fungsi-fungsi lainnya pada bagian berikutnya.

## Dokumentasi

A. Autentikasi (Register dan Login)
![image](https://github.com/ch0clat/Sisop-FP-2024-MH-IT18/assets/151893499/e1b7e921-f6a6-48ac-a1de-29678bd6f00a)

B. Penggunaan DiscorIT
1. List Channel dan Room
![image](https://github.com/ch0clat/Sisop-FP-2024-MH-IT18/assets/151893499/d77fe60a-23ea-4ee6-8a89-2bd1cc0a142e)
(masukin gambar list room)
3. Akses Channel dan Room
(masukin gambar)
4. Fitur Chat
(masukin gambar)

C. Root
Akun yang pertama kali mendaftar otomatis mendapatkan peran "root".
Root dapat masuk ke channel manapun tanpa key dan create, update, dan delete pada channel dan room, mirip dengan admin [D].
Root memiliki kemampuan khusus untuk mengelola user, seperti: list, edit, dan Remove.
![image](https://github.com/ch0clat/Sisop-FP-2024-MH-IT18/assets/151893499/6132febb-22e5-49f9-a92c-35ac56f8c262)

D. Admin Channel
Setiap user yang membuat channel otomatis menjadi admin di channel tersebut. Informasi tentang user disimpan dalam file auth.csv.
Admin dapat create, update, dan delete pada channel dan room, serta dapat remove, ban, dan unban user di channel mereka.
![image](https://github.com/ch0clat/Sisop-FP-2024-MH-IT18/assets/151893499/eb5e68d1-d465-4941-89ac-09482a774a31)
1. Channel
Informasi tentang semua channel disimpan dalam file channel.csv. Semua perubahan dan aktivitas user pada channel dicatat dalam file users.log.
CREATE CHANNEL
![image](https://github.com/ch0clat/Sisop-FP-2024-MH-IT18/assets/151893499/884db7d7-eaeb-4cf5-b863-68a31a99af03)
EDIT CHANNEL
(masukin gambar)
DELETE CHANNEL
(masukin gambar)
2. Room
Semua perubahan dan aktivitas user pada room dicatat dalam file users.log.
CREATE ROOM
(masukin gambar)
EDIT ROOM
(masukin gambar)
DELETE ROOM
(masukin gambar)
3. Ban 
Admin dapat melakukan ban pada user yang nakal. Aktivitas ban tercatat pada users.log. Ketika di ban, role "user" berubah menjadi "banned". Data tetap tersimpan dan user tidak dapat masuk ke dalam channel.
(masukin gambar)
4. Unban 
Admin dapat melakukan unban pada user yang sudah berperilaku baik. Aktivitas unban tercatat pada users.log. Ketika di unban, role "banned" berubah kembali menjadi "user" dan dapat masuk ke dalam channel.
(masukin gambar)
5. Remove user
Admin dapat remove user dan tercatat pada users.log.
(masukin gambar)
E. User
User dapat mengubah informasi profil mereka, user yang di ban tidak dapat masuk kedalam channel dan dapat keluar dari room, channel, atau keluar sepenuhnya dari DiscorIT.
1. Edit User Username
(masukin gambar)
2. Edit User Password
(masukin gambar)
3. Banned User
(masukin gambar)
4. Exit
(masukin gambar)

F. Error Handling
Jika ada command yang tidak sesuai penggunaannya. Maka akan mengeluarkan pesan error dan tanpa keluar dari program client.
(masukin gambar)

G. Monitor
User dapat menampilkan isi chat secara real-time menggunakan program monitor. Jika ada perubahan pada isi chat, perubahan tersebut akan langsung ditampilkan di terminal.
Sebelum dapat menggunakan monitor, pengguna harus login terlebih dahulu dengan cara yang mirip seperti login di DiscorIT.
Untuk keluar dari room dan menghentikan program monitor dengan perintah "EXIT".
Monitor dapat digunakan untuk menampilkan semua chat pada room, mulai dari chat pertama hingga chat yang akan datang nantinya.
(masukin gambar list room)


