#ifndef USERS_H
#define USERS_H

typedef struct {
    char *username;
    char *password;
    int is_admin; // 1 for admin, 0 for user
} User;

User users[] = {
    {"admin1", "adminpass", 1},
    {"user1", "userpass", 0},
    {"admin2", "adminpass2", 1},
    {"user2", "userpass2", 0},
    {"f","f",0},
    {"j","j",1}
};

#endif
