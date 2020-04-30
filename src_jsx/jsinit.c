/*
 * $Id: //depot/peter/w/js/jsinit.c#3 $
 */
#include "js.h"

FILE *              gErrFile;
FILE *              gOutFile;
size_t              gStackChunkSize         = 8192;

JSClass jshell_class = {
    "jshell", 
    JSCLASS_NEW_RESOLVE,
    JS_PropertyStub,  
    JS_PropertyStub,
    JS_PropertyStub,  
    JS_PropertyStub,
    JS_EnumerateStandardClasses, 
    JS_ResolveStub,
    JS_ConvertStub,   
    JS_FinalizeStub
};

/* report an error during a load 
 * wraps the error reporter for jsshel...
 * if no report given, just print message.
 * (where does a report come from anyway?)
 *
 * if a report is given don't print exceptions (why not?)
 */
static void
load_err_rpt( 
		JSContext *		cx, 
		const char *	message, 
		JSErrorReport *	report)
{
    if (!report) {
        fprintf(gErrFile, "%s\n", message);
        return;
    }

    /* Ignore any exceptions */
    if (JSREPORT_IS_EXCEPTION(report->flags))
        return;

    /* Otherwise, fall back to the ordinary error reporter. */
    jshell_error_reporter(cx, message, report);
}

static int
is_there(char *candidate)
{
	struct stat 	fin;
	int				result = 0;

	/* XXX work around access(2) false positives for superuser */
	if (access(candidate, R_OK) == 0 &&
		stat(candidate, &fin) == 0 &&
		S_ISREG(fin.st_mode) )
	{
		result = 1;
	}
	return result;
}

char* xstrsep( char** str, char* pat )
{
    char*   t;
    char*   result = NULL;
    assert( str != NULL );
    if( *str != NULL )
    {
        result = *str;
        t = strstr( *str, pat );
        if( t == NULL )
        {
            *str = NULL;
        }
        else
        {
            *t = '\0';
            *str = t + strlen(pat);
        }
    }
    return result;
}

/*
 * walking thorugh all path componentes and test if a file exists that
 * is prefixed by that path component.
 *
 * Errors:
 * 		buffer for path component too small.
 */
static int
get_first_match(
		char *p_path, 
		const char *filename,
		int	  sizeof_candidate,
		char *candidate)
{
	const char *	d               = NULL;
	int 			found           = 0;
	char			pbuf[PATH_MAX]  = { '\0' };
	char *			path            = pbuf;
    static char     pathsep[]       = { PATHSEP, '\0' };

    if( p_path != NULL )
    {
        strcpy( path, p_path );

        found = 0;
        while ((d = xstrsep(&path, pathsep)) != NULL) 
        {
            if (*d == '\0')
            d = ".";
            if (sprintf(candidate, "%s%c%s", d, FILESEPARATOR, filename) 
                    >= sizeof_candidate)
            {
                fprintf( stderr, "candidate buffer too short when searching for '%s/%s'.\n", d, filename);
                continue;
            }
            if (is_there(candidate)) 
            {
                found = 1;
                break;
            }
        }
    }
	// found = (found ? 0 : -1);
	return found;
}
 


/*
 * load a script using special error reporting
 * each parameter is treated as a file name to look for.
 */
static JSBool
load(
		JSContext *cx, 
		JSObject *obj, 
		uintN argc, 
		jsval *argv, 
		jsval *rval)
{
    uintN               i;
    JSString *          str;
    const char *        filename;
	char				full_path[PATH_MAX];
    JSScript *          script;
    JSBool              ok;
    jsval               result;
    JSErrorReporter     older;
    uint32              oldopts;

    for (i = 0; i < argc; i++) {
        str = JS_ValueToString(cx, argv[i]);
        if (!str)
            return JS_FALSE;
        argv[i] = STRING_TO_JSVAL(str);
        filename = JS_GetStringBytes(str);
		if( get_first_match( getenv("JS_PATH"), filename, sizeof(full_path),
						full_path ) )
		{
			errno = 0;
			older = JS_SetErrorReporter(cx, load_err_rpt);
			oldopts = JS_GetOptions(cx);
			JS_SetOptions(cx, oldopts | JSOPTION_COMPILE_N_GO);
			script = JS_CompileFile(cx, obj, full_path);
			if (!script) {
				ok = JS_FALSE;
			} else {
				ok = JS_ExecuteScript(cx, obj, script, &result);
				JS_DestroyScript(cx, script);
			}
			JS_SetOptions(cx, oldopts);
			JS_SetErrorReporter(cx, older);
		}
        if (!ok)
            return JS_FALSE;
    }

    return JS_TRUE;
}

