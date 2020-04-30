#include "js.h"
#undef SCCSID
#include <windows.h>
#include <sql.h>
#include <sqlext.h>

/**************************************************************************************************
 *
 * ODBC Object
 * -----------
 */

/*
 *
static void
set_XXX_fields(
        JSContext *         cx, 
        JSObject *          obj, 
        XXX *               fields
        )
{
    JS_GetProperty(cx,obj,"name",&v); fields->name = JS_GetStringBytes(JSVAL_TO_STRING(v));
    JS_GetProperty(cx,obj,"type",&v); fields->type = JSVAL_TO_INT(v);
}

static void
get_XXX_fields(
        JSContext *         cx, 
        JSObject *          obj, 
        XXX *               fields
        )
{
    v = STRING_TO_JSVAL(JS_NewStringCopyN(cx, fields->name, fields->nlen)); JS_SetProperty( cx, obj, "name", &v );
    v = INT_TO_JSVAL(fields->nlen); JS_SetProperty( cx, obj, "nlen", &v );
}


static JSBool
func(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    JS_ASSERT( argc == 5 );

    JS_ValueToInt32(cx, argv[0], &h);
    name = JS_GetStringBytes(JS_ValueToString(cx, argv[1]));

    JS_GetArrayLength(cx, JSVAL_TO_OBJECT(argv[2]), &n);
    for( i = 0; i < n; i++ )
    {
        JS_GetElement( cx, JSVAL_TO_OBJECT(argv[2]), i, &v );
        set_RFC_PARAMETER_fields( cx, JSVAL_TO_OBJECT(v), &exporting[i] );
    }

    *rval = INT_TO_JSVAL( Call(h, name, exporting, changing, tables) );
    return JS_TRUE;
}

*/

static JSBool
odbc_sql_alloc_env(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    HENV    h;
    SQLAllocEnv(&h);
    *rval = INT_TO_JSVAL( h );
    return JS_TRUE;
}


static JSBool
odbc_sql_alloc_connect(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    HENV    env;
    HDBC    h;

    JS_ASSERT(argc);

    env = (HENV)JSVAL_TO_INT( argv[0] );
    SQLAllocConnect(env, &h);
    *rval = INT_TO_JSVAL( h );
    return JS_TRUE;
}

static JSBool
odbc_sql_driver_connect(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    HDBC    h;
    char *  name;
    int     rc;

    JS_ASSERT(argc == 2);

    h = (HENV)JSVAL_TO_INT( argv[0] );
    name = JS_GetStringBytes(JS_ValueToString(cx, argv[1]));
    rc = SQLDriverConnect(h,0, name,SQL_NTS,NULL,0,NULL,SQL_DRIVER_NOPROMPT);

    if( rc == SQL_SUCCESS )
        rc = SQLSetConnectAttr( h, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF, SQL_IS_UINTEGER );

    *rval = INT_TO_JSVAL( rc );
    return JS_TRUE;
}


static JSBool
odbc_sql_alloc_stmt(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    HDBC    h;
    HSTMT   sh;
    int     rc;

    JS_ASSERT(argc == 1);

    h = (HENV)JSVAL_TO_INT( argv[0] );
    rc = SQLAllocStmt(h,&sh);

    *rval = INT_TO_JSVAL( sh );
    return JS_TRUE;
}


static JSBool
odbc_sql_tables(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    SQLHSTMT        StatementHandle;
    SQLCHAR *       CatalogName;
    SQLSMALLINT     NameLength1;
    SQLCHAR *       SchemaName;
    SQLSMALLINT     NameLength2;
    SQLCHAR *       TableName;
    SQLSMALLINT     NameLength3;
    SQLCHAR *       TableType;
    SQLSMALLINT     NameLength4;
    int     rc;

    StatementHandle = JSVAL_TO_INT( argv[0] );
    CatalogName = argv[1] ? JS_GetStringBytes(JS_ValueToString(cx, argv[1])) : NULL;
    SchemaName  = argv[2] ? JS_GetStringBytes(JS_ValueToString(cx, argv[2])) : NULL;
    TableName = argv[3] ? JS_GetStringBytes(JS_ValueToString(cx, argv[3])) : NULL;
    TableType = argv[4] ? JS_GetStringBytes(JS_ValueToString(cx, argv[4])) : NULL;
    rc = SQLTables( StatementHandle, 
                        CatalogName, CatalogName ? strlen(CatalogName) : 0, 
                        SchemaName, SchemaName ? strlen(SchemaName) : 0,
                        TableName, TableName ? strlen(TableName) : 0,
                        TableType, TableType ? strlen(TableType) : 0 );
    *rval = INT_TO_JSVAL( rc );
    return JS_TRUE;
}


