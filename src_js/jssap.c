#include "js.h"
#undef SCCSID
#include "saprfc.h"
#include "sapitab.h"

/*
 * Implementierung von drei Objektklassen
 * 1. SapTable
 * 2. SapConnection
 *      open
 *      close
 * 3. SapRequest
 *      addTable
 *      addParam
 */

enum sap_tinyid {
    SAP_RDONLY, 
    SAP_CURSOR, 
    SAP_COLS, 
    SAP_LENG, 
    SAP_FILL,
    SAP_CONNECT_STRING,
    SAP_STATUS
};

/******************************************************************
 * SapConnection
 *
 * Methoden
 *      Name        Implementierung
 *      =========== ===================
 *      open        sap_con_open
 *      close       sap_con_close
 *      =========== ===================
 *
 * Properties
 *
 *  sap_get_con_prop
 *      Name
 *      =================== ===================
 *      connectString       
 *      status
 *
 *
 */
static JSClass sap_con_class;

struct sap_con {
    char *              con_str;
    RFC_HANDLE          con;
    RFC_ERROR_INFO_EX   err;
};

static JSPropertySpec sap_connection_props[] = {
    {"connectString",       SAP_CONNECT_STRING,     JSPROP_ENUMERATE},
    {"status",              SAP_STATUS,             JSPROP_ENUMERATE},
    {0}
};



static JSBool
sap_get_con_prop(
    JSContext * cx, 
    JSObject *  obj, 
    jsval       id, 
    jsval *     vp)
{
    struct sap_con* private;
    jsint i = JSVAL_TO_INT(id);

    private = JS_GetInstancePrivate(cx, obj, &sap_con_class, NULL);

    switch( i )
    {
    case SAP_STATUS:
        *vp = STRING_TO_JSVAL(JS_NewStringCopyN(cx, &private->err, sizeof(private->err)));
        break;
    case SAP_CONNECT_STRING:
        *vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, private->con_str));
        break;
    }
    // fprintf( stderr, "get con property %s\n", JS_GetStringBytes(JS_ValueToString(cx, id)) );
    return JS_TRUE;
}

static void
sap_con_finalize(JSContext *cx, JSObject *obj)
{
    struct sap_con* private;

    private = JS_GetInstancePrivate(cx, obj, &sap_con_class, NULL);
    if( private )
    {
        if( private->con )
            RfcClose( private->con );
        JS_free( cx, private );
    }
}


static JSClass sap_con_class = {
    "SapConnection", JSCLASS_HAS_PRIVATE | JSCLASS_NEW_RESOLVE,
    JS_PropertyStub,  JS_PropertyStub,  sap_get_con_prop,  JS_PropertyStub,
    JS_EnumerateStub,  (JSResolveOp)JS_ResolveStub,
    JS_ConvertStub,      sap_con_finalize
};

/* constructor
 * 
 * arguments
 *      1. connection parameters
 *
 * action
 *      allocate memory for error info and connection handle.
 */

static JSBool       sap_con_constructor 
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result)
{
    struct sap_con *    private;

    private = malloc(sizeof(*private));
    private->con_str = JS_strdup(p_cx, JS_GetStringBytes(JS_ValueToString(p_cx, p_argv[0])));
    private->con = 0;
    JS_SetPrivate( p_cx, p_obj, private );
    return JS_TRUE;
}


static JSBool
sap_con_open(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    struct sap_con* private;

    private = JS_GetInstancePrivate(cx, obj, &sap_con_class, NULL);
    private->con = RfcOpenEx( private->con_str, &private->err );
    *rval = INT_TO_JSVAL( private->con != 0 );
    return JS_TRUE;
}

static JSBool
sap_con_close(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    struct sap_con* private;

    private = JS_GetInstancePrivate(cx, obj, &sap_con_class, NULL);
    RfcClose( private->con );
    private->con = 0;
    return JS_TRUE;
}

static JSFunctionSpec sap_connection_methods[] = {
    {"open",             sap_con_open,      0},
    {"close",           sap_con_close,      0},
    {0}
};

