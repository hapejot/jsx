#include "js.h"

static JSErrorFormatString format_string[JSErr_Limit] = 
{
#define MSG_DEF(name, number, count, exception, format) \
    { format, count } ,
#include "jsshell.msg"
#undef MSG_DEF
};

const JSErrorFormatString *
jshell_GetErrorMessage(
    void *          userRef, 
    const char *    locale, 
    const uintN     errorNumber)
{
    if ((errorNumber > 0) && (errorNumber < JSShellErr_Limit))
        return &format_string[errorNumber];
    return NULL;
}