static JSBool
odbc_sql_columns(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    SQLHSTMT        StatementHandle;
    SQLCHAR *       CatalogName;
    SQLSMALLINT     NameLength1;
    SQLCHAR *       SchemaName;
    SQLSMALLINT     NameLength2;
    SQLCHAR *       TableName;
    SQLSMALLINT     NameLength3;
    SQLCHAR *       ColumnName;
    SQLSMALLINT     NameLength4;
    int     rc;

    StatementHandle = JSVAL_TO_INT( argv[0] );
    CatalogName = argv[1] ? JS_GetStringBytes(JS_ValueToString(cx, argv[1])) : NULL;
    SchemaName  = argv[2] ? JS_GetStringBytes(JS_ValueToString(cx, argv[2])) : NULL;
    TableName = argv[3] ? JS_GetStringBytes(JS_ValueToString(cx, argv[3])) : NULL;
    ColumnName = argv[4] ? JS_GetStringBytes(JS_ValueToString(cx, argv[4])) : NULL;
    rc = SQLColumns( StatementHandle, 
                        CatalogName, CatalogName ? strlen(CatalogName) : 0, 
                        SchemaName, SchemaName ? strlen(SchemaName) : 0,
                        TableName, TableName ? strlen(TableName) : 0,
                        ColumnName, ColumnName ? strlen(ColumnName) : 0 );
    *rval = INT_TO_JSVAL( rc );
    return JS_TRUE;
}

static JSBool
odbc_sql_end_tran(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    SQLSMALLINT     htype;
    SQLHANDLE       h;
    SQLSMALLINT     completion_type;
    char *          s;
    int             rc = JS_TRUE;
    int             result;

    JS_ASSERT(argc == 3);

    s = JS_GetStringBytes(JS_ValueToString(cx, argv[0]));
    if( strcmp("SQL_HANDLE_ENV", s) == 0 ) htype = SQL_HANDLE_ENV;
    else if( strcmp("SQL_HANDLE_DBC", s) == 0 ) htype = SQL_HANDLE_DBC;
    else rc = JS_FALSE;
    h  = (HENV)JSVAL_TO_INT( argv[1] );
    s = JS_GetStringBytes(JS_ValueToString(cx, argv[2]));
    if( strcmp("SQL_COMMIT", s) == 0 ) completion_type = SQL_COMMIT;
    else if( strcmp("SQL_ROLLBACK", s) == 0 ) completion_type = SQL_ROLLBACK;
    else rc = JS_FALSE;
    if( rc == JS_TRUE )
    {
        result = SQLEndTran( htype, h, completion_type );
        *rval = INT_TO_JSVAL( result );
    }
    return rc;
}

static JSBool
odbc_sql_prepare(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    HSTMT   h;
    char *  cmd;
    int     rc;

    JS_ASSERT(argc == 2);

    h = (HENV)JSVAL_TO_INT( argv[0] );
    cmd = JS_GetStringBytes(JS_ValueToString(cx, argv[1]));
    rc = SQLPrepare( h, cmd, SQL_NTS );
    *rval = INT_TO_JSVAL( rc );
    return JS_TRUE;
}

static JSBool
odbc_sql_num_result_cols(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    HSTMT       h;
    SQLSMALLINT n;

    JS_ASSERT(argc == 1);

    h = (HENV)JSVAL_TO_INT( argv[0] );
    SQLNumResultCols( h, &n );
    *rval = INT_TO_JSVAL( n );
    return JS_TRUE;
}

