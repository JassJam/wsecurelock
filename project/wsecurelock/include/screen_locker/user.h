#pragma once

#include <stdbool.h>

typedef struct user_t
{
    char user_name[64];
    char password[256];
    int  password_length;
} user_t;

bool password_is_empty(const user_t* password);
bool password_authenticate(user_t* pw);
void password_reset(user_t* pw);

void password_pop(user_t* pw);
void password_append(user_t* pw, const char* str, int len);

int password_count_chars(const user_t* pw);
