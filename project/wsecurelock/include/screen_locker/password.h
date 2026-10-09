#pragma once

#include <stdbool.h>

typedef struct
{
    char buffer[256];
    int  length;
} password_t;

bool password_is_empty(const password_t* password);
bool password_authenticate(password_t* pw);
void password_reset(password_t* pw);

void password_pop(password_t* pw);
void password_append(password_t* pw, const char* str, int len);

int password_count_chars(const password_t* pw);
