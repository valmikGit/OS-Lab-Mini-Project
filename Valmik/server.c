#include "server.h"
typedef struct
{
    int member_id;
    char name[100];
    char address[100];
} Member;

// Dummy user database (replace with actual database or authentication mechanism)
typedef struct
{
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
} User;

User users[] = {
    {"valmik", "valmiklibrary"},
    {"f", "f"}
    // {"user2", "password2"},
    // Add more users as needed
};

// Define a structure to represent a book
typedef struct
{
    int book_id;
    char title[MAX_BOOK_TITLE_LENGTH];
    char author[MAX_BOOK_AUTHOR_LENGTH];
    int is_issued; // Flag to indicate if the book is currently issued or not
    int issued_to; // Member ID of the member to whom the book is issued
} Book;

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    pthread_t threads[MAX_CLIENTS];
    int i = 0;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind socket to address and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }

    // Set server socket to non-blocking mode
    if (fcntl(server_socket, F_SETFL, O_NONBLOCK) < 0)
    {
        perror("Error setting socket to non-blocking mode");
        exit(EXIT_FAILURE);
    }

    // Accept incoming connections and handle them in separate threads
    while (1)
    {
        socklen_t client_address_length = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
        if (client_socket < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                // No incoming connections, continue to the next iteration
                continue;
            }
            else
            {
                perror("Error accepting connection");
                exit(EXIT_FAILURE);
            }
        }

        // Print message when a client connects
        printf("Client connected. Socket FD: %d\n", client_socket);

        // Create a new thread to handle the client
        if (pthread_create(&threads[i++], NULL, handle_client, (void *)&client_socket) != 0)
        {
            perror("Error creating thread");
            exit(EXIT_FAILURE);
        }

        // Print message when a thread is created
        printf("Thread created for client.\n");

        // Reset thread index if maximum clients reached
        if (i >= MAX_CLIENTS)
        {
            i = 0;
            // Join finished threads to release resources
            while (i < MAX_CLIENTS)
            {
                pthread_join(threads[i++], NULL);
            }
            i = 0;
        }
    }

    return 0;
}

// Function to handle client requests
void *handle_client(void *arg)
{
    printf("(inside handle client done)---\n");
    int client_socket = *((int *)arg);
    printf("Client socket: %d . If it is -1 then we have a problem.\n", client_socket);
    // Verify if the client socket is valid
    if (client_socket < 0) {
        perror("Invalid client socket descriptor");
        return NULL; // Return or handle the error appropriately
    }


    // Step 1: Authenticate the user
    printf("now doing auth user---\n");
    if (authenticate_user(client_socket))
    {

        printf("Auth user done");
    }
    else
    {
        printf("Auth user failed");
    }
    // Step 2: Present menu options and handle client requests
    char choice;
    do
    {
        // Send menu options to client
        send(client_socket, "\nLibrary Management Menu:\n", strlen("\nLibrary Management Menu:\n"), 0);
        send(client_socket, "1. Manage Books\n", strlen("1. Manage Books\n"), 0);
        send(client_socket, "2. Manage Members\n", strlen("2. Manage Members\n"), 0);
        send(client_socket, "3. Issue Book\n", strlen("3. Issue Book\n"), 0);
        send(client_socket, "4. Return Book\n", strlen("4. Return Book\n"), 0);
        send(client_socket, "5. Exit\n", strlen("5. Exit\n"), 0);
        send(client_socket, "Enter your choice: ", strlen("Enter your choice: "), 0);

        // Receive client choice
        recv(client_socket, &choice, sizeof(choice), 0);

        // Handle client choice
        switch (choice)
        {
        case '1':
            manage_books(client_socket);
            break;
        case '2':
            manage_members(client_socket);
            break;
        case '3':
            issue_book(client_socket);
            break;
        case '4':
            return_book(client_socket);
            break;
        case '5':
            send(client_socket, "Exiting...\n", strlen("Exiting...\n"), 0);
            break;
        default:
            send(client_socket, "Invalid choice. Please try again.\n", strlen("Invalid choice. Please try again.\n"), 0);
            break;
        }
    } while (choice != '5');

    // Close client socket
    close(client_socket);
    pthread_exit(NULL);
}

