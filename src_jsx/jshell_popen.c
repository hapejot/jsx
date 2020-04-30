#include "js.h"

JSBool
jshell_popen(
    JSContext * cx, 
    JSObject *  obj, 
    uintN       argc, 
    jsval *     argv, 
    jsval *     rval)
{
    JSString *  str         = NULL;
    JSBool      status      = JS_TRUE;
	char*		name		= NULL;
	char*		ptype		= "r+";
	FILE*		f;
    int         rc;

    if( argc >= 1 )
    {
        str = JS_ValueToString(cx, argv[0]);
        if (!str)
            status = JS_FALSE;
        else
        {
			name = JS_GetStringBytes(str);
			if( argc == 2 )
			{
        		str = JS_ValueToString(cx, argv[1]);
				ptype = JS_GetStringBytes(str);
			}

            system( JS_GetStringBytes(str) ); 
            *rval = INT_TO_JSVAL( rc );
        }
    }
    else
        status = JS_FALSE;

    return status;
}

