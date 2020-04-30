#include "platform.h"
#include "js.h"

static JSClass      process_class;
struct process 
{
    JSFunction*     func;
    JSObject*       obj;
    JSContext*      cx;
	char*			exe;
	int				fdIn;
	int				fdOut;
	char**			argv;
};

static JSBool       process_write         
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result
)
{
    JSBool          status  = JS_TRUE;
	char*    		str = NULL;
    struct process* priv    = JS_GetInstancePrivate(p_cx, p_obj, &process_class, NULL);

	if( p_argc >= 1 )
	{
		str = JS_GetStringBytes(JS_ValueToString(p_cx, p_argv[0]));
		/* write data to process input fd */
		write(priv->fdIn, str, strlen(str));
	}
	else
	{
		status = JS_FALSE;
	}
    return status;
}

static JSBool       process_read         
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result
)
{
    JSBool          status  = JS_TRUE;
	char*    		str;
    struct process* priv    = JS_GetInstancePrivate(p_cx, p_obj, &process_class, NULL);
	int 			n;

	str = malloc(0x10000);
	n = read(priv->fdOut, str, 0x10000);
	*p_result = STRING_TO_JSVAL(JS_NewStringCopyN(p_cx, str, n));
	/* read from output fd */
    return status;
}

static JSBool       process_exec         
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result
)
{
    JSBool          status  = JS_TRUE;
	char*    		str;
    struct process* priv    = JS_GetInstancePrivate(p_cx, p_obj, &process_class, NULL);
	int 			n;
	int				i;
	int				pid;
	int				fdIn[2];
	int				fdOut[2];

	priv->argv = calloc( p_argc+2, sizeof(char*) );
	priv->argv[0] = priv->exe;
	for(i = 0; i < p_argc; i++)
	{
		priv->argv[i+1] = strdup(JS_GetStringBytes(JSVAL_TO_STRING(p_argv[i])));
	}
	priv->argv[p_argc+1] = NULL;

	pipe(fdIn);
	pipe(fdOut);

	pid = vfork();
	if( pid == -1 )		// Error
	{
		status = JS_FALSE;
	}
	else if( pid == 0 ) // Child
	{
		dup2(fdIn[1], STDIN_FILENO);
		dup2(fdOut[0], STDOUT_FILENO);
		close(fdIn[0]);
		close(fdOut[1]);
		execv(priv->exe, priv->argv);
	}
	// Parent
	close(fdIn[1]);
	close(fdOut[0]);
	priv->fdIn = fdIn[0];
	priv->fdOut = fdOut[1];
	*p_result = INT_TO_JSVAL(pid);
    return status;
}

static JSBool       process_close         
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result
)
{
    JSBool          status  = JS_TRUE;
	char*    		str = NULL;
    struct process* priv    = JS_GetInstancePrivate(p_cx, p_obj, &process_class, NULL);

    return status;
}

static JSBool       process_constructor 
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result
)
{
    struct process* priv;
    int             status;
	JSString*		str;

    priv = JS_malloc( p_cx, sizeof(struct process) );
    priv->cx = p_cx;
	priv->obj = p_obj;
	str = JS_ValueToString(p_cx, p_argv[0]);
	priv->exe = strdup(JS_GetStringBytes(str));
    JS_SetPrivate( p_cx, p_obj, priv );
    return JS_TRUE;
}

#define JSPROCESS_EXE			-1
#define JSPROCESS_STDINFD		-2
#define JSPROCESS_STDOUTFD		-3
#define JSPROCESS_INPUTENDED	-4

static JSBool       process_get_property
(
    JSContext *     p_cx, 
    JSObject *      p_obj, 
    jsval           p_id, 
    jsval *         p_vp)
{
    JSBool          status  = JS_TRUE;
    jsint           id;
    struct process* priv    = JS_GetInstancePrivate(p_cx, p_obj, &process_class, NULL);
    id = JSVAL_TO_INT(p_id);
    switch( id )
    {
    }
    return status;
}

static JSBool       process_set_property
(
    JSContext *     p_cx, 
    JSObject *      p_obj, 
    jsval           p_id, 
    jsval *         p_vp)
{
    JSBool          status  = JS_TRUE;
    struct process* priv    = JS_GetInstancePrivate(p_cx, p_obj, &process_class, NULL);
    int             id;

    id = JSVAL_TO_INT( p_id );
    switch( id )
    {
		case JSPROCESS_INPUTENDED:
			close(priv->fdIn);
			break;
    }
    return status;
}

static void
process_finalize
(
    JSContext *     p_cx,
    JSObject *      p_obj)
{
    struct process*  priv    = JS_GetInstancePrivate(p_cx, p_obj, &process_class, NULL);
    if( priv )
    {
		free(priv->exe);
		free(priv);
    }
}


static JSPropertySpec process_props[] = {
    {"Exe",        JSPROCESS_EXE,          JSPROP_ENUMERATE},
    {"StdInFd",        JSPROCESS_STDINFD,          JSPROP_ENUMERATE},
    {"StdOutFd",        JSPROCESS_STDOUTFD,          JSPROP_ENUMERATE},
    {"InputEnded",        JSPROCESS_INPUTENDED,          JSPROP_ENUMERATE},
    {0}
};


static JSFunctionSpec process_methods[] = {
    {"write",      	process_write,  0},
	{"close",		process_close,  0},
	{"read",		process_read,  0},
	{"exec",		process_exec,  0},
    {0}
};

static JSClass process_class = {
    "Process", 
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub,
    process_get_property,  
    process_set_property,
    JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,
    process_finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};


JSBool jsprocess_initialize(
    JSContext *         p_cx,
    JSObject *          p_obj
)
{
    JSObject *          this;

    JS_InitClass( p_cx, p_obj, NULL, 
            &process_class, 
            process_constructor, 0, 
            process_props, 
            process_methods,
            NULL,
            NULL );
}
