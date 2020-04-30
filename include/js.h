#ifndef JS_H
#define JS_H

#include "jsstddef.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "jstypes.h"
#include "jsarena.h"
#include "jsutil.h"
#include "jsprf.h"
#include "jsapi.h"
#include "jsatom.h"
#include "jscntxt.h"
#include "jsdbgapi.h"
#include "jsemit.h"
#include "jsfun.h"
#include "jsgc.h"
#include "jslock.h"
#include "jsobj.h"
#include "jsparse.h"
#include "jsscope.h"
#include "jsscript.h"

typedef enum JSShellErrNum {
#define MSG_DEF(name, number, count, exception, format) \
    name = number,
#include "jsshell.msg"
#undef MSG_DEF
    JSShellErr_Limit
} JSShellErrNum;

extern FILE*        gOutFile;

extern JSBool jshell_print(JSContext*, JSObject *, uintN, jsval*, jsval*);
extern JSBool jshell_system(JSContext*, JSObject *, uintN, jsval*, jsval*);
extern void jshell_process(JSContext *cx, JSObject *obj, char *filename);
extern void jshell_error_reporter( JSContext *     cx, const char *    message, JSErrorReport * report);
extern JSBool js_net_initialize(JSContext*, JSObject*);
#endif