/******************************************************************
 * SapRequest
 *
 * Methoden
 *      Name        Implementierung
 *      =========== ===================
 *      export      sap_req_export
 *      import      sap_req_import
 *      table       sap_req_table
 *      =========== ===================
 *
 * Properties
 *
 *  sap_get_req_prop
 *      Name
 *      =================== ===================
 *
 *
 */
static JSClass sap_req_class;

struct sap_req {
    void *      import;
    void *      export;
    void *      table;
};

static JSPropertySpec sap_req_props[] = {
    {0}
};



static JSBool
sap_get_req_prop(
    JSContext * cx, 
    JSObject *  obj, 
    jsval       id, 
    jsval *     vp)
{
    struct sap_req* private;
    jsint i = JSVAL_TO_INT(id);

    private = JS_GetInstancePrivate(cx, obj, &sap_con_class, NULL);

//  switch( i )
//  {
//  case SAP_CONNECT_STRING:
//      *vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, private->con_str));
//      break;
//  }
//  fprintf( stderr, "get con property %s\n", JS_GetStringBytes(JS_ValueToString(cx, id)) );
    return JS_TRUE;
}

static void
sap_req_finalize(JSContext *cx, JSObject *obj)
{
    struct sap_req* private;

    private = JS_GetInstancePrivate(cx, obj, &sap_req_class, NULL);
    if( private )
    {
    }
}


static JSClass sap_req_class = {
    "SapRequest", JSCLASS_HAS_PRIVATE | JSCLASS_NEW_RESOLVE,
    JS_PropertyStub,  JS_PropertyStub,  sap_get_req_prop,  JS_PropertyStub,
    JS_EnumerateStub,  (JSResolveOp)JS_ResolveStub,
    JS_ConvertStub,      sap_req_finalize
};

/* constructor
 * 
 * arguments
 *      1. connection parameters
 *
 * action
 *      allocate memory for error info and connection handle.
 */

static JSBool       sap_req_constructor 
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result)
{
    struct sap_req *    private;

    private = malloc(sizeof(*private));
    JS_SetPrivate( p_cx, p_obj, private );
    return JS_TRUE;
}


static JSBool
sap_req_import(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    struct sap_req* private;

    private = JS_GetInstancePrivate(cx, obj, &sap_req_class, NULL);
    return JS_TRUE;
}

static JSFunctionSpec sap_req_methods[] = {
    {"import",          sap_req_import,     0},
    {0}
};

/***************************
 * Allgemeines
 */

static JSBool
sap_addProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    return JS_TRUE;
}

static JSBool
sap_delProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    return JS_TRUE;
}

static JSBool
sap_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    return JS_TRUE;
}

static JSBool
sap_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    return JS_TRUE;
}

static JSBool
sap_enumerate(JSContext *cx, JSObject *obj)
{
    return JS_TRUE;
}

static JSBool
sap_resolve(JSContext *cx, JSObject *obj, jsval id, uintN flags,
            JSObject **objp)
{
/*
    fprintf(gOutFile, "resolving sap property %s, flags {%s,%s,%s}\n",
               JS_GetStringBytes(JS_ValueToString(cx, id)),
               (flags & JSRESOLVE_QUALIFIED) ? "qualified" : "",
               (flags & JSRESOLVE_ASSIGNING) ? "assigning" : "",
               (flags & JSRESOLVE_DETECTING) ? "detecting" : "");
*/
    return JS_TRUE;
}

static JSBool
sap_convert(JSContext *cx, JSObject *obj, JSType type, jsval *vp)
{
    fprintf(gOutFile, "converting SAP to %s type\n", JS_GetTypeName(cx, type));
    return JS_TRUE;
}

static void
sap_finalize(JSContext *cx, JSObject *obj)
{
    fprintf(gOutFile, "finalizing sap %p\n", obj);
}


static JSBool       sap_constructor 
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result)
{
    return JS_TRUE;
}

