#include "screen_locker/user.h"
#include "screen_locker/auth.h"

#include <string.h>

bool password_is_empty(const user_t* password)
{
    return password->password_length == 0;
}

void password_reset(user_t* pw)
{
    pw->password_length = 0;
    explicit_bzero(pw->password, sizeof(pw->password));
}

bool password_authenticate(user_t* pw)
{
    pw->password[pw->password_length] = '\0';
    return sl_auth_login(pw);
}

void password_pop(user_t* pw)
{
    if (pw->password_length == 0)
    {
        return;
    }
    pw->password_length--;
}

void password_append(user_t* pw, const char* str, int len)
{
    const int max_buffer_size = sizeof(pw->password) - 1;

    int copy_len = len;
    if (pw->password_length + copy_len > max_buffer_size)
    {
        copy_len = max_buffer_size - pw->password_length;
    }
    if (copy_len != 0)
    {
        memcpy(pw->password + pw->password_length, str, copy_len);
        pw->password_length += copy_len;
    }
}

int password_count_chars(const user_t* pw)
{
    int n = 0;
    for (size_t i = 0; i < pw->password_length; i++)
    {
        if (((unsigned char) pw->password[i] & 0xC0) != 0x80)
        {
            n++;
        }
    }
    return n;
}
