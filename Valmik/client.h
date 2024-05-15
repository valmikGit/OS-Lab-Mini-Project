#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Define constants for socket communication
#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define MAX_USERNAME_LENGTH 100
#define MAX_PASSWORD_LENGTH 100
#define MAX_BOOK_TITLE_LENGTH 100
#define MAX_BOOK_AUTHOR_LENGTH 100
#define MAX_ATTEMPTS 10
#define TIMEOUT_SECONDS 5   // 5 seconds

// Function prototypes
void issue_book(int client_socket);
void return_book(int client_socket);