/********************************************************
 * SAP TABLE
 *
 * Properties
 *      sap_get_table_prop
 *
 */

static JSClass sap_table_class;

static void
sap_table_finalize(JSContext *cx, JSObject *obj)
{
    ITAB_H      handle;
    if( obj != NULL )
    {
        handle = JS_GetInstancePrivate(cx, obj, &sap_table_class, NULL);
        ItDelete( handle );
    }
}

static void
sap_table_free(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    ITAB_H      handle;
    handle = JS_GetInstancePrivate(cx, obj, &sap_table_class, NULL);
    ItFree( handle );
}

static void
sap_table_app_line(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    if( argc == 1 )
    {
        ITAB_H      handle;
        void *      t;
        char *      s;
        int         n;
        JSString * str;

        handle = JS_GetInstancePrivate(cx, obj, &sap_table_class, NULL);
        t = ItAppLine( handle );
        str = JS_ValueToString(cx, argv[0]);
        s = JS_GetStringBytes(str);
        n = JS_GetStringLength(str);
        memcpy( t, s, n );
    }
}

static void
sap_table_get_line(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    if( argc == 1 )
    {
        int     line_no;
        ITAB_H      handle;
        void *      t;
        char *      s;

        JS_ValueToInt32( cx, argv[0], &line_no );
        handle = JS_GetInstancePrivate(cx, obj, &sap_table_class, NULL);
        s = ItGetLine( handle, line_no );
        if( s )
            *rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, s));
    }
}


static JSBool
sap_get_table_prop(
        JSContext *cx, 
        JSObject *obj, 
        jsval id, 
        jsval *vp)
{
    ITAB_H  handle;
    jsint i = JSVAL_TO_INT(id);

    handle = JS_GetInstancePrivate(cx, obj, &sap_table_class, NULL);

    switch( i )
    {
    case SAP_LENG:
        *vp = INT_TO_JSVAL( ItLeng(handle) );
        break;
    case SAP_FILL:
        *vp = INT_TO_JSVAL( ItFill(handle) );
        break;
    }
    // fprintf( stderr, "get table property %s\n", JS_GetStringBytes(JS_ValueToString(cx, id)) );
    return JS_TRUE;
}



static JSClass sap_table_class = {
    "SapTable", JSCLASS_HAS_PRIVATE | JSCLASS_NEW_RESOLVE,
    sap_addProperty,  sap_delProperty,  sap_get_table_prop,  sap_setProperty,
    sap_enumerate,    (JSResolveOp)sap_resolve,
    sap_convert,      sap_table_finalize
};

static JSPropertySpec sap_table_props[] = {
    {"cursor",          SAP_CURSOR,     JSPROP_ENUMERATE},
    {"cols",            SAP_COLS,       JSPROP_ENUMERATE},
    {"leng",            SAP_LENG,       JSPROP_ENUMERATE},
    {"fill",            SAP_FILL,       JSPROP_ENUMERATE},
    {0}
};


static JSFunctionSpec sap_table_methods[] = {
    {"free",            sap_table_free,         0},
    {"app_line",        sap_table_app_line,     1},
    {"get_line",        sap_table_get_line,     1},
    {0}
};


static JSBool       sap_table_constructor 
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result)
{
    int     row_size    = 1000;
    ITAB_H  handle      = 0;

    if( p_argc > 0 )
        JS_ValueToInt32( p_cx, p_argv[0], &row_size );
    handle = ItCreate( "JsTable", row_size, 0, 0 );
    JS_SetPrivate( p_cx, p_obj, handle );
    return JS_TRUE;
}

/*************************************************************
 * global initialization
 */