// Dummy authentication function
int authenticate_user(int client_socket)
{
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    char auth_message[100];

    // Receive username from client

    printf("(inside auth user) Ready to receive client auth data\n");
    int bytes_received = recv(client_socket, username, sizeof(username), 0);
    if (bytes_received < 0)
    {
        perror("Error receiving username from client");
        // Handle error appropriately, e.g., close the connection
        close(client_socket);
        return 0;
    }

    // Receive password from client
    recv(client_socket, password, sizeof(password), 0);
    // while ((getchar()) != '\n');
    printf("Recieved username: %s and pwd %s ", username, password);

    // Dummy authentication logic
    int authenticated = 0;
    for (int i = 0; i < sizeof(users) / sizeof(users[0]); i++)
    {
        if (strcmp(username, users[i].username) == 0 && strcmp(password, users[i].password) == 0)
        {
            authenticated = 1;
            printf(" is matched to %s\n", users[i].username);
            break;
        }
        // else{
        //     printf("Username: %s vs received username:%s\n", users[i].username, username);
        //     printf("Password: %s vs received pass: %s\n", users[i].password, password);
        // }
    }

    // Send authentication result to client
    if (authenticated)
    {
        strcpy(auth_message, "Authentication successful");
    }
    else
    {
        strcpy(auth_message, "Authentication failed");
    }
    send(client_socket, auth_message, strlen(auth_message), 0);
    printf("authenticted: %d\n", authenticated);
    return authenticated;
}

// Function to manage books
void manage_books(int client_socket)
{
    char choice;
    do
    {
        // Send menu options to client
        send(client_socket, "\nManage Books Menu:\n", strlen("\nManage Books Menu:\n"), 0);
        send(client_socket, "1. Add Book\n", strlen("1. Add Book\n"), 0);
        send(client_socket, "2. Delete Book\n", strlen("2. Delete Book\n"), 0);
        send(client_socket, "3. Modify Book\n", strlen("3. Modify Book\n"), 0);
        send(client_socket, "4. Search Book\n", strlen("4. Search Book\n"), 0);
        send(client_socket, "5. Back to Main Menu\n", strlen("5. Back to Main Menu\n"), 0);
        send(client_socket, "Enter your choice: ", strlen("Enter your choice: "), 0);

        // Receive client choice
        recv(client_socket, &choice, sizeof(char), 0);

        // Handle client choice
        switch (choice)
        {
        case '1':
            add_book(client_socket);
            break;
        case '2':
            delete_book(client_socket);
            break;
        case '3':
            modify_book(client_socket);
            break;
        case '4':
            search_book(client_socket);
            break;
        case '5':
            send(client_socket, "Returning to main menu...\n", strlen("Returning to main menu...\n"), 0);
            break;
        default:
            send(client_socket, "Invalid choice. Please try again.", strlen("Invalid choice. Please try again."), 0);
            break;
        }
    } while (choice != '5');
}

// Function to add a book to the database
void add_book(int client_socket)
{
    int book_id;
    char title[100];
    char author[100];

    // Receive book details from client
    recv(client_socket, &book_id, sizeof(int), 0);
    recv(client_socket, title, sizeof(title), 0);
    recv(client_socket, author, sizeof(author), 0);

    // Insert book into database
    int result = insert_book_to_database(book_id, title, author);

    // Send result to client
    if (result == 0)
    {
        send(client_socket, "Book added successfully.", strlen("Book added successfully."), 0);
    }
    else
    {
        send(client_socket, "Failed to add book.", strlen("Failed to add book."), 0);
    }
}

// Function to delete a book from the database
void delete_book(int client_socket)
{
    int book_id;

    // Receive book ID from client
    recv(client_socket, &book_id, sizeof(int), 0);

    // Delete book from database
    int result = delete_book_from_database(book_id);

    // Send result to client
    if (result == 0)
    {
        send(client_socket, "Book deleted successfully.", strlen("Book deleted successfully."), 0);
    }
    else
    {
        send(client_socket, "Failed to delete book.", strlen("Failed to delete book."), 0);
    }
}

