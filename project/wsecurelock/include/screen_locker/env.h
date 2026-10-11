#pragma once

#include <stdlib.h>

#define SL_ENV_LOGIN_SERVICE "SL_ENV_LOGIN_SERVICE"
#define SL_ENV_LOGIN_SERVICE_DEFAULT "login"

const char* get_env_str(const char* name, const char* def)
{
    const char* val = getenv(name);
    if (val == NULL)
    {
        return def;
    }
    return val;
}