JSBool
js_sap_init_class(
    JSContext*      cx,
    JSObject*       glob)
{
    JSObject*       it;
    
    it = JS_InitClass( cx, glob, NULL,
            &sap_con_class,
            sap_con_constructor, 0,
            sap_connection_props,
            sap_connection_methods,
            NULL,
            NULL );

    it = JS_InitClass( cx, glob, NULL,
            &sap_table_class,
            sap_table_constructor, 0,
            sap_table_props,
            sap_table_methods,
            NULL,
            NULL );

    it = JS_InitClass( cx, glob, NULL,
            &sap_req_class,
            sap_req_constructor, 0,
            sap_req_props,
            sap_req_methods,
            NULL,
            NULL );

    return JS_TRUE;
}
/**************************************************************************************************
 *
 * RFC Object
 * -----------
 *  one to one rfc sdk function interface.
 *  strucutre conversion routines
 */

static void
set_RFC_ATTRIBUTES_fields(
        JSContext *         cx, 
        JSObject *          obj, 
        RFC_ATTRIBUTES *    fields
        )
{
}

static void
get_RFC_ATTRIBUTES_fields(
        JSContext *         cx, 
        JSObject *          obj, 
        RFC_ATTRIBUTES *    fields
        )
{
    jsval   v;

    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->dest)); JS_SetProperty( cx, obj, "dest", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->own_host)); JS_SetProperty( cx, obj, "own_host", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->partner_host)); JS_SetProperty( cx, obj, "partner_host", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->systnr)); JS_SetProperty( cx, obj, "systnr", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->sysid)); JS_SetProperty( cx, obj, "sysid", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->client)); JS_SetProperty( cx, obj, "client", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->user)); JS_SetProperty( cx, obj, "user", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->language)); JS_SetProperty( cx, obj, "language", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyN(cx,&fields->trace, 1)); JS_SetProperty( cx, obj, "trace", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->ISO_language)); JS_SetProperty( cx, obj, "ISO_language", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->own_codepage)); JS_SetProperty( cx, obj, "own_codepage", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->partner_codepage)); JS_SetProperty( cx, obj, "partner_codepage", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyN(cx,&fields->rfc_role, 1)); JS_SetProperty( cx, obj, "rfc_role", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyN(cx,&fields->own_type, 1)); JS_SetProperty( cx, obj, "own_type", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->own_rel)); JS_SetProperty( cx, obj, "own_rel", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyN(cx,&fields->partner_type, 1)); JS_SetProperty( cx, obj, "partner_type", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->partner_rel)); JS_SetProperty( cx, obj, "partner_rel", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->kernel_rel)); JS_SetProperty( cx, obj, "kernel_rel", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->CPIC_convid)); JS_SetProperty( cx, obj, "CPIC_convid", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyN(cx,&fields->password_sate, 1)); JS_SetProperty( cx, obj, "password_sate", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->own_codepage_pcs)); JS_SetProperty( cx, obj, "own_codepage_pcs", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->pcs)); JS_SetProperty( cx, obj, "pcs", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->real_partner_codepage)); JS_SetProperty( cx, obj, "real_partner_codepage", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->progname)); JS_SetProperty( cx, obj, "progname", &v );
}
static void
get_RFC_ERROR_INFO_EX_fields(
        JSContext *             cx, 
        JSObject *              obj, 
        RFC_ERROR_INFO_EX *     fields
        )
{
    jsval   v;

    v = STRING_TO_JSVAL(JS_NewStringCopyN(cx, &fields->group, 1)); JS_SetProperty( cx, obj, "group", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, &fields->key)); JS_SetProperty( cx, obj, "key", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, &fields->message)); JS_SetProperty( cx, obj, "message", &v );
}

static void
set_RFC_PARAMETER_fields(
        JSContext *             cx, 
        JSObject *              obj, 
        RFC_PARAMETER *         fields
        )
{
    jsval   v;
    int     rc;

    rc = JS_GetProperty(cx,obj,"name",&v); fields->name = JS_GetStringBytes(JSVAL_TO_STRING(v));
    fields->nlen = strlen(fields->name);
    rc = JS_GetProperty(cx,obj,"type",&v); fields->type = JSVAL_TO_INT(v);

    rc = JS_GetProperty(cx,obj,"leng",&v);

    if( rc && JSVAL_IS_INT(v) )
    { 
        fields->leng = JSVAL_TO_INT(v);
        fields->addr = malloc(fields->leng);
        memset( fields->addr, ' ', fields->leng );
    }
    else
    {
        rc = JS_GetProperty(cx,obj,"addr",&v);
        if( JSVAL_IS_STRING(v) )
        {
            fields->addr = JS_GetStringBytes(JSVAL_TO_STRING(v));
            fields->leng = strlen(fields->addr);
        }
    }
}