// Function to modify a book in the database file
void modify_book(int client_socket)
{
    int book_id;
    char title[100];
    char author[100];
    char buffer[256];

    // Prompt user for book ID
    send(client_socket, "Enter book ID to modify: ", strlen("Enter book ID to modify: "), 0);
    recv(client_socket, &book_id, sizeof(int), 0);

    FILE *file = fopen(FILENAME, "rb+"); // Open file in read/write mode for binary reading and writing
    if (file == NULL)
    {
        printf("Error opening file.\n");
        return; // Return if unable to open file
    }

    int found = 0; // Flag to indicate if the book was found and modified

    // Read book records from the file and search for the specified book ID
    int current_book_id;
    char current_title[100];
    char current_author[100];

    while (fread(&current_book_id, sizeof(int), 1, file) == 1 &&
           fread(current_title, sizeof(char), 100, file) == 100 &&
           fread(current_author, sizeof(char), 100, file) == 100)
    {
        if (current_book_id == book_id)
        {
            found = 1; // Set flag to indicate book found

            // Prompt user for new book details
            send(client_socket, "Enter new book title: ", strlen("Enter new book title: "), 0);
            recv(client_socket, title, sizeof(title), 0);
            title[strcspn(title, "\n")] = 0; // Remove newline character

            send(client_socket, "Enter new book author: ", strlen("Enter new book author: "), 0);
            recv(client_socket, author, sizeof(author), 0);
            author[strcspn(author, "\n")] = 0; // Remove newline character

            // Move file pointer back to the beginning of the current record for modification
            fseek(file, -sizeof(int) - 200, SEEK_CUR); // 4 bytes for int + 100 bytes for each char array

            // Write modified book details to file
            fwrite(&current_book_id, sizeof(int), 1, file);
            fwrite(title, sizeof(char), strlen(title) + 1, file);   // Include null terminator
            fwrite(author, sizeof(char), strlen(author) + 1, file); // Include null terminator

            // Inform user about successful modification
            sprintf(buffer, "Book with ID %d modified successfully.\n", book_id);
            send(client_socket, buffer, strlen(buffer), 0);
            break; // Exit loop after modifying the book
        }
    }

    fclose(file);

    if (!found)
    {
        send(client_socket, "Book not found.\n", strlen("Book not found.\n"), 0);
    }
}

// Function to search for a book in the database file
void search_book(int client_socket)
{
    char search_query[100];
    char buffer[256];

    // Prompt user for search query
    send(client_socket, "Enter search query (title, author, or book ID): ", strlen("Enter search query (title, author, or book ID): "), 0);
    recv(client_socket, search_query, sizeof(search_query), 0);
    search_query[strcspn(search_query, "\n")] = 0; // Remove newline character

    FILE *file = fopen(FILENAME, "rb"); // Open file in read mode for binary reading
    if (file == NULL)
    {
        printf("Error opening file.\n");
        return; // Return if unable to open file
    }

    int found = 0; // Flag to indicate if any matching books were found

    // Read book records from the file and search for matches based on the search query
    int current_book_id;
    char current_title[100];
    char current_author[100];

    while (fread(&current_book_id, sizeof(int), 1, file) == 1 &&
           fread(current_title, sizeof(char), 100, file) == 100 &&
           fread(current_author, sizeof(char), 100, file) == 100)
    {
        // Check if the search query matches the book ID, title, or author
        if (strcmp(search_query, current_title) == 0 ||
            strcmp(search_query, current_author) == 0 ||
            atoi(search_query) == current_book_id)
        {
            // Send matching book details to the client
            sprintf(buffer, "Book ID: %d\nTitle: %s\nAuthor: %s\n", current_book_id, current_title, current_author);
            send(client_socket, buffer, strlen(buffer), 0);
            found = 1; // Set flag to indicate matching book found
        }
    }

    fclose(file);

    if (!found)
    {
        send(client_socket, "No matching books found.\n", strlen("No matching books found.\n"), 0);
    }
}

// Function to manage members
void manage_members(int client_socket)
{
    char choice;
    do
    {
        // Send menu options to client
        send(client_socket, "\nMember Management Menu:\n", strlen("\nMember Management Menu:\n"), 0);
        send(client_socket, "1. Add Member\n", strlen("1. Add Member\n"), 0);
        send(client_socket, "2. Delete Member\n", strlen("2. Delete Member\n"), 0);
        send(client_socket, "3. Modify Member\n", strlen("3. Modify Member\n"), 0);
        send(client_socket, "4. Search Member\n", strlen("4. Search Member\n"), 0);
        send(client_socket, "5. Exit\n", strlen("5. Exit\n"), 0);
        send(client_socket, "Enter your choice: ", strlen("Enter your choice: "), 0);

        // Receive choice from client
        recv(client_socket, &choice, sizeof(char), 0);

        // Perform actions based on choice
        switch (choice)
        {
        case '1':
            add_member(client_socket);
            break;
        case '2':
            delete_member(client_socket);
            break;
        case '3':
            modify_member(client_socket);
            break;
        case '4':
            search_member(client_socket);
            break;
        case '5':
            // Exit the function
            return;
        default:
            send(client_socket, "Invalid choice. Please try again.\n", strlen("Invalid choice. Please try again.\n"), 0);
        }
    } while (choice != '5');
}