static JSBool
odbc_sql_fetch(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    HSTMT       h;

    JS_ASSERT(argc == 1);

    h = (HENV)JSVAL_TO_INT( argv[0] );
    *rval = INT_TO_JSVAL(SQLFetch(h));
    return JS_TRUE;
}

static JSBool
odbc_sql_get_data(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    HSTMT       h;
    int         i;
    char      * data        = NULL;
    int         bufsize     = 1000;
    int         data_size   = bufsize + 1;
    SDWORD      len;
    int         rc;
    int         data_len    = 0;

    JS_ASSERT(argc == 2);

    h = (HENV)JSVAL_TO_INT( argv[0] );
    i = JSVAL_TO_INT( argv[1] );
    data = malloc(data_size);
    rc = SQLGetData( h, i, SQL_C_CHAR, data, bufsize+1, &len );
//  fprintf( stderr, "***len: %d %d\n", len, data_len );
    while( rc == SQL_SUCCESS_WITH_INFO )
    {
        data_len = data_size - 1;
        data_size += bufsize;
        data = realloc(data, data_size);
        rc = SQLGetData( h, i, SQL_C_CHAR, data + data_len, bufsize+1, &len );
//      fprintf( stderr, "****len: %d %d\n", len, data_len );
    }
    data_len += len;
    if( rc == SQL_SUCCESS && data_len >= 0 )
        *rval = STRING_TO_JSVAL( JS_NewStringCopyN(cx, data, data_len) );
    else
        *rval = JSVAL_NULL;
    free( data );
    return JS_TRUE;
}

static JSBool
odbc_sql_free_stmt(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    HSTMT       h;

    JS_ASSERT(argc == 1);

    h = (HENV)JSVAL_TO_INT( argv[0] );
    SQLFreeStmt(h, SQL_DROP);
    return JS_TRUE;
}

static JSBool
odbc_sql_execute(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    HSTMT       h;

    JS_ASSERT(argc == 1);

    h = (HENV)JSVAL_TO_INT( argv[0] );
    SQLExecute(h);
    return JS_TRUE;
}

  static JSBool
odbc_sql_exec_direct(
          JSContext *cx, 
          JSObject *obj, 
          uintN argc, 
          jsval *argv, 
          jsval *rval)
{
    HSTMT   h;
    char *  cmd;
    int     rc;
  
    JS_ASSERT(argc == 2);

    h = (HENV)JSVAL_TO_INT( argv[0] );
    cmd = JS_GetStringBytes(JS_ValueToString(cx, argv[1]));
    rc = SQLExecDirect( h, cmd, SQL_NTS );
    *rval = INT_TO_JSVAL( rc );
    return JS_TRUE;
}


static JSBool
odbc_sql_disconnect(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    HDBC        h;

    JS_ASSERT(argc == 1);

    h = (HENV)JSVAL_TO_INT( argv[0] );
    SQLDisconnect(h);
    return JS_TRUE;
}

static JSBool
odbc_sql_free_connect(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    HDBC        h;

    JS_ASSERT(argc == 1);

    h = (HENV)JSVAL_TO_INT( argv[0] );
    SQLFreeConnect(h);
    return JS_TRUE;
}


static JSBool
odbc_sql_free_env(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    HENV        h;

    JS_ASSERT(argc == 1);

    h = (HENV)JSVAL_TO_INT( argv[0] );
    SQLFreeEnv(h);
    return JS_TRUE;
}

