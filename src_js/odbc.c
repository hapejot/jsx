#include "platform.h"
SCCSID(jsw, "$Id: //depot/peter/w/js/gen.js#1 $");
#include "jsw_int.h"
static JSBool odbc_gdbm_open(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    int         rc;