// Function to add a member
void add_member(int client_socket)
{
    Member new_member;
    char buffer[256];

    // Prompt user for member details
    send(client_socket, "Enter member name: ", strlen("Enter member name: "), 0);
    recv(client_socket, new_member.name, sizeof(new_member.name), 0);
    new_member.name[strcspn(new_member.name, "\n")] = 0; // Remove newline character

    send(client_socket, "Enter member address: ", strlen("Enter member address: "), 0);
    recv(client_socket, new_member.address, sizeof(new_member.address), 0);
    new_member.address[strcspn(new_member.address, "\n")] = 0; // Remove newline character

    // Open file for appending
    FILE *file = fopen(MEMBER_FILE, "ab");
    if (file == NULL)
    {
        printf("Error opening file.\n");
        return; // Return if unable to open file
    }

    // Write member details to file
    if (fwrite(&new_member, sizeof(Member), 1, file) != 1)
    {
        printf("Error writing to file.\n");
        fclose(file);
        return; // Return if unable to write to file
    }

    fclose(file);

    // Send success message to client
    sprintf(buffer, "Member added successfully.\nMember ID: %d\n", new_member.member_id);
    send(client_socket, buffer, strlen(buffer), 0);
}

// Function to delete a member
void delete_member(int client_socket)
{
    int delete_id;
    char buffer[256];
    int found = 0;

    // Prompt user for member ID to delete
    send(client_socket, "Enter member ID to delete: ", strlen("Enter member ID to delete: "), 0);
    recv(client_socket, &delete_id, sizeof(int), 0);

    // Open the file for reading and writing
    FILE *file = fopen(MEMBER_FILE, "rb+");
    if (file == NULL)
    {
        printf("Error opening file.\n");
        return; // Return if unable to open file
    }

    // Temporary file for writing non-deleted members
    FILE *temp_file = fopen("temp.dat", "wb");
    if (temp_file == NULL)
    {
        printf("Error creating temporary file.\n");
        fclose(file);
        return; // Return if unable to create temporary file
    }

    // Read member records from the file
    Member current_member;
    while (fread(&current_member, sizeof(Member), 1, file) == 1)
    {
        // Check if the member ID matches the one to be deleted
        if (current_member.member_id == delete_id)
        {
            found = 1; // Set flag to indicate member found
            continue;  // Skip writing this member to temporary file (deleting)
        }

        // Write non-deleted member records to temporary file
        if (fwrite(&current_member, sizeof(Member), 1, temp_file) != 1)
        {
            printf("Error writing to temporary file.\n");
            fclose(file);
            fclose(temp_file);
            return; // Return if unable to write to temporary file
        }
    }

    fclose(file);
    fclose(temp_file);

    // Replace original file with temporary file
    if (remove(MEMBER_FILE) != 0 || rename("temp.dat", MEMBER_FILE) != 0)
    {
        printf("Error updating file.\n");
        return; // Return if unable to update file
    }

    // Send appropriate message to client
    if (found)
    {
        sprintf(buffer, "Member with ID %d deleted successfully.\n", delete_id);
        send(client_socket, buffer, strlen(buffer), 0);
    }
    else
    {
        send(client_socket, "Member not found.\n", strlen("Member not found.\n"), 0);
    }
}