static void
get_RFC_PARAMETER_fields(
        JSContext *             cx, 
        JSObject *              obj, 
        RFC_PARAMETER *         fields
        )
{
    jsval   v;

    v = STRING_TO_JSVAL(JS_NewStringCopyN(cx, fields->name, fields->nlen)); JS_SetProperty( cx, obj, "name", &v );
    v = INT_TO_JSVAL(fields->nlen); JS_SetProperty( cx, obj, "nlen", &v );
    v = INT_TO_JSVAL(fields->type); JS_SetProperty( cx, obj, "type", &v );
    v = INT_TO_JSVAL(fields->leng); JS_SetProperty( cx, obj, "leng", &v );
    v = STRING_TO_JSVAL(JS_NewStringCopyN(cx, fields->addr, fields->leng)); JS_SetProperty( cx, obj, "addr", &v );
}

static void
set_RFC_TABLE_fields(
        JSContext *             cx, 
        JSObject *              obj, 
        RFC_TABLE *             fields
        )
{
    jsval   v;

    JS_GetProperty(cx,obj,"name",&v); fields->name = JS_GetStringBytes(JSVAL_TO_STRING(v));
    fields->nlen = strlen(fields->name);
    JS_GetProperty(cx,obj,"type",&v); fields->type = JSVAL_TO_INT(v);
    JS_GetProperty(cx,obj,"ithandle",&v); fields->ithandle = JSVAL_TO_INT(v);
    // JS_GetProperty(cx,obj,"itmode",&v); fields->itmode = JSVAL_TO_INT(v);
}

static void
get_RFC_TABLE_fields(
        JSContext *             cx, 
        JSObject *              obj, 
        RFC_TABLE *             fields
        )
{
    jsval   v;

    v = STRING_TO_JSVAL(JS_NewStringCopyN(cx, fields->name, fields->nlen)); JS_SetProperty( cx, obj, "name", &v );
    v = INT_TO_JSVAL(fields->nlen); JS_SetProperty( cx, obj, "nlen", &v );
    v = INT_TO_JSVAL(fields->type); JS_SetProperty( cx, obj, "type", &v );
    v = INT_TO_JSVAL(fields->ithandle); JS_SetProperty( cx, obj, "ithandle", &v );
    // v = INT_TO_JSVAL(fields->itmode); JS_SetProperty( cx, obj, "itmode", &v );
}

static void
set_RFC_TYPE_ELEMENT_fields(
        JSContext *             cx, 
        JSObject *              obj, 
        RFC_TYPE_ELEMENT *             fields
        )
{
    jsval   v;

    JS_GetProperty(cx,obj,"name",&v);     fields->name = JS_GetStringBytes(JSVAL_TO_STRING(v));
    JS_GetProperty(cx,obj,"type",&v);     fields->type = JSVAL_TO_INT(v);
    JS_GetProperty(cx,obj,"length",&v);   fields->length = JSVAL_TO_INT(v);
    JS_GetProperty(cx,obj,"decimals",&v); fields->length = JSVAL_TO_INT(v);
}

static void
get_RFC_TYPE_ELEMENT_fields(
        JSContext *             cx, 
        JSObject *              obj, 
        RFC_TYPE_ELEMENT *             fields
        )
{
    jsval   v;

    v = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, fields->name)); JS_SetProperty( cx, obj, "name", &v );
    v = INT_TO_JSVAL(fields->type); JS_SetProperty( cx, obj, "type", &v );
    v = INT_TO_JSVAL(fields->length); JS_SetProperty( cx, obj, "length", &v );
    v = INT_TO_JSVAL(fields->decimals); JS_SetProperty( cx, obj, "decimals", &v );
}


