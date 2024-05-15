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

pthread_mutex_t books_mutex = PTHREAD_MUTEX_INITIALIZER;

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
    int num_books = sizeof(books) / sizeof(books[0]) - 1; // Adjust for NULL termination
    char buffer[BUF_SIZE];
    for (int i = 0; i < num_books; i++) {
        snprintf(buffer, BUF_SIZE, "%d: %s\n", i + 1, books[i]);
        send(client_socket, buffer, strlen(buffer), 0);
        usleep(1000); // Small delay to ensure data is sent properly
    }
}

// Function to delete a book
void delete_book(int client_socket) {
    int num_books = sizeof(books) / sizeof(books[0]) - 1; // Adjust for NULL termination
    char buffer[BUF_SIZE];

    // Display books with indices
    display_books(client_socket);

    // Request book number to delete
    send(client_socket, "Enter the number of the book to delete: ", strlen("Enter the number of the book to delete: "), 0);
    int bytes_read = read(client_socket, buffer, BUF_SIZE - 1);
    buffer[bytes_read] = '\0'; // Ensure null-terminated string
    printf("Received book number to delete: %s\n", buffer); // Debug print

    int book_index = atoi(buffer) - 1;

    // Validate book index
    if (book_index >= 0 && book_index < num_books) {
        pthread_mutex_lock(&books_mutex);
        for (int i = book_index; i < num_books - 1; i++) {
            books[i] = books[i + 1];
        }
        books[num_books - 1] = NULL; // Nullify the last book entry
        pthread_mutex_unlock(&books_mutex);

        send(client_socket, "Book deleted successfully.\n", strlen("Book deleted successfully.\n"), 0);
        printf("Book deleted successfully.\n"); // Debug print
    } else {
        send(client_socket, "Invalid book number.\n", strlen("Invalid book number.\n"), 0);
        printf("Invalid book number.\n"); // Debug print
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
    printf("Received role: %d\n", expected_admin); // Debug print

    // Request username
    send(client_socket, "Username: ", strlen("Username: "), 0);
    bytes_read = read(client_socket, buffer, BUF_SIZE - 1);
    buffer[bytes_read] = '\0'; // Ensure null-terminated string
    char username[BUF_SIZE];
    strncpy(username, buffer, BUF_SIZE);
    printf("Received username: %s\n", username); // Debug print

    // Request password
    send(client_socket, "Password: ", strlen("Password: "), 0);
    bytes_read = read(client_socket, buffer, BUF_SIZE - 1);
    buffer[bytes_read] = '\0'; // Ensure null-terminated string
    char password[BUF_SIZE];
    strncpy(password, buffer, BUF_SIZE);
    printf("Received password: %s\n", password); // Debug print

    // Authenticate user
    if (!authenticate(username, password, &is_admin) || is_admin != expected_admin) {
        send(client_socket, "Invalid username, password, or role\n", strlen("Invalid username, password, or role\n"), 0);
        close(client_socket);
        return NULL;
    }

    if (is_admin) {
        send(client_socket, "Authentication successful (admin)\n", strlen("Authentication successful (admin)\n"), 0);
        send(client_socket, "You have admin privileges.\n", strlen("You have admin privileges.\n"), 0);
        printf("Admin authenticated: %s\n", username); // Debug print

        // Admin options
        send(client_socket, "Select an action:\n1. List books\n2. Delete a book\n", strlen("Select an action:\n1. List books\n2. Delete a book\n"), 0);
        bytes_read = read(client_socket, buffer, BUF_SIZE - 1);
        buffer[bytes_read] = '\0'; // Ensure null-terminated string
        int action = atoi(buffer);
        printf("Admin selected action: %d\n", action); // Debug print

        if (action == 1) {
            send(client_socket, "Library Books:\n", strlen("Library Books:\n"), 0);
            display_books(client_socket);
        } else if (action == 2) {
            delete_book(client_socket);
        } else {
            send(client_socket, "Invalid action.\n", strlen("Invalid action.\n"), 0);
            printf("Invalid admin action: %d\n", action); // Debug print
        }
    } else {
        send(client_socket, "Authentication successful (user)\n", strlen("Authentication successful (user)\n"), 0);
        send(client_socket, "You have user privileges.\n", strlen("You have user privileges.\n"), 0);
        printf("User authenticated: %s\n", username); // Debug print

        // Display books
        send(client_socket, "Library Books:\n", strlen("Library Books:\n"), 0);
        display_books(client_socket);
    }

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