// Function to modify a member
void modify_member(int client_socket)
{
    int modify_id;
    char buffer[256];
    int found = 0;

    // Prompt user for member ID to modify
    send(client_socket, "Enter member ID to modify: ", strlen("Enter member ID to modify: "), 0);
    recv(client_socket, &modify_id, sizeof(int), 0);

    // Open the file for reading and writing
    FILE *file = fopen(MEMBER_FILE, "rb+");
    if (file == NULL)
    {
        printf("Error opening file.\n");
        return; // Return if unable to open file
    }

    // Read and update member records in the file
    Member current_member;
    while (fread(&current_member, sizeof(Member), 1, file) == 1)
    {
        // Check if the member ID matches the one to be modified
        if (current_member.member_id == modify_id)
        {
            found = 1; // Set flag to indicate member found

            // Prompt user for new member details
            send(client_socket, "Enter new member name: ", strlen("Enter new member name: "), 0);
            recv(client_socket, current_member.name, sizeof(current_member.name), 0);
            current_member.name[strcspn(current_member.name, "\n")] = 0; // Remove newline character

            send(client_socket, "Enter new member address: ", strlen("Enter new member address: "), 0);
            recv(client_socket, current_member.address, sizeof(current_member.address), 0);
            current_member.address[strcspn(current_member.address, "\n")] = 0; // Remove newline character

            // Move file pointer back to the beginning of the current record for modification
            fseek(file, -sizeof(Member), SEEK_CUR);

            // Write modified member details to file
            if (fwrite(&current_member, sizeof(Member), 1, file) != 1)
            {
                printf("Error writing to file.\n");
                fclose(file);
                return; // Return if unable to write to file
            }

            // Send success message to client
            sprintf(buffer, "Member with ID %d modified successfully.\n", modify_id);
            send(client_socket, buffer, strlen(buffer), 0);
            break; // Exit loop after modifying the member
        }
    }

    fclose(file);

    // Send appropriate message to client if member not found
    if (!found)
    {
        send(client_socket, "Member not found.\n", strlen("Member not found.\n"), 0);
    }
}

// Function to search for a member
void search_member(int client_socket)
{
    char search_query[100];
    char buffer[256];
    int found = 0;

    // Prompt user for search query
    send(client_socket, "Enter search query (member ID, name, or address): ", strlen("Enter search query (member ID, name, or address): "), 0);
    recv(client_socket, search_query, sizeof(search_query), 0);
    search_query[strcspn(search_query, "\n")] = 0; // Remove newline character

    // Open the file for reading
    FILE *file = fopen(MEMBER_FILE, "rb");
    if (file == NULL)
    {
        printf("Error opening file.\n");
        return; // Return if unable to open file
    }

    // Read and search member records in the file
    Member current_member;
    while (fread(&current_member, sizeof(Member), 1, file) == 1)
    {
        // Check if the search query matches the member ID, name, or address
        if (strstr(search_query, "ID") != NULL && atoi(search_query + 3) == current_member.member_id)
        {
            found = 1;
        }
        else if (strstr(search_query, "name") != NULL && strstr(current_member.name, search_query + 5) != NULL)
        {
            found = 1;
        }
        else if (strstr(search_query, "address") != NULL && strstr(current_member.address, search_query + 8) != NULL)
        {
            found = 1;
        }

        // If member matches search query, send member details to client
        if (found)
        {
            sprintf(buffer, "Member ID: %d\nName: %s\nAddress: %s\n", current_member.member_id, current_member.name, current_member.address);
            send(client_socket, buffer, strlen(buffer), 0);
            found = 0; // Reset found flag for next search
        }
    }

    fclose(file);

    // If no matching members found, send message to client
    if (!found)
    {
        send(client_socket, "No matching members found.\n", strlen("No matching members found.\n"), 0);
    }
}

// Function to insert a book into the database file
int insert_book_to_database(int book_id, char *title, char *author)
{
    FILE *file = fopen(FILENAME, "ab"); // Open file in append mode for binary writing
    if (file == NULL)
    {
        printf("Error opening file.\n");
        return 0; // Return failure if unable to open file
    }

    // Write book details to file
    if (fwrite(&book_id, sizeof(int), 1, file) != 1 ||
        fwrite(title, sizeof(char), strlen(title) + 1, file) != strlen(title) + 1 ||
        fwrite(author, sizeof(char), strlen(author) + 1, file) != strlen(author) + 1)
    {
        printf("Error writing to file.\n");
        fclose(file);
        return 0; // Return failure if unable to write to file
    }

    fclose(file);
    return 1; // Return success
}