static JSFunctionSpec shell_functions[] =
{
    {"print",       jshell_print,       1},
    {"load",        load,               1},
    {"system",      jshell_system,      1},
    {0}
};

int
jsinit(
    int             argc, 
    char **         argv, 
    char **         envp,
    JSRuntime **    p_rt,
    JSContext **    p_cx,
    JSObject **     p_glob
    )
{
    JSRuntime *     rt;
    JSContext *     cx;
    JSVersion       version;
    JSObject *      glob;
    JSObject *      js_argv;
	JSObject * 		obj;
    int             result	= 0;
    int             i;
    jsval           val;
#define PRINT_CONST(x) fprintf(stderr, "%-30s %d\n", #x, x)
//
//  PRINT_CONST(JS_HAS_ERROR_EXCEPTIONS);
//  PRINT_CONST(JS_HAS_EXCEPTIONS);
#undef PRINT_CONST
	

    gErrFile = stderr;
    gOutFile = stdout;

    version = JSVERSION_DEFAULT;

    argc--;
    argv++;

    rt = JS_NewRuntime(64L * 1024L * 1024L);
    if (!rt)
        return 1;

	if( rt )
	{
		// struct lconv * conv = localeconv();
		// setlocale(LC_ALL, getenv("LC_ALL"));
		// rt->thousandsSeparator = strdup(conv->thousands_sep);
		// rt->decimalSeparator = strdup(conv->decimal_point);
		// rt->numGrouping = strdup(conv->grouping);
	}

    cx = JS_NewContext(rt, gStackChunkSize);
    if (!cx)
        return 1;
    JS_SetErrorReporter(cx, jshell_error_reporter);

    glob = JS_NewObject(cx, &jshell_class, NULL, NULL);
    if (!glob)
        return 1;
    if (!JS_InitStandardClasses(cx, glob))
        return 1;
    // jsits_init_class( cx, glob );
    // js_net_initialize( cx, glob );
    // jsthread_initialize( cx, glob );
    // js_sap_initialize( cx, glob );
    // js_odbc_initialize( cx, glob );
    // jsos_initialize( cx, glob );
	// sqlite_initialize( cx, glob );
	// jsprocess_initialize(cx, glob);
    if (!JS_DefineFunctions(cx, glob, shell_functions))
        return 1;

    /* Set version only after there is a global object. */
    js_argv = JS_NewArrayObject( cx, 0, NULL );
    for( i = 0; i < argc; i++ )
    {
        JSString *  str;
        jsval       val;
        str = JS_NewStringCopyZ( cx, argv[i] );
        val = STRING_TO_JSVAL( str );
        JS_SetElement( cx, js_argv, i, &val );
    }
    val = OBJECT_TO_JSVAL( js_argv );
    JS_SetProperty( cx, glob, "argv", &val );

	obj = JS_NewObject( cx, NULL, NULL, NULL );
    val = OBJECT_TO_JSVAL( obj );
    JS_SetProperty( cx, glob, "VERSION", &val );

	val = INT_TO_JSVAL( BUILDNO );
	JS_SetProperty( cx, obj, "build", &val );

    obj = JS_NewObject( cx, NULL, NULL, NULL );
	if( envp )
	{
	for( i = 0; envp[i] != NULL; i++ )
    {
        char*   p;
        p = strchr( envp[i], '=' );
        if( p )
        {
            JSString*  str;
            *p = 0;
            str = JS_NewStringCopyZ( cx, p+1 );
            val = STRING_TO_JSVAL( str );
            JS_SetProperty( cx, obj, envp[i], &val );
            *p = '=';
        }
    }
	}
    val = OBJECT_TO_JSVAL( obj );
    JS_SetProperty( cx, glob, "Env", &val );

    *p_cx = cx;
    *p_rt = rt;
    *p_glob = glob;
}

void jsexit(
    JSRuntime *    p_rt,
    JSContext *    p_cx
        )
{
    JS_DestroyContext(p_cx);
    JS_DestroyRuntime(p_rt);
    JS_ShutDown();
}
