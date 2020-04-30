#include "js.h"

/*
 * Define a JS object called "it".  Give it class operations that printf why
 * they're being called for tutorial purposes.
 */
enum its_tinyid {
    ITS_COLOR, ITS_HEIGHT, ITS_WIDTH, ITS_FUNNY, ITS_ARRAY, ITS_RDONLY
};

static JSPropertySpec its_props[] = {
    {"color",           ITS_COLOR,      JSPROP_ENUMERATE},
    {"height",          ITS_HEIGHT,     JSPROP_ENUMERATE},
    {"width",           ITS_WIDTH,      JSPROP_ENUMERATE},
    {"funny",           ITS_FUNNY,      JSPROP_ENUMERATE},
    {"array",           ITS_ARRAY,      JSPROP_ENUMERATE},
    {"rdonly",          ITS_RDONLY,     JSPROP_READONLY},
    {0}
};

static JSBool
its_item(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    *rval = OBJECT_TO_JSVAL(obj);
    if (argc != 0)
        JS_SetCallReturnValue2(cx, argv[0]);
    return JS_TRUE;
}

static JSFunctionSpec its_methods[] = {
    {"item",            its_item,       0},
    {0}
};

static JSBool its_noisy;    /* whether to be noisy when finalizing it */

static JSBool
its_addProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    if (its_noisy) {
        fprintf(gOutFile, "adding its property %s,",
               JS_GetStringBytes(JS_ValueToString(cx, id)));
        fprintf(gOutFile, " initial value %s\n",
               JS_GetStringBytes(JS_ValueToString(cx, *vp)));
    }
    return JS_TRUE;
}

static JSBool
its_delProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    if (its_noisy) {
        fprintf(gOutFile, "deleting its property %s,",
               JS_GetStringBytes(JS_ValueToString(cx, id)));
        fprintf(gOutFile, " current value %s\n",
               JS_GetStringBytes(JS_ValueToString(cx, *vp)));
    }
    return JS_TRUE;
}

static JSBool
its_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    if (its_noisy) {
        fprintf(gOutFile, "getting its property %s,",
               JS_GetStringBytes(JS_ValueToString(cx, id)));
        fprintf(gOutFile, " current value %s\n",
               JS_GetStringBytes(JS_ValueToString(cx, *vp)));
    }
    return JS_TRUE;
}

static JSBool
its_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    extern JSClass js_ArrayClass;

    if (its_noisy) {
        fprintf(gOutFile, "setting its property %s,",
               JS_GetStringBytes(JS_ValueToString(cx, id)));
        fprintf(gOutFile, " new value %s\n",
               JS_GetStringBytes(JS_ValueToString(cx, *vp)));
    }
    if (JSVAL_IS_STRING(id) &&
        !strcmp(JS_GetStringBytes(JSVAL_TO_STRING(id)), "noisy")) {
        return JS_ValueToBoolean(cx, *vp, &its_noisy);
    }
    else if( JSVAL_TO_INT(id) == ITS_COLOR )
    {
        JSObject* a;
        jsval       o;
        char*       s;
        int         n;
        int         i;

        s = JS_GetStringBytes(JS_ValueToString(cx, *vp));
        n = strlen(s);
        a = js_ConstructObject(cx, &js_ArrayClass, NULL, NULL, 0, NULL);
        for( i = 0; i < n; i++ )
        {
            o = INT_TO_JSVAL(s[i]);
            JS_SetElement( cx, a, i, &o );
        }
        o = OBJECT_TO_JSVAL(a);
        JS_SetProperty( cx, obj, "array", &o );
    }
    return JS_TRUE;
}

static JSBool
its_enumerate(JSContext *cx, JSObject *obj)
{
    if (its_noisy)
        fprintf(gOutFile, "enumerate its properties\n");
    return JS_TRUE;
}

static JSBool
its_resolve(JSContext *cx, JSObject *obj, jsval id, uintN flags,
            JSObject **objp)
{
    if (its_noisy) {
        fprintf(gOutFile, "resolving its property %s, flags {%s,%s,%s}\n",
               JS_GetStringBytes(JS_ValueToString(cx, id)),
               (flags & JSRESOLVE_QUALIFIED) ? "qualified" : "",
               (flags & JSRESOLVE_ASSIGNING) ? "assigning" : "",
               (flags & JSRESOLVE_DETECTING) ? "detecting" : "");
    }
    return JS_TRUE;
}

static JSBool
its_convert(JSContext *cx, JSObject *obj, JSType type, jsval *vp)
{
    if (its_noisy)
        fprintf(gOutFile, "converting it to %s type\n", JS_GetTypeName(cx, type));
    return JS_TRUE;
}

static void
its_finalize(JSContext *cx, JSObject *obj)
{
    if (its_noisy)
        fprintf(gOutFile, "finalizing it\n");
}




static JSClass its_class = {
    "It", JSCLASS_NEW_RESOLVE,
    its_addProperty,  its_delProperty,  its_getProperty,  its_setProperty,
    its_enumerate,    (JSResolveOp)its_resolve,
    its_convert,      its_finalize
};

static JSBool       its_constructor 
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result)
{
    return JS_TRUE;
}


JSBool
jsits_init_class(
    JSContext*      cx,
    JSObject*       glob)
{
    JSObject*       it;
    
    it = JS_InitClass( cx, glob, NULL,
            &its_class,
            its_constructor, 0,
            its_props,
            its_methods,
            NULL,
            NULL );

    return JS_TRUE;
}

JSBool
jsits_initialize(
    JSContext*      cx,
    JSObject*       glob)
{
    JSObject*       it;
    JSBool          status      = JS_TRUE;
    

    it = JS_DefineObject(cx, glob, "it", &its_class, NULL, 0);
    if (!it)
        status = JS_FALSE;
    if (status && !JS_DefineProperties(cx, it, its_props))
        status = JS_FALSE;
    if (status && !JS_DefineFunctions(cx, it, its_methods))
        status = JS_FALSE;
    return status;
}