// Function to delete a book from the database file
int delete_book_from_database(int book_id)
{
    FILE *file = fopen(FILENAME, "rb"); // Open file in read mode for binary reading
    if (file == NULL)
    {
        printf("Error opening file.\n");
        return 0; // Return failure if unable to open file
    }

    FILE *temp_file = fopen("temp.dat", "wb"); // Open a temporary file for writing
    if (temp_file == NULL)
    {
        printf("Error creating temporary file.\n");
        fclose(file);
        return 0; // Return failure if unable to create temporary file
    }

    int found = 0; // Flag to indicate if the book was found and deleted

    // Read book records from the file
    int current_book_id;
    char title[100];
    char author[100];

    while (fread(&current_book_id, sizeof(int), 1, file) == 1 &&
           fread(title, sizeof(char), 100, file) == 100 &&
           fread(author, sizeof(char), 100, file) == 100)
    {
        if (current_book_id == book_id)
        {
            found = 1; // Set flag to indicate book found
        }
        else
        {
            // Write book details to temporary file (excluding the one to be deleted)
            fwrite(&current_book_id, sizeof(int), 1, temp_file);
            fwrite(title, sizeof(char), strlen(title) + 1, temp_file);
            fwrite(author, sizeof(char), strlen(author) + 1, temp_file);
        }
    }

    fclose(file);
    fclose(temp_file);

    if (!found)
    {
        printf("Book not found.\n");
        remove("temp.dat"); // Delete temporary file
        return 0;           // Return failure if book not found
    }

    // Replace original file with temporary file
    if (remove(FILENAME) != 0 || rename("temp.dat", FILENAME) != 0)
    {
        printf("Error updating file.\n");
        return 0; // Return failure if unable to update file
    }

    return 1; // Return success
}

int generate_unique_book_id()
{
    static int counter = 0;
    return ++counter;
}

// Function to issue a book to a member
void issue_book(int client_socket)
{
    int member_id, book_id;
    printf("Enter your member_id\n");
    scanf("%d", &member_id);
    printf("Enter the ID of the book you want to issue.\n");
    scanf("%d", &book_id);
    // Open the file for reading and writing in binary mode
    FILE *file = fopen(FILENAME, "rb+");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    // Search for the book in the file
    Book book;
    int found = 0;
    while (fread(&book, sizeof(Book), 1, file) == 1)
    {
        if (book.book_id == book_id)
        {
            found = 1;
            break;
        }
    }

    // If the book is found, check if it is available for issue
    if (found)
    {
        if (book.is_issued == 1)
        {
            // Book is already issued, send a message to the client
            send(client_socket, "Book is already issued to another member.\n", strlen("Book is already issued to another member.\n"), 0);
        }
        else
        {
            // Book is available, update its status and issued_to fields
            book.is_issued = 1;
            book.issued_to = member_id;

            // Move the file pointer back to update the record
            fseek(file, -sizeof(Book), SEEK_CUR);
            fwrite(&book, sizeof(Book), 1, file);

            // Send a confirmation message to the client
            send(client_socket, "Book issued successfully.\n", strlen("Book issued successfully.\n"), 0);
        }
    }
    else
    {
        // Book not found, send a message to the client
        send(client_socket, "Book not found.\n", strlen("Book not found.\n"), 0);
    }

    // Close the file
    fclose(file);
}

// Function to return a book by a member
void return_book(int client_socket)
{
    int member_id, book_id;
    printf("Enter your member_id\n");
    scanf("%d", &member_id);
    printf("Enter the ID of the book you want to return.\n");
    scanf("%d", &book_id);
    // Open the file for reading and writing in binary mode
    FILE *file = fopen(FILENAME, "rb+");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    // Search for the book in the file
    Book book;
    int found = 0;
    while (fread(&book, sizeof(Book), 1, file) == 1)
    {
        if (book.book_id == book_id)
        {
            found = 1;
            break;
        }
    }

    // If the book is found, check if it is issued to the specified member
    if (found)
    {
        if (book.is_issued == 0 || book.issued_to != member_id)
        {
            // Book is either not issued or not issued to this member, send a message to the client
            send(client_socket, "Book is not issued to this member.\n", strlen("Book is not issued to this member.\n"), 0);
        }
        else
        {
            // Book is issued to this member, update its status and issued_to fields
            book.is_issued = 0;
            book.issued_to = -1; // Set issued_to to -1 to indicate that the book is not issued to anyone

            // Move the file pointer back to update the record
            fseek(file, -sizeof(Book), SEEK_CUR);
            fwrite(&book, sizeof(Book), 1, file);

            // Send a confirmation message to the client
            send(client_socket, "Book returned successfully.\n", strlen("Book returned successfully.\n"), 0);
        }
    }
    else
    {
        // Book not found, send a message to the client
        send(client_socket, "Book not found.\n", strlen("Book not found.\n"), 0);
    }

    // Close the file
    fclose(file);
}