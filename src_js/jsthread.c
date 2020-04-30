#include "platform.h"
#include "js.h"

static JSClass      thread_class;
struct thread 
{
    JSFunction*     func;
    JSObject*       obj;
    JSContext*      cx;
//  pthread_t       thread;
};

static JSBool       thread_create         
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result)
{
    JSBool          status  = JS_TRUE;
    return status;
}

static void*        thread_proc
(
    void*           arg
)
{
    struct thread*  priv;
    jsval           v;
    priv = arg;
    JS_CallFunction(priv->cx, priv->obj, priv->func, 0, NULL, &v);
    return NULL;
}

static JSBool       thread_constructor 
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result)
{
    struct thread*  priv;
    JSFunction*     func            = NULL;
    int             status;

    priv = JS_malloc( p_cx, sizeof(struct thread) );
    priv->cx = p_cx;
    if (JSVAL_IS_FUNCTION(p_cx, p_argv[0])) 
        priv->func = JS_GetPrivate(p_cx, JSVAL_TO_OBJECT(p_argv[0]));
    else
        priv->func = NULL;
    JS_SetPrivate( p_cx, p_obj, priv );
//  status = pthread_create( &priv->thread, NULL, thread_proc, priv );
//  pthread_yield();
    return JS_TRUE;
}

static JSBool       thread_get_property
(
    JSContext *     p_cx, 
    JSObject *      p_obj, 
    jsval           p_id, 
    jsval *         p_vp)
{
    JSBool          status  = JS_TRUE;
    jsint           id;
    void*           priv    = JS_GetInstancePrivate(p_cx, p_obj, &thread_class, NULL);
    id = JSVAL_TO_INT(p_id);
    switch( id )
    {
    }
    return status;
}

static JSBool       thread_set_property
(
    JSContext *     p_cx, 
    JSObject *      p_obj, 
    jsval           p_id, 
    jsval *         p_vp)
{
    JSBool          status  = JS_TRUE;
    void*           priv    = JS_GetInstancePrivate(p_cx, p_obj, &thread_class, NULL);
    int             id;

    id = JSVAL_TO_INT( p_id );
    switch( id )
    {
    }
    return status;
}

static void
thread_finalize
(
    JSContext *     p_cx,
    JSObject *      p_obj)
{
    struct thread*  priv    = JS_GetInstancePrivate(p_cx, p_obj, &thread_class, NULL);
//  struct socket * sock    = JS_GetInstancePrivate(p_cx, p_obj, &socket_class, NULL);
    // JS_free( p_cx, sock );
    if( priv )
    {
//      pthread_join( priv->thread, NULL );
    }
}


static JSPropertySpec thread_props[] = {
//  {"blocking",        JSTHREAD_BLOCKING,          JSPROP_ENUMERATE},
    {0}
};


static JSFunctionSpec thread_methods[] = {
//  {"accept",      socket_accept,  0},
    {0}
};

static JSClass thread_class = {
    "Thread", 
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub,
    thread_get_property,  
    thread_set_property,
    JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,
    thread_finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};


JSBool jsthread_initialize(
    JSContext *         p_cx,
    JSObject *          p_obj
)
{
    JSObject *          this;

    JS_InitClass( p_cx, p_obj, NULL, 
            &thread_class, 
            thread_constructor, 0, 
            thread_props, 
            thread_methods,
            NULL,
            NULL );
}