static JSBool
sap_rfc_call_ex(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    RFC_PARAMETER       exporting[32];
    RFC_PARAMETER       changing[32];
    RFC_TABLE           tables[32];
    RFC_HANDLE          h;
    rfc_char_t*         name;
    int                 n;
    int                 i;
    jsval               v;


    JS_ASSERT( argc == 5 );

    JS_ValueToInt32(cx, argv[0], &h);
    name = JS_GetStringBytes(JS_ValueToString(cx, argv[1]));

    JS_GetArrayLength(cx, JSVAL_TO_OBJECT(argv[2]), &n);
    for( i = 0; i < n; i++ )
    {
        JS_GetElement( cx, JSVAL_TO_OBJECT(argv[2]), i, &v );
        set_RFC_PARAMETER_fields( cx, JSVAL_TO_OBJECT(v), &exporting[i] );
    }
    exporting[i].name = NULL;
  
    changing[0].name  = NULL;
    
    JS_GetArrayLength(cx, JSVAL_TO_OBJECT(argv[4]), &n);
    for( i = 0; i < n; i++ )
    {
        JS_GetElement( cx, JSVAL_TO_OBJECT(argv[4]), i, &v );
        set_RFC_TABLE_fields( cx, JSVAL_TO_OBJECT(v), &tables[i] );
    }
    tables[i].name    = NULL;



    *rval = INT_TO_JSVAL( RfcCallEx(h, name, exporting, changing, tables) );
    return JS_TRUE;
}

static JSBool
sap_rfc_install_structure(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    int                 rc;
    rfc_char_t *        name;
    RFC_TYPE_ELEMENT *  elements;
    unsigned            entries;
    RFC_TYPEHANDLE      handle;

    rc = RfcInstallStructure(name, elements, entries, &handle);

    *rval = INT_TO_JSVAL( rc );
    return JS_TRUE;
}


static JSBool
sap_rfc_receive_ex(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    RFC_PARAMETER       importing[32];
    RFC_PARAMETER       changing[32];
    RFC_TABLE           tables[32];
    RFC_HANDLE          h;
    rfc_char_t          except       = NULL;
    int                 n;
    int                 i;
    jsval               v;

    JS_ASSERT( argc == 5 );
  
    changing[0].name  = NULL;

    JS_ValueToInt32(cx, argv[0], &h);
    JS_GetArrayLength(cx, JSVAL_TO_OBJECT(argv[1]), &n);
    for( i = 0; i < n; i++ )
    {
        JS_GetElement( cx, JSVAL_TO_OBJECT(argv[1]), i, &v );
        set_RFC_PARAMETER_fields( cx, JSVAL_TO_OBJECT(v), &importing[i] );
    }
    importing[i].name = NULL;

    JS_GetArrayLength(cx, JSVAL_TO_OBJECT(argv[3]), &n);
    for( i = 0; i < n; i++ )
    {
        JS_GetElement( cx, JSVAL_TO_OBJECT(argv[3]), i, &v );
        set_RFC_TABLE_fields( cx, JSVAL_TO_OBJECT(v), &tables[i] );
    }
    tables[i].name    = NULL;

    *rval = INT_TO_JSVAL( RfcReceiveEx(h,importing, changing, tables, &except) );

    JS_GetArrayLength(cx, JSVAL_TO_OBJECT(argv[1]), &n);
    for( i = 0; i < n; i++ )
    {
        JS_GetElement( cx, JSVAL_TO_OBJECT(argv[1]), i, &v );
        get_RFC_PARAMETER_fields( cx, JSVAL_TO_OBJECT(v), &importing[i] );
    }
    return JS_TRUE;
}


