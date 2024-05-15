#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF_SIZE 1024

// Function to trim the newline character from the end of the string
void trim_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

void read_server_response(int sock) {
    char buffer[BUF_SIZE];
    int bytes_read = read(sock, buffer, BUF_SIZE - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0'; // Null-terminate the string
        printf("Received from server: %s\n", buffer); // Debug print
    }
}

void send_input_to_server(int sock) {
    char buffer[BUF_SIZE];
    fgets(buffer, BUF_SIZE, stdin);
    trim_newline(buffer); // Trim newline character
    send(sock, buffer, strlen(buffer), 0);
    printf("Sent to server: %s\n", buffer); // Debug print
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // Select role
    read_server_response(sock); // Prompt: Select role (1 for admin, 0 for user):
    send_input_to_server(sock); // Send role

    // Username
    read_server_response(sock); // Prompt: Username:
    send_input_to_server(sock); // Send username

    // Password
    read_server_response(sock); // Prompt: Password:
    send_input_to_server(sock); // Send password

    // Authentication response
    read_server_response(sock); // Read authentication response

    // Handle admin options or user book list display
    int adminAction = 0;
    while (1) {
        read_server_response(sock); // Read the server's prompt for action or book list
        
        // Check if the server is prompting for an action
        if (strstr(buffer, "Select an action:") != NULL) {
            send_input_to_server(sock); // Send the chosen action (1 for list, 2 for delete)

            read_server_response(sock); // Read server's response to the chosen action
            
            // If the action is to delete a book, read and handle the book number input
            if (strstr(buffer, "Enter the number of the book to delete:") != NULL) {
                send_input_to_server(sock); // Send the book number to delete
                read_server_response(sock); // Read the server's confirmation message
            }
        } else {
            // Display books if not an action prompt
            printf("%s", buffer);
        }
        
        // Check for the end of the response
        if (strstr(buffer, "Book deleted successfully.") != NULL || strstr(buffer, "Invalid book number.") != NULL) {
            break; // Exit the loop after the admin action is completed
        }
    }

    close(sock);
    return 0;
}
