/*
 * $Id: //depot/peter/w/js/main.c#18 $
 */
#include "js.h"

int
main(
    int             argc, 
    char **         argv, 
    char **         envp)
{
    JSRuntime*  rt;
    JSContext*  cx;
    JSObject*   glob;

    jsinit( argc, argv, envp, &rt, &cx, &glob );
    if( argc > 1 )
        jshell_process( cx, glob, argv[1] );
    else
        jshell_process( cx, glob, "-" );
    jsexit( rt, cx );
    return 0;
}
