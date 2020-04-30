#include "js.h"
/* SCCSID(jsw, "$Id: //depot/peter/w/js/sqlite.c#2 $"); */
#include "sqlite3.h"
static JSBool x_open(
	  JSContext * cx,
   JSObject *  obj,
   uintN       argc,
   jsval *     argv,
   jsval *     rval)
{
	  int         rc;
    char* arg0; /* In */
    void* arg1; /* Out */
    int result;

    arg0 = JSVAL_IS_NULL(argv[0]) ? NULL : JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
    result = sqlite3_open(
        arg0
       ,&arg1
);

*rval = INT_TO_JSVAL(arg1);
    return JS_TRUE;
}
static JSBool x_close(
	  JSContext * cx,
   JSObject *  obj,
   uintN       argc,
   jsval *     argv,
   jsval *     rval)
{
	  int         rc;
    void* arg0; /* In */
    int result;

    JS_ValueToECMAInt32(cx,argv[0],&arg0);
    result = sqlite3_close(
        arg0
);

    return JS_TRUE;
}
static JSBool x_prepare(
	  JSContext * cx,
   JSObject *  obj,
   uintN       argc,
   jsval *     argv,
   jsval *     rval)
{
	  int         rc;
    void* arg0; /* In */
    char* arg1; /* In */
    int arg2; /* Const */
    void* arg3; /* Out */
    char* arg4; /* Const */
    int result;

    JS_ValueToECMAInt32(cx,argv[0],&arg0);
    arg1 = JSVAL_IS_NULL(argv[1]) ? NULL : JS_GetStringBytes(JSVAL_TO_STRING(argv[1]));
    arg2 = -1;
    arg4 = NULL;
    result = sqlite3_prepare(
        arg0
       ,arg1
       ,arg2
       ,&arg3
       ,arg4
);

*rval = INT_TO_JSVAL(arg3);
    return JS_TRUE;
}
static JSBool x_column_count(
	  JSContext * cx,
   JSObject *  obj,
   uintN       argc,
   jsval *     argv,
   jsval *     rval)
{
	  int         rc;
    void* arg0; /* In */
    int result;

    JS_ValueToECMAInt32(cx,argv[0],&arg0);
    result = sqlite3_column_count(
        arg0
);

*rval = INT_TO_JSVAL(result);
    return JS_TRUE;
}
static JSBool x_column_name(
	  JSContext * cx,
   JSObject *  obj,
   uintN       argc,
   jsval *     argv,
   jsval *     rval)
{
	  int         rc;
    void* arg0; /* In */
    int arg1; /* In */
    char* result;

    JS_ValueToECMAInt32(cx,argv[0],&arg0);
    JS_ValueToECMAInt32(cx,argv[1],&arg1);
    result = sqlite3_column_name(
        arg0
       ,arg1
);

*rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, result));
    return JS_TRUE;
}
static JSBool x_step(
	  JSContext * cx,
   JSObject *  obj,
   uintN       argc,
   jsval *     argv,
   jsval *     rval)
{
	  int         rc;
    void* arg0; /* In */
    int result;

    JS_ValueToECMAInt32(cx,argv[0],&arg0);
    result = sqlite3_step(
        arg0
);

*rval = INT_TO_JSVAL(result);
    return JS_TRUE;
}
static JSBool x_column_text(
	  JSContext * cx,
   JSObject *  obj,
   uintN       argc,
   jsval *     argv,
   jsval *     rval)
{
	  int         rc;
    void* arg0; /* In */
    int arg1; /* In */
    char* result;

    JS_ValueToECMAInt32(cx,argv[0],&arg0);
    JS_ValueToECMAInt32(cx,argv[1],&arg1);
    result = sqlite3_column_text(
        arg0
       ,arg1
);

*rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, result));
    return JS_TRUE;
}
static JSBool x_finalize(
	  JSContext * cx,
   JSObject *  obj,
   uintN       argc,
   jsval *     argv,
   jsval *     rval)
{
	  int         rc;
    void* arg0; /* In */
    int result;

    JS_ValueToECMAInt32(cx,argv[0],&arg0);
    result = sqlite3_finalize(
        arg0
);

*rval = INT_TO_JSVAL(result);
    return JS_TRUE;
}
static JSBool x_bind_text(
	  JSContext * cx,
   JSObject *  obj,
   uintN       argc,
   jsval *     argv,
   jsval *     rval)
{
	  int         rc;
    void* arg0; /* In */
    int arg1; /* In */
    char* arg2; /* In */
    int arg3; /* Const */
    void* arg4; /* Const */
    int result;

    JS_ValueToECMAInt32(cx,argv[0],&arg0);
    JS_ValueToECMAInt32(cx,argv[1],&arg1);
    arg2 = JSVAL_IS_NULL(argv[2]) ? NULL : JS_GetStringBytes(JSVAL_TO_STRING(argv[2]));
    arg3 = -1;
    arg4 = SQLITE_TRANSIENT;
    result = sqlite3_bind_text(
        arg0
       ,arg1
       ,arg2
       ,arg3
       ,arg4
);

*rval = INT_TO_JSVAL(result);
    return JS_TRUE;
}
static JSBool x_errmsg(
	  JSContext * cx,
   JSObject *  obj,
   uintN       argc,
   jsval *     argv,
   jsval *     rval)
{
	  int         rc;
    void* arg0; /* In */
    char* result;

    JS_ValueToECMAInt32(cx,argv[0],&arg0);
    result = sqlite3_errmsg(
        arg0
);

*rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, result));
    return JS_TRUE;
}
static JSBool x_errcode(
	  JSContext * cx,
   JSObject *  obj,
   uintN       argc,
   jsval *     argv,
   jsval *     rval)
{
	  int         rc;
    void* arg0; /* In */
    int result;

    JS_ValueToECMAInt32(cx,argv[0],&arg0);
    result = sqlite3_errcode(
        arg0
);

*rval = INT_TO_JSVAL(result);
    return JS_TRUE;
}
static JSClass class = {
    "SQLITE", JSCLASS_NEW_RESOLVE,
    JS_PropertyStub,  JS_PropertyStub,  JS_PropertyStub,  JS_PropertyStub,
    JS_EnumerateStub,  (JSResolveOp)JS_ResolveStub,
    JS_ConvertStub,      JS_FinalizeStub
};

static JSFunctionSpec methods[] = {
{"sqlite3_open", x_open,     1},
{"sqlite3_close", x_close,     1},
{"sqlite3_prepare", x_prepare,     1},
{"sqlite3_column_count", x_column_count,     1},
{"sqlite3_column_name", x_column_name,     1},
{"sqlite3_step", x_step,     1},
{"sqlite3_column_text", x_column_text,     1},
{"sqlite3_finalize", x_finalize,     1},
{"sqlite3_bind_text", x_bind_text,     1},
{"sqlite3_errmsg", x_errmsg,     1},
{"sqlite3_errcode", x_errcode,     1},

{0}
};

INT sqlite_initialize ( JSContext* cx, JSObject* glob)
{
    JSBool          status      = JS_TRUE;
    JSObject*       obj;

    obj = JS_DefineObject( cx, glob, "SQLITE", &class, NULL, 0 );
    if( !obj ) status = JS_FALSE;
//  if( status && !JS_DefineProperties(cx, obj, its_props);
    if( status && !JS_DefineFunctions(cx, obj, methods) )
        status = JS_FALSE;
    return status;
}
