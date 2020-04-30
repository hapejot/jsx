#include "js.h"

void
jshell_process(
    JSContext * cx, 
    JSObject *  obj, 
    char *      filename)
{
    JSScript *  script;
    jsval       result;
    FILE *      file;
	char *		source = NULL;
	char *		buffer = NULL;
	int			i		= 0;
	int			c       = 0;
    int         n       = 4000;

    if (!filename || strcmp(filename, "-") == 0) 
    {
        file = stdin;
    } 
    else 
    {
        file = fopen(filename, "r");
        if (!file) 
        {
/*
			JS_ReportErrorNumber(cx, NULL, NULL,
                               JSSMSG_CANT_OPEN, filename, strerror(errno));
            gExitCode = EXITCODE_FILE_NOT_FOUND;
*/
            return;
        }
    }
    while( c != -1 )
    {
        buffer = realloc(buffer, n);
        while( i < n && (c = fgetc(file)) != -1 )
        {
            buffer[i++] = c;
        }
        n <<= 1;
    }
	buffer[i]=0;
	n = i;
	fclose(file);

	if( buffer[0] == '#' && buffer[1] == '!' )
	{
		for( i = 2; i < n; i++ )
			if( buffer[i] == '\n' ) break;
		source = buffer+i;
	}
	else
	{
		source = buffer;
		i = 0;
	}
    script = JS_CompileScript(cx, obj, source, n - i, filename, 1);
    if (script) 
    {
        (void)JS_ExecuteScript(cx, obj, script, &result);
        JS_DestroyScript(cx, script);
    }
    return;
}

