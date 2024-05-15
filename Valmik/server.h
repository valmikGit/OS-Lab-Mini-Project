#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h> // For non-blocking I/O
#include <errno.h>

// Define constants for socket communication
#define PORT 8080
#define MAX_CLIENTS 10
#define MAX_USERNAME_LENGTH 100
#define MAX_PASSWORD_LENGTH 100
#define FILENAME "books.dat"
#define MAX_MEMBERS 100
#define MEMBER_FILE "members.dat"
#define MAX_BOOK_TITLE_LENGTH 100
#define MAX_BOOK_AUTHOR_LENGTH 100
// Function prototypes
void *handle_client(void *arg);
int authenticate_user(int client_socket);
void manage_books(int client_socket);
void add_book(int client_socket);
void delete_book(int client_socket);
void modify_book(int client_socket);
void search_book(int client_socket);
void manage_members(int client_socket);
void add_member(int client_socket);
void delete_member(int client_socket);
void modify_member(int client_socket);
void search_member(int client_socket);
int insert_book_to_database(int book_id, char *title, char *author);
int delete_book_from_database(int book_id);
int generate_unique_book_id();
void issue_book(int client_socket);
void return_book(int client_socket);