static JSBool
odbc_sql_bind_parameter(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    HSTMT       h;
    int         rc;
    int         i;
    char *      str;
    JSString*   jstr;
    int         sqltype     = SQL_CHAR;

    JS_ASSERT(argc >= 3);

    h = (HSTMT)JSVAL_TO_INT( argv[0] );
    i = JSVAL_TO_INT( argv[1] );
    jstr = JS_ValueToString(cx, argv[2]);
    str = JS_GetStringBytes(jstr);
    if( argc > 3 )
    {
        char * s;
        s = JS_GetStringBytes(JS_ValueToString(cx, argv[3]));
        if( strcmp(s, "longvarchar")==0 )  sqltype = SQL_LONGVARCHAR;
        else if( strcmp(s, "date")==0 )  sqltype = SQL_TYPE_DATE;
        else if( strcmp(s, "time")==0 )  sqltype = SQL_TYPE_TIME;
    }
    rc = SQLBindParameter(h, i, SQL_PARAM_INPUT, SQL_C_CHAR, sqltype, strlen(str), 0, str, strlen(str), NULL );
    return JS_TRUE;
}

static JSBool
odbc_sql_col_attribute(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    SQLHSTMT        StatementHandle;
    SQLUSMALLINT    ColumnNumber;
    SQLUSMALLINT    FieldIdentifier;
    SQLPOINTER      CharacterAttributePtr;
    SQLSMALLINT     BufferLength;
    SQLSMALLINT     StringLength;
    SQLINTEGER      NumericAttribute;
    SQLRETURN       rc;
    char *          s;
    static char     buffer[8000];
    /* arguments:
     *      handle_type             in {"env", "dbc", "stmt", "desc"}
     *      handle                  opaque
     *      record number           integer
     *      diag identifier         in {"returncode", "row_count", "number", "message_text", "native"}
     *
     * returns
     *      diag info ptr as string
     */

    StatementHandle = JSVAL_TO_INT( argv[0] );
    ColumnNumber = JSVAL_TO_INT( argv[1] );

    s = JS_GetStringBytes(JSVAL_TO_STRING(argv[2]));
    if( strcmp(s, "label") == 0 )  FieldIdentifier = SQL_DESC_LABEL;
    else if( strcmp(s, "length") == 0 )  FieldIdentifier = SQL_DESC_LENGTH;
    else return JS_FALSE;

    CharacterAttributePtr = buffer;
    StringLength = sizeof(buffer);
    rc = SQLColAttribute (
            StatementHandle,
            ColumnNumber,
            FieldIdentifier,
            CharacterAttributePtr,
            sizeof(buffer),
            &StringLength,
            &NumericAttribute
            );


    switch( FieldIdentifier )
    {
        case SQL_DESC_LABEL:
            *rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, CharacterAttributePtr));
            break;
        case SQL_DESC_LENGTH:
            *rval = INT_TO_JSVAL( NumericAttribute );
            break;
    }

    return JS_TRUE;
}



