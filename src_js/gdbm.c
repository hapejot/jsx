#include "platform.h"
SCCSID(jsw, "$Id: //depot/peter/w/js/gen.js#1 $");
#include "jsw_int.h"
static JSBool gdbm_gdbm_open(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    int         rc;
    char* arg0;
    int arg1;
    int arg2;
    void* arg3;
    void* result;
    arg0 = JSVAL_IS_NULL(argv[0]) ? NULL : JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
    JS_ValueToECMAInt32(cx,argv[1],&arg1);
    JS_ValueToECMAInt32(cx,argv[2],&arg2);
    JS_ValueToECMAUint32(cx,argv[3],&arg3);
    result = gdbm_open(arg0, arg1, arg2, arg3);
    *rval = INT_TO_JSVAL(result);
    return JS_TRUE;
}
static JSBool gdbm_gdbm_close(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    int         rc;
    void* arg0;
    JS_ValueToECMAUint32(cx,argv[0],&arg0);
    gdbm_close(arg0);
    return JS_TRUE;
}
static JSBool gdbm_gdbm_store(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    int         rc;
    void* arg0;
