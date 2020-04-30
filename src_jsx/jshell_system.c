#include "js.h"

JSBool
jshell_system(
    JSContext * cx, 
    JSObject *  obj, 
    uintN       argc, 
    jsval *     argv, 
    jsval *     rval)
{
    JSString *  str         = NULL;
    JSBool      status      = JS_TRUE;
    int         rc;

    if( argc == 1 )
    {
        str = JS_ValueToString(cx, argv[0]);
        if (!str)
            status = JS_FALSE;
        else
        {
            rc = system( JS_GetStringBytes(str) ); 
            *rval = INT_TO_JSVAL( rc );
        }
    }
    else
        status = JS_FALSE;

    return status;
}

