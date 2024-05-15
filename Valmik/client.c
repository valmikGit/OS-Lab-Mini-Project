
#include "client.h"
int main()
{
    int client_socket;
    struct sockaddr_in server_address;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0)
    {
        perror("Invalid address / Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Authentication
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    char auth_message[100];

    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    // Send username to server
    send(client_socket, username, strlen(username), 0);

    // Send password to server
    send(client_socket, password, strlen(password), 0);
    printf("Sent username and password to server and Waiting for server to respond\n");

    // Set timeout for receiving server response
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SECONDS;
    timeout.tv_usec = 0;
    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        perror("Error setting timeout");
        exit(EXIT_FAILURE);
    }
    // Receive authentication result from server
    // Receive authentication result from server with timeout and retry logic
    int attempts = 0;
    while (attempts < MAX_ATTEMPTS)
    {
        if (recv(client_socket, auth_message, sizeof(auth_message), 0) > 0)
        {
            printf("%s\n", auth_message);
            break; // Exit the loop if response received successfully
        }
        else
        {
            attempts++;
            printf("Attempt %d: No response from server. Retrying...\n", attempts);
            // Resend username and password
            send(client_socket, username, strlen(username), 0);
            send(client_socket, password, strlen(password), 0);
        }
    }
    printf("%s\n", auth_message);

    if (strcmp(auth_message, "Authentication successful") == 0)
    {
        // Display main menu options
        char choice;
        do
        {
            printf("\nMain Menu:\n");
            // printf("1. Manage Books\n");
            // printf("2. Manage Members\n");
            printf("3. Issue Book\n");
            printf("4. Return Book\n");
            printf("5. Exit\n");
            printf("Enter your choice: ");
            scanf(" %c", &choice);

            // Send choice to server
            send(client_socket, &choice, sizeof(char), 0);

            // Handle choice
            switch (choice)
            {
            // case '1':
            //     manage_books(client_socket);
            //     break;
            // case '2':
            //     manage_members(client_socket);
            //     break;
            case '3':
                issue_book(client_socket);
                break;
            case '4':
                return_book(client_socket);
                break;
            case '5':
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
            }
        } while (choice != '5');
    }

    // Close socket
    close(client_socket);

    return 0;
}

// Function to handle issuing a book
void issue_book(int client_socket)
{
    int book_id;
    char title[MAX_BOOK_TITLE_LENGTH];
    char author[MAX_BOOK_AUTHOR_LENGTH];

    // Receive book details from user
    printf("Enter Book ID: ");
    scanf("%d", &book_id);
    printf("Enter Book Title: ");
    scanf("%s", title);
    printf("Enter Book Author: ");
    scanf("%s", author);

    // Send book details to server
    send(client_socket, &book_id, sizeof(int), 0);
    send(client_socket, title, strlen(title), 0);
    send(client_socket, author, strlen(author), 0);

    // Receive result from server
    char result[100];
    recv(client_socket, result, sizeof(result), 0);
    printf("%s\n", result);
}

// Function to handle returning a book
void return_book(int client_socket)
{
    int book_id;

    // Receive book ID from user
    printf("Enter Book ID to return: ");
    scanf("%d", &book_id);

    // Send book ID to server
    send(client_socket, &book_id, sizeof(int), 0);

    // Receive result from server
    char result[100];
    recv(client_socket, result, sizeof(result), 0);
    printf("%s\n", result);
}
