#include "screen_locker/user.h"
#include "screen_locker/env.h"

#include <security/pam_appl.h>

#include <string.h>

int on_converse(int                        num_msg,
                const struct pam_message** msg,
                struct pam_response**      resp,
                void*                      appdata_ptr)
{
    struct pam_response* result = (struct pam_response*) calloc(num_msg, sizeof *resp);
    if (!result)
    {
        return PAM_BUF_ERR;
    }

    const user_t* user = (const user_t*) appdata_ptr;

    for (int i = 0; i < num_msg; i++)
    {
        switch (msg[i]->msg_style)
        {
        case PAM_PROMPT_ECHO_OFF:
        case PAM_PROMPT_ECHO_ON:
            result[i].resp = strdup(user->password);
            if (!result[i].resp)
            {
                goto fail;
            }
            break;
        case PAM_ERROR_MSG:
        case PAM_TEXT_INFO: break;
        default           : goto fail;
        }
    }
    *resp = result;
    return PAM_SUCCESS;

fail:
    for (int i = 0; i < num_msg; i++)
    {
        if (result[i].resp)
        {
            explicit_bzero(result[i].resp, strlen(result[i].resp));
            free(result[i].resp);
        }
    }

    free(result);
    return PAM_CONV_ERR;
}

bool sl_auth_login(user_t* pw)
{
    struct pam_conv conv;
    conv.conv        = on_converse;
    conv.appdata_ptr = pw;

    const char* login_service = get_env_str(SL_ENV_LOGIN_SERVICE, SL_ENV_LOGIN_SERVICE_DEFAULT);

    pam_handle_t* handle = NULL;
    if (pam_start(login_service, pw->user_name, &conv, &handle) != PAM_SUCCESS)
    {
        return false;
    }

    const bool login = pam_authenticate(handle, 0) == PAM_SUCCESS;
    pam_end(handle, login ? PAM_SUCCESS : PAM_AUTH_ERR);

    return login;
}