static JSBool
odbc_sql_get_diag_field(
        JSContext *cx, 
        JSObject *obj, 
        uintN argc, 
        jsval *argv, 
        jsval *rval)
{
    SQLSMALLINT     HandleType;
    SQLHANDLE       Handle;
    SQLSMALLINT     RecNumber;
    SQLSMALLINT     DiagIdentifier;
    SQLPOINTER      DiagInfoPtr;
    SQLSMALLINT     BufferLength;
    SQLSMALLINT     StringLengthPtr;
    SQLRETURN       rc;
    char *          s;
    static char     buffer[8000];
    /* arguments:
     *      handle_type             in {"env", "dbc", "stmt", "desc"}
     *      handle                  opaque
     *      record number           integer
     *      diag identifier         in {"returncode", "row_count", "number", "message_text", "native"}
     *
     * returns
     *      diag info ptr as string
     */

    JS_ASSERT(argc == 4);

    s = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
    if( strcmp(s, "env") == 0 ) HandleType = SQL_HANDLE_ENV;
    else if( strcmp(s, "dbc") == 0 ) HandleType = SQL_HANDLE_DBC;
    else if( strcmp(s, "stmt") == 0 ) HandleType = SQL_HANDLE_STMT;
    else if( strcmp(s, "desc") == 0 ) HandleType = SQL_HANDLE_DESC;

    Handle = JSVAL_TO_INT( argv[1] );
    RecNumber = JSVAL_TO_INT( argv[2] );

    s = JS_GetStringBytes(JSVAL_TO_STRING(argv[3]));
    if( strcmp(s, "returncode") == 0 )  DiagIdentifier = SQL_DIAG_RETURNCODE;
    else if( strcmp(s, "row_count") == 0 )  DiagIdentifier = SQL_DIAG_ROW_COUNT;
    else if( strcmp(s, "number") == 0 )  DiagIdentifier = SQL_DIAG_NUMBER;
    else if( strcmp(s, "message_text") == 0 )  DiagIdentifier = SQL_DIAG_MESSAGE_TEXT;
    else if( strcmp(s, "native") == 0 )  DiagIdentifier = SQL_DIAG_NATIVE;
    else if( strcmp(s, "sqlstate") == 0 )  DiagIdentifier = SQL_DIAG_SQLSTATE;
    else return JS_FALSE;

    DiagInfoPtr = buffer;
    StringLengthPtr = sizeof(buffer);
    rc = SQLGetDiagField(HandleType, Handle, RecNumber, DiagIdentifier, DiagInfoPtr, sizeof(buffer), &StringLengthPtr );


    switch( DiagIdentifier )
    {
        case SQL_DIAG_MESSAGE_TEXT:
        case SQL_DIAG_SQLSTATE:
            *rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, DiagInfoPtr));
            break;
        case SQL_DIAG_RETURNCODE:
        case SQL_DIAG_ROW_COUNT:
        case SQL_DIAG_NUMBER:
        case SQL_DIAG_NATIVE:
            *rval = INT_TO_JSVAL( *((SQLINTEGER*)DiagInfoPtr) );
            break;
    }

    return JS_TRUE;
}





static JSClass odbc_class = {
    "ODBC", JSCLASS_NEW_RESOLVE,
    JS_PropertyStub,  JS_PropertyStub,  JS_PropertyStub,  JS_PropertyStub,
    JS_EnumerateStub,  (JSResolveOp)JS_ResolveStub,
    JS_ConvertStub,      JS_FinalizeStub
};

static JSFunctionSpec odbc_methods[] = {
    {"SQLAllocEnv",              odbc_sql_alloc_env,            1},
    {"SQLAllocConnect",          odbc_sql_alloc_connect,        1},
    {"SQLDriverConnect",         odbc_sql_driver_connect,       1},
    {"SQLAllocStmt",             odbc_sql_alloc_stmt,           1},
    {"SQLExecDirect",            odbc_sql_exec_direct,          1},
    {"SQLNumResultCols",         odbc_sql_num_result_cols,      1},
    {"SQLFetch",                 odbc_sql_fetch,                1},
    {"SQLGetData",               odbc_sql_get_data,             1},
    {"SQLFreeStmt",              odbc_sql_free_stmt,            1},
    {"SQLDisconnect",            odbc_sql_disconnect,           1},
    {"SQLFreeConnect",           odbc_sql_free_connect,         1},
    {"SQLFreeEnv",               odbc_sql_free_env,             1},
    {"SQLPrepare",               odbc_sql_prepare,              1},
    {"SQLBindParameter",         odbc_sql_bind_parameter,       1},
    {"SQLExecute",               odbc_sql_execute,              1},
    {"SQLGetDiagField",          odbc_sql_get_diag_field,       1},
    {"SQLColAttribute",          odbc_sql_col_attribute,        1},
    {"SQLTables",                odbc_sql_tables,               1},
    {"SQLColumns",               odbc_sql_columns,              1},
    {"SQLEndTran",               odbc_sql_end_tran,             1},
    {0}
};


JSBool
js_odbc_initialize(
    JSContext*      cx,
    JSObject*       glob)
{
    JSBool          status      = JS_TRUE;
    JSObject*       sap;

    sap = JS_DefineObject( cx, glob, "ODBC", &odbc_class, NULL, 0 );
    if( !sap ) status = JS_FALSE;
//  if( status && !JS_DefineProperties(cx, sap, its_props);
    if( status && !JS_DefineFunctions(cx, sap, odbc_methods) )
        status = JS_FALSE;
    return status;
}
