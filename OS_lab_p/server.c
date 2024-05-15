#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "books.h"
#include "users.h"

#define PORT 8080
#define BUF_SIZE 1024

// Function to authenticate user
int authenticate(const char *username, const char *password, int *is_admin) {
    int num_users = sizeof(users) / sizeof(users[0]);
    for (int i = 0; i < num_users; i++) {
        if (strcmp(username, users[i].username) == 0 && strcmp(password, users[i].password) == 0) {
            *is_admin = users[i].is_admin;
            return 1; // Authentication successful
        }
    }
    return 0; // Authentication failed
}

// Function to display books
void display_books(int client_socket) {
    int num_books = sizeof(books) / sizeof(books[0]);
    char buffer[BUF_SIZE];
    for (int i = 0; i < num_books; i++) {
        snprintf(buffer, BUF_SIZE, "%s\n", books[i]);
        send(client_socket, buffer, strlen(buffer), 0);
        usleep(1000); // Small delay to ensure data is sent properly
    }
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);
    char buffer[BUF_SIZE];
    int bytes_read;
    int is_admin = 0;
    int expected_admin;

    // Request role
    send(client_socket, "Select role (1 for admin, 0 for user): ", strlen("Select role (1 for admin, 0 for user): "), 0);
    bytes_read = read(client_socket, buffer, BUF_SIZE - 1);
    buffer[bytes_read] = '\0'; // Ensure null-terminated string
    expected_admin = atoi(buffer);

    // Request username
    send(client_socket, "Username: ", strlen("Username: "), 0);
    bytes_read = read(client_socket, buffer, BUF_SIZE - 1);
    buffer[bytes_read] = '\0'; // Ensure null-terminated string
    char username[BUF_SIZE];
    strncpy(username, buffer, BUF_SIZE);

    // Request password
    send(client_socket, "Password: ", strlen("Password: "), 0);
    bytes_read = read(client_socket, buffer, BUF_SIZE - 1);
    buffer[bytes_read] = '\0'; // Ensure null-terminated string
    char password[BUF_SIZE];
    strncpy(password, buffer, BUF_SIZE);

    // Authenticate user
    if (!authenticate(username, password, &is_admin) || is_admin != expected_admin) {
        send(client_socket, "Invalid username, password, or role\n", strlen("Invalid username, password, or role\n"), 0);
        close(client_socket);
        return NULL;
    }

    if (is_admin) {
        send(client_socket, "Authentication successful (admin)\n", strlen("Authentication successful (admin)\n"), 0);
        send(client_socket, "You have admin privileges.\n", strlen("You have admin privileges.\n"), 0);
    } else {
        send(client_socket, "Authentication successful (user)\n", strlen("Authentication successful (user)\n"), 0);
        send(client_socket, "You have user privileges.\n", strlen("You have user privileges.\n"), 0);
    }

    // Display books
    send(client_socket, "Library Books:\n", strlen("Library Books:\n"), 0);
    display_books(client_socket);

    close(client_socket);
    return NULL;
}

int main() {
    int server_fd, *client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        client_socket = malloc(sizeof(int));
        if ((*client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            free(client_socket);
            continue;
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)client_socket) != 0) {
            perror("Failed to create thread");
            close(*client_socket);
            free(client_socket);
        }
        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}
