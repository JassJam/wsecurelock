#include "screen_locker/password.h"

#include <string.h>

bool password_is_empty(const password_t* password)
{
    return password->length == 0;
}

void password_reset(password_t* pw)
{
    pw->length = 0;
    explicit_bzero(pw->buffer, sizeof(pw->buffer));
}

bool password_authenticate(password_t* pw)
{
    pw->buffer[pw->length] = '\0';
    return true;
}

void password_pop(password_t* pw)
{
    if (pw->length == 0)
    {
        return;
    }
    pw->length--;
}

void password_append(password_t* pw, const char* str, int len)
{
    const int max_buffer_size = sizeof(pw->buffer) - 1;

    int copy_len = len;
    if (pw->length + copy_len > max_buffer_size)
    {
        copy_len = max_buffer_size - pw->length;
    }
    if (copy_len != 0)
    {
        memcpy(pw->buffer + pw->length, str, copy_len);
        pw->length += copy_len;
    }
}

int password_count_chars(const password_t* pw)
{
    int n = 0;
    for (size_t i = 0; i < pw->length; i++)
    {
        if (((unsigned char) pw->buffer[i] & 0xC0) != 0x80)
        {
            n++;
        }
    }
    return n;
}