static JSBool
sap_rfc_open_ex(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    RFC_ERROR_INFO_EX   err;
    *rval = INT_TO_JSVAL(RfcOpenEx( 
                    JS_GetStringBytes(JS_ValueToString(cx, argv[0])),
                    &err ));
    get_RFC_ERROR_INFO_EX_fields(cx, JSVAL_TO_OBJECT(argv[1]), &err);
    return JS_TRUE;
}

static JSBool
sap_rfc_close(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    RFC_HANDLE  h;
    JS_ValueToInt32(cx, argv[0], &h);
    RfcClose( h );
    return JS_TRUE;
}

static JSBool
sap_it_create(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    ITAB_H  handle;
    char *  name;
    int     width;

    name = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
    width = JSVAL_TO_INT(argv[1]);
    handle = ItCreate(name, width, 0, 0);
    *rval = INT_TO_JSVAL(handle);
    return JS_TRUE;
}

static JSBool
sap_it_app_line(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    ITAB_H      handle;
    void *      t;
    char *      s;
    int         n;
    JSString * str;

    handle = JSVAL_TO_INT(argv[0]);
    t = ItAppLine( handle );
    str = JS_ValueToString(cx, argv[1]);
    s = JS_GetStringBytes(str);
    n = JS_GetStringLength(str);
    memcpy( t, s, n );
    return JS_TRUE;
}

static JSBool
sap_it_get_line(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    int     handle;
    int     n;
    void *  s;

    handle = JSVAL_TO_INT(argv[0]);
    n = JSVAL_TO_INT(argv[1]);
    s = ItGetLine( handle, n );
    if( s )
        *rval = STRING_TO_JSVAL(JS_NewStringCopyN(cx, s, ItLeng(handle)));
    return JS_TRUE;
}


static JSBool
sap_it_fill(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    ITAB_H      handle;
    int         rc;

    handle = JSVAL_TO_INT(argv[0]);
    rc = ItFill( handle );
    *rval = INT_TO_JSVAL( rc );
    return JS_TRUE;
}


static JSBool
sap_rfc_get_attributes(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    if( argc == 2 )
    {
        RFC_HANDLE  h;
        RFC_ATTRIBUTES   att;
        JSObject *  o   = JSVAL_TO_OBJECT(argv[1]);

        JS_ValueToInt32(cx, argv[0], &h);
        *rval = INT_TO_JSVAL( RfcGetAttributes(h, &att) );
        get_RFC_ATTRIBUTES_fields(cx, o, &att);
    }
    return JS_TRUE;
}


static JSClass sap_class = {
    "SAP", JSCLASS_NEW_RESOLVE,
    JS_PropertyStub,  JS_PropertyStub,  JS_PropertyStub,  JS_PropertyStub,
    JS_EnumerateStub,  (JSResolveOp)JS_ResolveStub,
    JS_ConvertStub,      sap_con_finalize
};

static JSFunctionSpec sap_methods[] = {
    {"RfcOpenEx",           sap_rfc_open_ex,        1},
    {"RfcClose",            sap_rfc_close,          1},
    {"RfcGetAttributes",    sap_rfc_get_attributes, 2},
    {"RfcCallEx",           sap_rfc_call_ex,        5},
    {"RfcReceiveEx",        sap_rfc_receive_ex,     5},
    {"RfcInstallStructure", sap_rfc_install_structure, 4},
    {"ItCreate",            sap_it_create,          2},
    {"ItAppLine",           sap_it_app_line,        1},
    {"ItGetLine",           sap_it_get_line,        2},
    {"ItFill",              sap_it_fill,            1},
    {0}
};


JSBool
js_sap_initialize(
    JSContext*      cx,
    JSObject*       glob)
{
    JSBool          status      = JS_TRUE;
    JSObject*       sap;

    sap = JS_DefineObject( cx, glob, "SAP", &sap_class, NULL, 0 );
    if( !sap ) status = JS_FALSE;
//  if( status && !JS_DefineProperties(cx, sap, its_props);
    if( status && !JS_DefineFunctions(cx, sap, sap_methods) )
        status = JS_FALSE;
    return status;
}
