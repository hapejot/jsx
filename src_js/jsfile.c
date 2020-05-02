/*
 * JS File object
 * $Id: //depot/peter/w/js/jsfile.c#17 $
 */
#include "jsstddef.h"
#include "jspubtd.h"
#include "jsapi.h"
#include "jsstr.h"
#include "jsutil.h"
#include "jsobj.h"
#include "jsfun.h"
#include "jsregexp.h"
#include "platform.h"
// #include <dirent.h>

#define SPECIAL_FILE_STRING     "Special File"
#define CURRENTDIR_PROPERTY     "currentDir"
#define SEPARATOR_PROPERTY      "separator"
#define FILE_CONSTRUCTOR        "File"
#define PIPE_SYMBOL             '|'

#define ASCII                   0
#define UTF8                    1
#define UCS2                    2

#define asciistring             "text"
#define utfstring               "binary"
#define unicodestring           "unicode"

#define MAX_PATH_LENGTH         1024
#define MODE_SIZE               256
#define NUMBER_SIZE             32
#define MAX_LINE_LENGTH         256
#define URL_PREFIX              "file://"

#define STDINPUT_NAME           "Standard input stream"
#define STDOUTPUT_NAME          "Standard output stream"
#define STDERROR_NAME           "Standard error stream"

/* Error handling */
typedef enum JSFileErrNum {
#define MSG_DEF(name, number, count, exception, format) \
    name = number,
#include "jsfile.msg"
#undef MSG_DEF
    JSFileErr_Limit
#undef MSGDEF
} JSFileErrNum;

#define JSFILE_HAS_DFLT_MSG_STRINGS 1

JSErrorFormatString JSFile_ErrorFormatString[JSFileErr_Limit] = {
#define MSG_DEF(name, number, count, exception, format) \
    { format, count } ,
#include "jsfile.msg"
#undef MSG_DEF
};

const JSErrorFormatString *
JSFile_GetErrorMessage(
    void *          userRef, 
    const char *    locale,
    const uintN     errorNumber)
{
    if ((errorNumber > 0) && (errorNumber < JSFileErr_Limit))
        return &JSFile_ErrorFormatString[errorNumber];
	else
	    return NULL;
}

#define JSFILE_CHECK_WRITE      \
    if (!file->isOpen){     \
        JS_ReportWarning(cx,    \
                "File %s is closed, will open it for writing, proceeding",  \
                file->path);    \
    }else   \
    if(!js_canWrite(cx, file)){     \
        JS_ReportErrorNumber(cx, JSFile_GetErrorMessage, NULL,  \
            JSFILEMSG_CANNOT_WRITE, file->path);    \
    }

#define JSFILE_CHECK_READ      \
    if (!file->isOpen){     \
        JS_ReportWarning(cx,    \
                "File %s is closed, will open it for reading, proceeding",  \
                file->path); \
    }

#define JSFILE_CHECK_OPEN(op)      \
    if(!file->isOpen){     \
        JS_ReportErrorNumber(cx, JSFile_GetErrorMessage, NULL,  \
            JSFILEMSG_FILE_MUST_BE_CLOSED, op);    \
    }

#define JSFILE_CHECK_CLOSED(op)      \
    if(file->isOpen){     \
        JS_ReportErrorNumber(cx, JSFile_GetErrorMessage, NULL,  \
            JSFILEMSG_FILE_MUST_BE_OPEN, op);    \
    }

#define JSFILE_CHECK_ONE_ARG(op,stat)      \
    if (argc!=1){   \
        char str[NUMBER_SIZE];  \
                                \
        stat = JS_FALSE; \
        sprintf(str, "%d", argc);   \
        JS_ReportErrorNumber(cx, JSFile_GetErrorMessage, NULL,  \
            JSFILEMSG_EXPECTS_ONE_ARG_ERROR, op, str);  \
    }


/*
    Security mechanism, should define a callback for this.
    The parameters are as follows:
    SECURITY_CHECK(JSContext *cx, JSPrincipals *ps, char *op_name, JSFile *file)
*/
#define SECURITY_CHECK(cx, ps, op, file)    \
        /* Define a callback here... */


/* Structure representing the file internally */
typedef struct JSFile {
    char        *   path;          /* the path to the file. */
    JSBool          isOpen;
    JSString    *   linebuffer;    /* temp buffer used by readln. */
    int32           mode;           /* mode used to open the file: read, write, append, create, etc.. */
    int32           type;           /* Asciiz, utf, unicode */
    char            byteBuffer[3];  /* bytes read in advance by js_FileRead ( UTF8 encoding ) */
    jsint           nbBytesInBuf;   /* number of bytes stored in the buffer above */
    jschar          charBuffer;     /* character read in advance by readln ( mac files only ) */
    JSBool          charBufferUsed; /* flag indicating if the buffer above is being used */
    JSBool          hasRandomAccess;   /* can the file be randomly accessed? false for stdin, and
                                 UTF-encoded files. */
    JSBool          hasAutoflush;   /* should we force a flush for each line break? */
    /* We can actually put the following two in a union since they should never be used at the same time */
    int             handle;
    JSBool          isPipe;         /* if the file is really an OS pipe */
    unsigned char*  buffer;
	jsval*			chars;
    int             bufsize;
	JSBool			statLoaded;
	struct stat		stat;
    int             special;
} JSFile;

/* a few forward declarations... */
static JSClass file_class;
JS_PUBLIC_API(JSObject*) js_NewFileObject(JSContext *cx, char *filename);
static JSBool file_open(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static JSBool file_close(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

/* --------------------------- New filename manipulation procesures -------------------------- */
/* assumes we don't have leading/trailing spaces */
static JSBool
js_filenameHasAPipe(const char *filename)
{
    if(!filename) return JS_FALSE;
    return  filename[0]==PIPE_SYMBOL ||
            filename[strlen(filename)-1]==PIPE_SYMBOL;
}

static JSBool
js_isAbsolute(const char *name)
{
#if defined(XP_WIN) || defined(XP_OS2)
    return (strlen(name)>1)?((name[1]==':')?JS_TRUE:JS_FALSE):JS_FALSE;
#else
    return (name[0]
#   if defined(XP_UNIX) || defined(XP_BEOS)
            ==
#   else
            !=
#   endif
            FILESEPARATOR)?JS_TRUE:JS_FALSE;
#endif
}

/*
    Concatinates base and name to produce a valid filename.
    Returned string must be freed.
*/
static char*
js_combinePath(JSContext *cx, const char *base, const char *name)
{
    int len = strlen(base);
    char* result = (char*)JS_malloc(cx, len+strlen(name)+2);

    if (!result)  return NULL;

    strcpy(result, base);

    if (base[len-1]!=FILESEPARATOR
#if defined(XP_WIN) || defined(XP_OS2)
            && base[len-1]!=FILESEPARATOR2
#endif
            ) {
      result[len] = FILESEPARATOR;
      result[len+1] = '\0';
    }
    strcat(result, name);
    return result;
}

/* Extract the last component from a path name. Returned string must be freed */
static char *
js_fileBaseName(JSContext *cx, const char *pathname)
{
    jsint index, aux;
    char *result;

#if defined(XP_WIN) || defined(XP_OS2)
    /* First, get rid of the drive selector */
    if ((strlen(pathname)>=2)&&(pathname[1]==':')) {
        pathname = &pathname[2];
    }
#endif
    index = strlen(pathname)-1;
    /*
        remove trailing separators -- don't necessarily need to check for
        FILESEPARATOR2, but that's fine
    */
    while ((index>0)&&((pathname[index]==FILESEPARATOR)||
                       (pathname[index]==FILESEPARATOR2))) index--;
    aux = index;
    /* now find the next separator */
    while ((index>=0)&&(pathname[index]!=FILESEPARATOR)&&
                      (pathname[index]!=FILESEPARATOR2)) index--;
    /* allocate and copy */
    result = (char*)JS_malloc(cx, aux-index+1);
    if (!result)  return NULL;
    strncpy(result, &pathname[index+1], aux-index);
    result[aux-index] = '\0';
    return result;
}

/*
    Returns everything but the last component from a path name.
    Returned string must be freed.
*/
static char *
js_fileDirectoryName(JSContext *cx, const char *pathname)
{
    jsint index;
    char  *result;

#if defined(XP_WIN) || defined(XP_OS2)
    char  drive = '\0';
    const char *oldpathname = pathname;

    /* First, get rid of the drive selector */
    if ((strlen(pathname)>=2)&&(pathname[1]==':')) {
        drive = pathname[0];
        pathname = &pathname[2];
    }
#endif
    index = strlen(pathname)-1;
    while ((index>0)&&((pathname[index]==FILESEPARATOR)||
                       (pathname[index]==FILESEPARATOR2))) index--;
    while ((index>0)&&(pathname[index]!=FILESEPARATOR)&&
                      (pathname[index]!=FILESEPARATOR2)) index--;

    if (index>=0){
        result = (char*)JS_malloc(cx, index+4);
        if (!result)  return NULL;
#if defined(XP_WIN) || defined(XP_OS2)
        if (drive!='\0') {
            result[0] = toupper(drive);
            result[1] = ':';
            strncpy(&result[2], pathname, index);
			result[index+3] = '\0';
        }else
#endif
        {
            strncpy(result, pathname, index+1);
			result[index+1] = '\0';
        }

        /* add terminating separator */
        index = strlen(result)-1;
        result[index] = FILESEPARATOR;
        result[index+1] = '\0';
    } else{
#if defined(XP_WIN) || defined(XP_OS2)
        result = JS_strdup(cx, oldpathname); /* may include drive selector */
#else
        result = JS_strdup(cx, pathname);
#endif
    }

    return result;
}

static char *
js_absolutePath(JSContext *cx, const char * path)
{
    JSObject *obj;
    JSString *str;
    jsval prop;

    if (js_isAbsolute(path)){
        return JS_strdup(cx, path);
    }else{
        obj = JS_GetGlobalObject(cx);
        if (!JS_GetProperty(cx, obj, FILE_CONSTRUCTOR, &prop)) {
            JS_ReportErrorNumber(cx, JSFile_GetErrorMessage, NULL,
                 JSFILEMSG_FILE_CONSTRUCTOR_UNDEFINED_ERROR);
            return JS_strdup(cx, path);
        }
        obj = JSVAL_TO_OBJECT(prop);
        if (!JS_GetProperty(cx, obj, CURRENTDIR_PROPERTY, &prop)) {
            JS_ReportErrorNumber(cx, JSFile_GetErrorMessage, NULL,
                 JSFILEMSG_FILE_CURRENTDIR_UNDEFINED_ERROR);
            return JS_strdup(cx, path);
        }
        str = JS_ValueToString(cx, prop);
        if (!str ) {
            return JS_strdup(cx, path);
        }
        /* should we have an array of curr dirs indexed by drive for windows? */
        return js_combinePath(cx, JS_GetStringBytes(str), path);
    }
}

/* Side effect: will remove spaces in the beginning/end of the filename */
static char *
js_canonicalPath(
    JSContext *     cx, 
    char *          oldpath)
{
    char *          tmp;
    char *          path            = oldpath;
    char *          base;
    char *          dir;
    char *          current;
    char *          result;
    jsint           c;
    jsint           back            = 0;
    unsigned int    i               = 0;
    unsigned int    j               = strlen(path)-1;

    /* This is probably optional */
	/* Remove possible spaces in the beginning and end */
    while(i<strlen(path)-1 && path[i]==' ') i++;
	while(j>=0 && path[j]==' ') j--;

	tmp = JS_malloc(cx, j-i+2);
	strncpy(tmp, &path[i], j-i+1);
    tmp[j-i+1] = '\0';

    path = tmp;

    /* pipe support */
    if(js_filenameHasAPipe(path))
        return JS_strdup(cx, path);
    /* file:// support */
    if(!strncmp(path, URL_PREFIX, strlen(URL_PREFIX)))
        return js_canonicalPath(cx, &path[strlen(URL_PREFIX)-1]);

    if (!js_isAbsolute(path))
        path = js_absolutePath(cx, path);
    else
        path = JS_strdup(cx, path);

    result = JS_strdup(cx, "");

    current = path;

    base = js_fileBaseName(cx, current);
    dir = js_fileDirectoryName(cx, current);

    /* TODO: MAC -- not going to work??? */
    while (strcmp(dir, current)) {
        if (!strcmp(base, "..")) {
            back++;
        } else
        if(!strcmp(base, ".")){
            /* ??? */
        } else {
            if (back>0)
                back--;
            else {
                tmp = result;
                result = JS_malloc(cx, strlen(base)+1+strlen(tmp)+1);
                if (!result) {
                    JS_free(cx, dir);
                    JS_free(cx, base);
                    JS_free(cx, current);
                    return NULL;
                }
                strcpy(result, base);
                c = strlen(result);
                if (*tmp) {
                    result[c] = FILESEPARATOR;
                    result[c+1] = '\0';
                    strcat(result, tmp);
                }
                JS_free(cx, tmp);
            }
        }
        JS_free(cx, current);
        JS_free(cx, base);
        current = dir;
        base =  js_fileBaseName(cx, current);
        dir = js_fileDirectoryName(cx, current);
    }

    tmp = result;
    result = JS_malloc(cx, strlen(dir)+1+strlen(tmp)+1);
    if (!result) {
        JS_free(cx, dir);
        JS_free(cx, base);
        JS_free(cx, current);
        return NULL;
    }
    strcpy(result, dir);
    c = strlen(result);
    if (tmp[0]!='\0') {
        if ((result[c-1]!=FILESEPARATOR)&&(result[c-1]!=FILESEPARATOR2)) {
            result[c] = FILESEPARATOR;
            result[c+1] = '\0';
        }
        strcat(result, tmp);
    }
    JS_free(cx, tmp);
    JS_free(cx, dir);
    JS_free(cx, base);
    JS_free(cx, current);

    return result;
}

/* -------------------------- Text conversion ------------------------------- */
/* The following is ripped from libi18n/unicvt.c and include files.. */

/*
 * UTF8 defines and macros
 */
#define ONE_OCTET_BASE          0x00    /* 0xxxxxxx */
#define ONE_OCTET_MASK          0x7F    /* x1111111 */
#define CONTINUING_OCTET_BASE   0x80    /* 10xxxxxx */
#define CONTINUING_OCTET_MASK   0x3F    /* 00111111 */
#define TWO_OCTET_BASE          0xC0    /* 110xxxxx */
#define TWO_OCTET_MASK          0x1F    /* 00011111 */
#define THREE_OCTET_BASE        0xE0    /* 1110xxxx */
#define THREE_OCTET_MASK        0x0F    /* 00001111 */
#define FOUR_OCTET_BASE         0xF0    /* 11110xxx */
#define FOUR_OCTET_MASK         0x07    /* 00000111 */
#define FIVE_OCTET_BASE         0xF8    /* 111110xx */
#define FIVE_OCTET_MASK         0x03    /* 00000011 */
#define SIX_OCTET_BASE          0xFC    /* 1111110x */
#define SIX_OCTET_MASK          0x01    /* 00000001 */

#define IS_UTF8_1ST_OF_1(x) (( (x)&~ONE_OCTET_MASK  ) == ONE_OCTET_BASE)
#define IS_UTF8_1ST_OF_2(x) (( (x)&~TWO_OCTET_MASK  ) == TWO_OCTET_BASE)
#define IS_UTF8_1ST_OF_3(x) (( (x)&~THREE_OCTET_MASK) == THREE_OCTET_BASE)
#define IS_UTF8_1ST_OF_4(x) (( (x)&~FOUR_OCTET_MASK ) == FOUR_OCTET_BASE)
#define IS_UTF8_1ST_OF_5(x) (( (x)&~FIVE_OCTET_MASK ) == FIVE_OCTET_BASE)
#define IS_UTF8_1ST_OF_6(x) (( (x)&~SIX_OCTET_MASK  ) == SIX_OCTET_BASE)
#define IS_UTF8_2ND_THRU_6TH(x) \
                    (( (x)&~CONTINUING_OCTET_MASK  ) == CONTINUING_OCTET_BASE)
#define IS_UTF8_1ST_OF_UCS2(x) \
            IS_UTF8_1ST_OF_1(x) \
            || IS_UTF8_1ST_OF_2(x) \
            || IS_UTF8_1ST_OF_3(x)


#define MAX_UCS2            0xFFFF
#define DEFAULT_CHAR        0x003F  /* Default char is "?" */
#define BYTE_MASK           0xBF
#define BYTE_MARK           0x80


/* Function: one_ucs2_to_utf8_char
 *
 * Function takes one UCS-2 char and writes it to a UTF-8 buffer.
 * We need a UTF-8 buffer because we don't know before this
 * function how many bytes of utf-8 data will be written. It also
 * takes a pointer to the end of the UTF-8 buffer so that we don't
 * overwrite data. This function returns the number of UTF-8 bytes
 * of data written, or -1 if the buffer would have been overrun.
 */

#define LINE_SEPARATOR      0x2028
#define PARAGRAPH_SEPARATOR 0x2029
static int16 one_ucs2_to_utf8_char(unsigned char *tobufp,
        unsigned char *tobufendp, uint16 onechar)
{

     int16 numUTF8bytes = 0;

    if((onechar == LINE_SEPARATOR)||(onechar == PARAGRAPH_SEPARATOR))
    {
        strcpy((char*)tobufp, "\n");
        return strlen((char*)tobufp);;
    }

        if (onechar < 0x80) {               numUTF8bytes = 1;
        } else if (onechar < 0x800) {       numUTF8bytes = 2;
        } else if (onechar <= MAX_UCS2) {   numUTF8bytes = 3;
        } else { numUTF8bytes = 2;
                 onechar = DEFAULT_CHAR;
        }

        tobufp += numUTF8bytes;

        /* return error if we don't have space for the whole character */
        if (tobufp > tobufendp) {
            return(-1);
        }


        switch(numUTF8bytes) {

            case 3: *--tobufp = (onechar | BYTE_MARK) & BYTE_MASK; onechar >>=6;
                    *--tobufp = (onechar | BYTE_MARK) & BYTE_MASK; onechar >>=6;
                    *--tobufp = onechar |  THREE_OCTET_BASE;
                    break;

            case 2: *--tobufp = (onechar | BYTE_MARK) & BYTE_MASK; onechar >>=6;
                    *--tobufp = onechar | TWO_OCTET_BASE;
                    break;
            case 1: *--tobufp = (unsigned char)onechar;  break;
        }

        return(numUTF8bytes);
}

/*
 * utf8_to_ucs2_char
 *
 * Convert a utf8 multibyte character to ucs2
 *
 * inputs: pointer to utf8 character(s)
 *         length of utf8 buffer ("read" length limit)
 *         pointer to return ucs2 character
 *
 * outputs: number of bytes in the utf8 character
 *          -1 if not a valid utf8 character sequence
 *          -2 if the buffer is too short
 */
static int16
utf8_to_ucs2_char(const unsigned char *utf8p, int16 buflen, uint16 *ucs2p)
{
    uint16 lead, cont1, cont2;

    /*
     * Check for minimum buffer length
     */
    if ((buflen < 1) || (utf8p == NULL)) {
        return -2;
    }
    lead = (uint16) (*utf8p);

    /*
     * Check for a one octet sequence
     */
    if (IS_UTF8_1ST_OF_1(lead)) {
        *ucs2p = lead & ONE_OCTET_MASK;
        return 1;
    }

    /*
     * Check for a two octet sequence
     */
    if (IS_UTF8_1ST_OF_2(*utf8p)) {
        if (buflen < 2)
            return -2;
        cont1 = (uint16) *(utf8p+1);
        if (!IS_UTF8_2ND_THRU_6TH(cont1))
            return -1;
        *ucs2p =  (lead & TWO_OCTET_MASK) << 6;
        *ucs2p |= cont1 & CONTINUING_OCTET_MASK;
        return 2;
    }

    /*
     * Check for a three octet sequence
     */
    else if (IS_UTF8_1ST_OF_3(lead)) {
        if (buflen < 3)
            return -2;
        cont1 = (uint16) *(utf8p+1);
        cont2 = (uint16) *(utf8p+2);
        if (   (!IS_UTF8_2ND_THRU_6TH(cont1))
            || (!IS_UTF8_2ND_THRU_6TH(cont2)))
            return -1;
        *ucs2p =  (lead & THREE_OCTET_MASK) << 12;
        *ucs2p |= (cont1 & CONTINUING_OCTET_MASK) << 6;
        *ucs2p |= cont2 & CONTINUING_OCTET_MASK;
        return 3;
    }
    else { /* not a valid utf8/ucs2 character */
        return -1;
    }
}

/* ----------------------------- Helper functions --------------------------- */
/* Ripped off from lm_win.c .. */
/* where is strcasecmp?.. for now, it's case sensitive..
 *
 * strcasecmp is in strings.h, but on windows it's called _stricmp...
 * will need to #ifdef this
*/


static void
file_load_stat( JSFile* p_file )
{
	if( p_file->statLoaded == JS_FALSE )
	{
		int rc = lstat( p_file->path, &p_file->stat );
		if( rc == 0 )
		{
			p_file->statLoaded = JS_TRUE;
		}
	}
}

static int32
js_FileHasOption(JSContext *cx, const char *oldoptions, const char *name)
{
    char *comma, *equal, *current;
    char *options = JS_strdup(cx, oldoptions);
    int32 found = 0;

	current = options;
    for (;;) {
        comma = strchr(current, ',');
        if (comma) *comma = '\0';
        equal = strchr(current, '=');
        if (equal) *equal = '\0';
        if (strcmp(current, name) == 0) {
            if (!equal || strcmp(equal + 1, "yes") == 0)
                found = 1;
            else
                found = atoi(equal + 1);
        }
        if (equal) *equal = '=';
        if (comma) *comma = ',';
        if (found || !comma)
            break;
        current = comma + 1;
    }
    JS_free(cx, options);
    return found;
}

/* empty the buffer */
static void
js_ResetBuffers(JSFile * file)
{
    file->charBufferUsed = JS_FALSE;
    file->nbBytesInBuf = 0;
    file->linebuffer = NULL;    /* TODO: check for mem. leak? */
}

/* Reset file attributes */
static void
js_ResetAttributes(JSFile * file){
	file->mode      = 0;
    file->type      = 0;
    file->isOpen    = JS_FALSE;
    file->handle    = 0;
    file->hasRandomAccess   = JS_TRUE; /* innocent until proven guilty */
	file->hasAutoflush      = JS_FALSE;
    file->isPipe            = JS_FALSE;
	file->statLoaded		= JS_FALSE;

	js_ResetBuffers(file);
}

/* ----------------------------- Property checkers -------------------------- */
static JSBool
js_exists(JSContext *cx, JSFile *file)
{
    return JS_TRUE;
}

static JSBool
js_canRead(JSContext *cx, JSFile *file)
{
    return JS_TRUE;
}

static JSBool
js_canWrite(JSContext *cx, JSFile *file)
{
    return JS_TRUE;
}

static JSBool
js_isFile(JSContext *cx, JSFile *file)
{
    return JS_TRUE;
}

static JSBool
js_isDirectory(JSContext *cx, JSFile *file)
{
    return JS_TRUE;
}

static jsval
js_size(JSContext *cx, JSFile *file)
{
    return JSVAL_VOID;
}

/* Return the parent object. Actually returns the Object of the parent directory. */
static jsval
js_parent(JSContext *cx, JSFile *file)
{
    return JSVAL_VOID;
}

static jsval
js_name(JSContext *cx, JSFile *file){
    return STRING_TO_JSVAL(JS_NewStringCopyZ(cx, js_fileBaseName(cx, file->path)));
}

/* ------------------------------ File object methods ---------------------------- */
static JSBool
file_open(
    JSContext *     cx, 
    JSObject *      obj, 
    uintN           argc, 
    jsval *         argv, 
    jsval *         rval)
{
    JSFile      *   file        = JS_GetInstancePrivate(cx, obj, &file_class, NULL);
    JSString	*   strmode;
    JSString *      strtype;
    char        *   ctype;
    char *          mode;
    int32           mask;
    int32           type;
    int             len;
    JSBool          status      = JS_TRUE;

    SECURITY_CHECK(cx, NULL, "open", file);

    /* Mode */
    if (argc >= 1)
    {
        JSString *  str;
        char *      c1;
        char *      c2;
        int         n;

        file->special = -1;

        str = JS_ValueToString(cx, argv[0]);
        c1 = JS_GetStringBytes(str); 
        while( c1 )
        {
            c2 = strchr( c1, ',' );
            if( c2 == NULL )
                n = strlen( c1 );
            else
            {
                n = c2 - c1;
                c2++;
            }
            if( strncmp("read", c1, n) == 0 )
                file->mode |= O_RDONLY;
            else if( strncmp("write", c1, n) == 0 )
                file->mode |= O_WRONLY;
            else if( strncmp("create", c1, n) == 0 )
                file->mode |= O_CREAT;
            else if( strncmp("truncate", c1, n) == 0 )
                file->mode |= O_TRUNC;
            else if( strncmp("append", c1, n) == 0 )
                file->mode |= O_APPEND;
            else if( strncmp("stdin", c1, n) == 0 )
                file->special = 0;
            else if( strncmp("stdout", c1, n) == 0 )
                file->special = 1;
            else if( strncmp("stderr", c1, n) == 0 )
                file->special = 2;
            c1 = c2;
        }
    }
    else
        file->mode = O_RDONLY;

    if( file->special >= 0 )
    {
        file->handle = file->special;
        file->statLoaded = 1;
    }
    else
    {
        file->handle = open(file->path, file->mode | O_BINARY, S_IREAD | S_IWRITE );
    }
    if( file->handle < 0 )
    {
        
        JS_ReportErrorNumber(cx, JSFile_GetErrorMessage, NULL,
            JSFILEMSG_OPEN_FAILED, STRING_TO_JSVAL(file->path));
        status = JS_FALSE;
    }
    else
    {
        file->isOpen = JS_TRUE;
    }
    if( status == JS_TRUE )
        *rval = JSVAL_TRUE;
    else
        *rval = JSVAL_FALSE;
    return status;
}

static JSBool
file_close(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSFile  *file = JS_GetInstancePrivate(cx, obj, &file_class, NULL);

    SECURITY_CHECK(cx, NULL, "close", file);

    if(file->isOpen)
    {
        close( file->handle );
    }

    js_ResetAttributes(file);
    *rval = JSVAL_TRUE;
    return JS_TRUE;
}


static JSBool
file_remove(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	JSFile  *file = JS_GetInstancePrivate(cx, obj, &file_class, NULL);

    SECURITY_CHECK(cx, NULL, "remove", file);
    JSFILE_CHECK_CLOSED("remove");

    *rval = JSVAL_FALSE;
    return JS_FALSE;
}

/* Raw PR-based function. No text processing. Just raw data copying. */
static JSBool
file_copyTo(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSFile      *file = JS_GetInstancePrivate(cx, obj, &file_class, NULL);
    char        *dest = NULL;
    char        *buffer;
    jsval		count, size;
    JSBool      fileInitiallyOpen=JS_FALSE;
    JSBool      status      = JS_TRUE;

    SECURITY_CHECK(cx, NULL, "copyTo", file);   /* may need a second argument!*/
    JSFILE_CHECK_ONE_ARG("copyTo", status);

    *rval = JSVAL_FALSE;
    return status;
}

static JSBool
file_renameTo(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSFile  *file = JS_GetInstancePrivate(cx, obj, &file_class, NULL);
    char    *dest;
    JSBool      status      = JS_TRUE;

    SECURITY_CHECK(cx, NULL, "renameTo", file); /* may need a second argument!*/
    JSFILE_CHECK_ONE_ARG("renameTo", status);
    JSFILE_CHECK_CLOSED("renameTo");

    *rval = JSVAL_FALSE;
    return status;
}

static JSBool
file_flush(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSFile *file = JS_GetInstancePrivate(cx, obj, &file_class, NULL);

    SECURITY_CHECK(cx, NULL, "flush", file);
    JSFILE_CHECK_OPEN("flush");

    *rval = JSVAL_FALSE;
    return JS_FALSE;
}

static JSBool
file_write(
    JSContext *     cx, 
    JSObject *      obj, 
    uintN           argc, 
    jsval *         argv, 
    jsval *         rval)
{
    JSFile      *   file = JS_GetInstancePrivate(cx, obj, &file_class, NULL);
    JSString    *   str;
    int32           count;
    uintN           i;
    JSBool          status          = JS_TRUE;
    JSBool          rc; 

	if (!file->isOpen)
	{
		jsval		exn;
        JSObject   *exn_o;
		jsval		vp;


		JS_ReportErrorNumber(cx, 
				JSFile_GetErrorMessage, 
				NULL, 
				JSFILEMSG_CANNOT_WRITE, 
				file->path);
		status = JS_FALSE;
		JS_GetPendingException( cx, &exn );
        rc = JS_ValueToObject(cx, exn, &exn_o);
        JS_ASSERT(rc == JS_TRUE);
		vp = INT_TO_JSVAL(JSFILEMSG_CANNOT_WRITE);
		JS_SetProperty(cx, exn_o, "no", &vp);
		vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, "File"));
		JS_SetProperty(cx, exn_o, "facility", &vp );
	}
	else
	{
		for( i = 0; i < argc; i++ )
		{
			jsuint      j;
			if( JSVAL_IS_OBJECT(argv[i]) 
					&& JS_IsArrayObject(cx, JSVAL_TO_OBJECT(argv[i])) )
			{
				jsuint      n;
				jsval       v;

				JS_GetArrayLength( cx, JSVAL_TO_OBJECT(argv[i]), &n);
				if( n > file->bufsize )
				{
					file->bufsize = n;
					file->buffer = JS_realloc( cx, file->buffer, file->bufsize );
				}
				for( j = 0; j < n; j++ )
				{
					JS_GetElement( cx, JSVAL_TO_OBJECT(argv[i]), j, &v );
					file->buffer[j] = JSVAL_TO_INT(v);
				}
				j = write( file->handle, file->buffer, n );
				*rval = INT_TO_JSVAL(j);
			}
			else if( JSVAL_IS_STRING(argv[i]) )
			{
				char *      buf = JS_GetStringBytes(JSVAL_TO_STRING(argv[i]));
				jsuint      n =  JS_GetStringLength(JSVAL_TO_STRING(argv[i]));
				j = write( file->handle, buf, n );
				*rval = INT_TO_JSVAL(j);
			}
		}
	}

    return status;
}

static JSBool
file_writeln(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSFile      *file = JS_GetInstancePrivate(cx, obj, &file_class, NULL);
    JSString    *str;

    SECURITY_CHECK(cx, NULL, "writeln", file);

    *rval = JSVAL_FALSE;
    return JS_FALSE;
}

static JSBool
file_writeAll(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSFile      *file = JS_GetInstancePrivate(cx, obj, &file_class, NULL);
    jsuint      i;
    jsuint      limit;
    JSObject    *array;
    JSObject    *elem;
    jsval       elemval;
    JSBool      status      = JS_TRUE;

    SECURITY_CHECK(cx, NULL, "writeAll", file);
    JSFILE_CHECK_ONE_ARG("writeAll", status);
    JSFILE_CHECK_WRITE;

    *rval = JSVAL_FALSE;
    return status;
}

static JSBool file_filelist(
    JSContext *     p_cx, 
    JSObject *      p_obj, 
    uintN           p_argc, 
    jsval *         p_argv, 
    jsval *         p_result)
{
    JSFile*         file;
    JSString*       str;
    int32           want;
    int32           count;
    jschar*         buf             = NULL;
    JSBool          status          = JS_TRUE;
    extern JSClass  js_ArrayClass;
    jsuint          n;
    jsuint          i;
    jsval           v;
    JSObject*       a;
    int             k;

    *p_result = JSVAL_FALSE;
    file = JS_GetInstancePrivate(p_cx, p_obj, &file_class, NULL);


	file_load_stat( file );
	if( S_ISDIR(file->stat.st_mode) )
	{
//      DIR* dir;
        JSXDIR  dir     = NULL;
//      struct dirent* dirent;
        JSXDIRENT       dirent;
        int rc          = 1; 

//	    dir = opendir( file->path );
		a = js_ConstructObject( p_cx, &js_ArrayClass, NULL, NULL, 0, NULL);
		i = 0;
		while( 1 )
		{
            if( dir == NULL )
                dir = jsx_find_first_file( file->path, &dirent );
            else
                rc = jsx_find_next_file( dir, &dirent );
			if( rc == 0 )
				break;
			str = JS_NewStringCopyZ(p_cx, jsx_get_filename(&dirent));
			v = STRING_TO_JSVAL(str);
			JS_SetElement( p_cx, a, i, &v );
			i++;
		}
		// closedir( dir );
        jsx_find_close(dir);
		*p_result = OBJECT_TO_JSVAL(a);
	}
	else
		status = JS_FALSE;

    return status;
}

static int timer1 = 0;
static int count1 = 0;

static JSBool
file_read(
    JSContext *     p_cx, 
    JSObject *      p_obj, 
    uintN           p_argc, 
    jsval *         p_argv, 
    jsval *         p_result)
{
    JSFile*         file;
    JSString*       str;
    int32           want;
    int32           count;
    jschar*         buf             = NULL;
    JSBool          status          = JS_TRUE;
    extern JSClass  js_ArrayClass;
    jsuint          n;
    jsuint          i;
    jsval           v;
    JSObject*       a;
    int             read_cnt;
	struct timeval tv0;
	struct timeval tv;
	// gettimeofday(&tv0,NULL);

    *p_result = JSVAL_FALSE;
    file = JS_GetInstancePrivate(p_cx, p_obj, &file_class, NULL);
    SECURITY_CHECK(p_cx, NULL, "read", file);

    if (status &&
            !JS_ValueToInt32(p_cx, p_argv[0], &want))
    {
        JS_ReportErrorNumber(p_cx, JSFile_GetErrorMessage, NULL,
            JSFILEMSG_FIRST_ARGUMENT_MUST_BE_A_NUMBER, "read", p_argv[0]);
    }

    if( want > file->bufsize )
    {
        file->bufsize = want;
        file->buffer = JS_realloc( p_cx, file->buffer, file->bufsize );
		file->chars  = JS_realloc( p_cx, file->chars,  file->bufsize * sizeof(jsval));
    }
    read_cnt = read( file->handle, file->buffer, want );
    if( read_cnt >= 0 )
    {
	for( i = 0; i < read_cnt; i++ )
	{
	    file->chars[i] = INT_TO_JSVAL(file->buffer[i]);
	}
	*p_result = JS_NewArrayObject(
				p_cx, 
				read_cnt, 
				file->chars);
		/*
        a = js_ConstructObject( p_cx, 
                                &js_ArrayClass, NULL, NULL, 0, NULL);
        for( i = 0; i < k; i++ )
        {
            v = INT_TO_JSVAL(file->buffer[i]);
            JS_SetElement( p_cx, a, i, &v );
        }
        *p_result = OBJECT_TO_JSVAL(a);
		*/
    }
	// gettimeofday(&tv,NULL);
	tv.tv_usec -= tv0.tv_usec;
	if( tv.tv_usec > 0 )
	{
	    timer1 += tv.tv_usec;
	    count1++;
	}

    return status;
}

static JSBool
file_readln(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSFile      *file = JS_GetInstancePrivate(cx, obj, &file_class, NULL);
    JSString    *str;
    jschar      *buf;
    int32       offset;
    intN        room;
    jschar      data, data2;
    JSBool      endofline;

    SECURITY_CHECK(cx, NULL, "readln", file);
    JSFILE_CHECK_READ;

    if (!file->linebuffer) {
        buf = JS_malloc(cx, MAX_LINE_LENGTH*(sizeof data));
        file->linebuffer = JS_NewUCString(cx, buf, MAX_LINE_LENGTH);
    }
    room = JS_GetStringLength(file->linebuffer);
    offset = 0;

    *rval = JSVAL_NULL;
    return JS_FALSE;
}

static JSBool
file_readAll(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSFile      *file = JS_GetInstancePrivate(cx, obj, &file_class, NULL);
    JSObject    *array;
    jsint       len;
    jsval       line;

    SECURITY_CHECK(cx, NULL, "readAll", file);
    JSFILE_CHECK_READ;

    array = JS_NewArrayObject(cx, 0, NULL);
    len = 0;

    while(file_readln(cx, obj, 0, NULL, &line)){
        JS_SetElement(cx, array, len, &line);
        len++;
    }

    *rval = OBJECT_TO_JSVAL(array);
    return JS_TRUE;
out:
    *rval = JSVAL_FALSE;
    return JS_FALSE;
}

static JSBool
file_seek(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSFile      *file = JS_GetInstancePrivate(cx, obj, &file_class, NULL);
    int32       toskip;
    int32       pos;
    JSBool      status      = JS_TRUE;

    SECURITY_CHECK(cx, NULL, "seek", file);
    JSFILE_CHECK_ONE_ARG("seek", status);
    JSFILE_CHECK_READ;

    if (!JS_ValueToInt32(cx, argv[0], &toskip)){
        JS_ReportErrorNumber(cx, JSFile_GetErrorMessage, NULL,
            JSFILEMSG_FIRST_ARGUMENT_MUST_BE_A_NUMBER, "seek", argv[0]);
        goto out;
    }

    if(!file->hasRandomAccess){
        JS_ReportErrorNumber(cx, JSFile_GetErrorMessage, NULL,
            JSFILEMSG_NO_RANDOM_ACCESS, file->path);
       goto out;
    }

    if(js_isDirectory(cx, file)){
        JS_ReportWarning(cx,"Seek on directories is not supported, proceeding");
        goto out;
    }

    if (pos!=-1) {
        *rval = INT_TO_JSVAL(pos);
        return JS_TRUE;
    }
out:
    *rval = JSVAL_VOID;
    return status;
}

static JSBool
file_mkdir(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSFile      *file = JS_GetInstancePrivate(cx, obj, &file_class, NULL);
    JSBool      status  = JS_TRUE;

    SECURITY_CHECK(cx, NULL, "mkdir", file);
    JSFILE_CHECK_ONE_ARG("mkdir", status);

    *rval = JSVAL_TRUE;
    return status;
}

static JSBool
file_toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval*rval)
{
    JSFile *file = JS_GetInstancePrivate(cx, obj, &file_class, NULL);

    *rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, file->path));
    return JS_TRUE;
}

static JSBool
file_toURL(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    JSFile *file = JS_GetInstancePrivate(cx, obj, &file_class, NULL);
    char url[MAX_PATH_LENGTH];
    jschar *urlChars;


    sprintf(url, "file://%s", file->path);
    /* TODO: js_escape in jsstr.h may go away at some point */

    urlChars = js_InflateString(cx, url, strlen(url));
    if (urlChars == NULL) return JS_FALSE;
    *rval = STRING_TO_JSVAL(js_NewString(cx, urlChars, strlen(url), 0));
    if (!js_str_escape(cx, obj, 0, rval, rval)) return JS_FALSE;

    return JS_TRUE;
out:
    *rval = JSVAL_VOID;
    return JS_FALSE;
}


static void
file_finalize(JSContext *cx, JSObject *obj)
{
    JSFile *file = JS_GetInstancePrivate(cx, obj, &file_class, NULL);

    if(file)
    {
	if (file->path)
	    JS_free(cx, file->path);
	JS_free(cx, file);
    }
    // fprintf(stderr,"timer1:%d / %d\n", timer1, count1);
}

/*
    Allocates memory for the file object, sets fields to defaults.
*/
static JSFile*
file_init(JSContext *cx, JSObject *obj, char *bytes)
{
    JSFile      *file;

    file = JS_malloc(cx, sizeof *file);
    if (!file) return NULL;
    memset(file, 0 , sizeof *file);

    js_ResetAttributes(file);

    file->path = js_canonicalPath(cx, bytes);

    if (!JS_SetPrivate(cx, obj, file)) {
        JS_ReportErrorNumber(cx, JSFile_GetErrorMessage, NULL,
            JSFILEMSG_CANNOT_SET_PRIVATE_FILE, file->path);
        JS_free(cx, file);
        return NULL;
    }else
        return file;
}

/* Returns a JSObject. This function is globally visible */
JS_PUBLIC_API(JSObject*)
js_NewFileObject(JSContext *cx, char *filename)
{
    JSObject    *obj;
    JSFile      *file;

    obj = JS_NewObject(cx, &file_class, NULL, NULL);
    if (!obj){
        JS_ReportErrorNumber(cx, JSFile_GetErrorMessage, NULL,
            JSFILEMSG_OBJECT_CREATION_FAILED, "js_NewFileObject");
        return NULL;
    }
    file = file_init(cx, obj, filename);
    if(!file) return NULL;
    return obj;
}

JSObject*
js_NewFileObjectFromFILE(JSContext *cx, FILE *nativehandle, char *filename,
    int32 mode, JSBool open, JSBool randomAccess)
{
    return NULL;
}

/*
    Real file constructor that is called from JavaScript.
    Basically, does error processing and calls file_init.
*/
static JSBool
file_constructor(
    JSContext *     cx, 
    JSObject *      obj, 
    uintN           argc, 
    jsval *         argv,
    jsval *         rval)
{
    JSString *      str;
    JSFile   *      file;

    str = (argc==0)?JS_InternString(cx, ""):JS_ValueToString(cx, argv[0]);

    if (!str){
        JS_ReportErrorNumber(cx, JSFile_GetErrorMessage, NULL,
            JSFILEMSG_FIRST_ARGUMENT_CONSTRUCTOR_NOT_STRING_ERROR, argv[0]);
        goto out;
    }

    file = file_init(cx, obj, JS_GetStringBytes(str));
    if (!file)  goto out;

    SECURITY_CHECK(cx, NULL, "constructor", file);

    return JS_TRUE;
out:
    *rval = JSVAL_VOID;
    return JS_FALSE;
}

/* -------------------- File methods and properties ------------------------- */
static JSFunctionSpec file_functions[] = {
    { "open",           file_open, 0},
    { "close",          file_close, 0},
    { "remove",         file_remove, 0},
    { "copyTo",         file_copyTo, 0},
    { "renameTo",       file_renameTo, 0},
    { "flush",          file_flush, 0},
    { "filelist",       file_filelist, 0},
    { "seek",           file_seek, 0},
    { "read",           file_read, 0},
    { "readln",         file_readln, 0},
    { "readAll",        file_readAll, 0},
    { "write",          file_write, 0},
    { "writeln",        file_writeln, 0},
    { "writeAll",       file_writeAll, 0},
    { "mkdir",          file_mkdir, 0},
	{ "toString",       file_toString, 0},
    { "toURL",			file_toURL, 0},
    {0}
};

enum file_tinyid {
    FILE_LENGTH             = -2,
    FILE_PARENT             = -3,
    FILE_PATH               = -4,
    FILE_NAME               = -5,
    FILE_ISDIR              = -6,
    FILE_ISFILE             = -7,
    FILE_EXISTS             = -8,
    FILE_CANREAD            = -9,
    FILE_CANWRITE           = -10,
    FILE_OPEN               = -11,
    FILE_TYPE               = -12,
    FILE_MODE               = -13,
    FILE_CREATED            = -14,
    FILE_MODIFIED           = -15,
    FILE_SIZE               = -16,
    FILE_RANDOMACCESS       = -17,
    FILE_POSITION           = -18,
    FILE_APPEND             = -19,
    FILE_REPLACE            = -20,
    FILE_AUTOFLUSH          = -21,
	FILE_ISLINK				= -22,
	FILE_LINKTARGET			= -23,
	FILE_DIR				= -24,
    FILE_SEP                = -25,
    FILE_SPECIAL            = -26,
};

static JSPropertySpec file_props[] = {
   {"length",          FILE_LENGTH,        JSPROP_ENUMERATE | JSPROP_READONLY },
   {"parent",          FILE_PARENT,        JSPROP_ENUMERATE | JSPROP_READONLY },
   {"path",            FILE_PATH,          JSPROP_ENUMERATE | JSPROP_READONLY },
   {"dir",             FILE_DIR,           JSPROP_ENUMERATE | JSPROP_READONLY },
   {"name",            FILE_NAME,          JSPROP_ENUMERATE | JSPROP_READONLY },
   {"isDirectory",     FILE_ISDIR,         JSPROP_ENUMERATE | JSPROP_READONLY },
   {"isFile",          FILE_ISFILE,        JSPROP_ENUMERATE | JSPROP_READONLY },
   {"exists",          FILE_EXISTS,        JSPROP_ENUMERATE | JSPROP_READONLY },
   {"canRead",         FILE_CANREAD,       JSPROP_ENUMERATE | JSPROP_READONLY },
   {"canWrite",        FILE_CANWRITE,      JSPROP_ENUMERATE | JSPROP_READONLY },
   {"canAppend",       FILE_APPEND,        JSPROP_ENUMERATE | JSPROP_READONLY },
   {"canReplace",      FILE_REPLACE,       JSPROP_ENUMERATE | JSPROP_READONLY },
   {"isOpen",          FILE_OPEN,          JSPROP_ENUMERATE | JSPROP_READONLY },
   {"type",            FILE_TYPE,          JSPROP_ENUMERATE | JSPROP_READONLY },
   {"mode",            FILE_MODE,          JSPROP_ENUMERATE | JSPROP_READONLY },
   {"creationTime",    FILE_CREATED,       JSPROP_ENUMERATE | JSPROP_READONLY },
   {"lastModified",    FILE_MODIFIED,      JSPROP_ENUMERATE | JSPROP_READONLY },
   {"size",            FILE_SIZE,          JSPROP_ENUMERATE | JSPROP_READONLY },
   {"hasRandomAccess", FILE_RANDOMACCESS,  JSPROP_ENUMERATE | JSPROP_READONLY },
   {"hasAutoFlush",    FILE_AUTOFLUSH,     JSPROP_ENUMERATE | JSPROP_READONLY },
   {"isLink",    	   FILE_ISLINK,        JSPROP_ENUMERATE | JSPROP_READONLY },
   {"linkTarget",	   FILE_LINKTARGET,    JSPROP_ENUMERATE | JSPROP_READONLY },
   {"sep",	           FILE_SEP,           JSPROP_ENUMERATE | JSPROP_READONLY },
   {"special",         FILE_SPECIAL,       JSPROP_ENUMERATE | JSPROP_READONLY },
   {"position",        FILE_POSITION,      JSPROP_ENUMERATE },
   {0}
};

/* ------------------------- Property getter/setter ------------------------- */
static JSBool
file_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    JSFile      *file = JS_GetInstancePrivate(cx, obj, &file_class, NULL);
    char        *str;
    jsint       tiny;
	JSBool		flag;

    tiny = JSVAL_TO_INT(id);
    if(!file) return JS_TRUE;

    switch (tiny) {
    case FILE_PARENT:
        SECURITY_CHECK(cx, NULL, "parent", file);
        *vp = js_parent(cx, file);
        break;
    case FILE_PATH:
        *vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, file->path));
        break;
	case FILE_DIR:
		str = js_fileDirectoryName(cx, file->path);
        *vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, str));
    	JS_free(cx, str);
        break;
    case FILE_NAME:
        *vp = js_name(cx, file);
        break;
    case FILE_ISDIR:
		file_load_stat( file );
        *vp = BOOLEAN_TO_JSVAL(S_ISDIR(file->stat.st_mode));
        break;
    case FILE_ISLINK:
		file_load_stat( file );
        *vp = BOOLEAN_TO_JSVAL(S_ISLNK(file->stat.st_mode));
        break;
    case FILE_LINKTARGET:
		file_load_stat( file );
		if( S_ISLNK(file->stat.st_mode) )
		{
			char	buf[4096];
			memset( buf, 0, sizeof(buf) );
			readlink( file->path, buf, sizeof(buf) );
        	*vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, buf));
		}
        break;
    case FILE_ISFILE:
		file_load_stat( file );
        *vp = BOOLEAN_TO_JSVAL(S_ISREG(file->stat.st_mode));
        break;
    case FILE_EXISTS:
		file_load_stat( file );
        *vp = BOOLEAN_TO_JSVAL(file->statLoaded);
        break;
    case FILE_OPEN:
        *vp = BOOLEAN_TO_JSVAL(file->isOpen);
        break;
    case FILE_SPECIAL:
        *vp = INT_TO_JSVAL(file->special);
        break;
    case FILE_TYPE:
        if(js_isDirectory(cx, file)){
            *vp = JSVAL_VOID;
            break;
        }

        switch (file->type) {
        case ASCII:
            *vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, asciistring));
            break;
        case UTF8:
            *vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, utfstring));
            break;
        case UCS2:
            *vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, unicodestring));
            break;
        default:
            JS_ReportWarning(cx, "Unsupported file type %d, proceeding",
                file->type);
        }
        break;
    case FILE_MODE:
		file_load_stat( file );
        *vp = INT_TO_JSVAL(file->stat.st_mode);
        break;
    case FILE_CREATED:
		file_load_stat( file );
        JS_NewNumberValue(cx, file->stat.st_ctime, vp);
        break;
        break;
    case FILE_MODIFIED:
		file_load_stat( file );
        JS_NewNumberValue(cx, file->stat.st_mtime, vp);
        break;
    case FILE_SIZE:
		file_load_stat( file );
        *vp = INT_TO_JSVAL(file->stat.st_size);
        break;
    case FILE_LENGTH:
        break;
    case FILE_RANDOMACCESS:
        break;
    case FILE_POSITION:
        break;
    case FILE_SEP:
        {
            static char    sep[] = { FILESEPARATOR, '\0' };
            *vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, sep));
        }
        break;
    default:
		/* this is some other property -- try to use the dir["file"] syntax */
		break;
    }
    return JS_TRUE;
out:
	*vp = JSVAL_VOID;
    return JS_FALSE;
}

static JSBool
file_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    JSFile  *file = JS_GetInstancePrivate(cx, obj, &file_class, NULL);
    jsint   slot;

    if (JSVAL_IS_STRING(id)){
        return JS_TRUE;
    }

    slot = JSVAL_TO_INT(id);

    switch (slot) {
    /* File.position  = 10 */
    case FILE_POSITION:
        SECURITY_CHECK(cx, NULL, "set_position", file);

        if(!file->hasRandomAccess){
            JS_ReportWarning(cx, "File %s doesn't support random access, can't "
                "report the position, proceeding");
            goto out;
        }
    }

    return JS_TRUE;
out:
	*vp = JSVAL_VOID;
	return JS_FALSE;
}

/*
    File.currentDir = new File("D:\") or File.currentDir = "D:\"
*/
static JSBool
file_currentDirSetter(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    JSObject *rhsObject;
    char     *path;
    JSFile   *file = JS_GetInstancePrivate(cx, rhsObject, &file_class, NULL);

    /* Look at the rhs and extract a file object from it */
    if (JSVAL_IS_OBJECT(*vp)){
        if (JS_InstanceOf(cx, rhsObject, &file_class, NULL)){
            /* Braindamaged rhs -- just return the old value */
            if (file && (!js_exists(cx, file) || !js_isDirectory(cx, file))){
                JS_GetProperty(cx, obj, CURRENTDIR_PROPERTY, vp);
                goto out;
            }else{
                rhsObject = JSVAL_TO_OBJECT(*vp);
                chdir(file->path);
                return JS_TRUE;
            }
        }else
            goto out;
    }else{
        path      = JS_GetStringBytes(JS_ValueToString(cx, *vp));
        rhsObject = js_NewFileObject(cx, path);
        if (!rhsObject)  goto out;

        if (!file || !js_exists(cx, file) || !js_isDirectory(cx, file)){
            JS_GetProperty(cx, obj, CURRENTDIR_PROPERTY, vp);
        }else{
            *vp = OBJECT_TO_JSVAL(rhsObject);
            chdir(path);
        }
    }
    return JS_TRUE;
out:
	*vp = JSVAL_VOID;
	return JS_FALSE;
}

/* Declare class */
static JSClass file_class = {
    FILE_CONSTRUCTOR, JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub,  file_getProperty,  file_setProperty,
    JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,   file_finalize
};

/* -------------------- Functions exposed to the outside -------------------- */
JS_PUBLIC_API(JSObject*)
js_InitFileClass(JSContext *cx, JSObject* obj, JSBool initStandardStreams)
{
    JSObject *file, *ctor, *afile;
    jsval    vp;
    char     *currentdir;
    char     separator[2];

    file = JS_InitClass(cx, obj, NULL, &file_class, file_constructor, 1,
        file_props, file_functions, NULL, NULL);
    if (!file) {
        JS_ReportErrorNumber(cx, JSFile_GetErrorMessage, NULL,
            JSFILEMSG_INIT_FAILED);
        return NULL;
    }

    ctor = JS_GetConstructor(cx, file);
    if (!ctor)  return NULL;

	/* Define CURRENTDIR property. We are doing this to get a
	slash at the end of the current dir */
    currentdir =  JS_malloc(cx, MAX_PATH_LENGTH);
    currentdir =  getcwd(currentdir, MAX_PATH_LENGTH);
    afile = js_NewFileObject(cx, currentdir);
    JS_free(cx, currentdir);
    vp = OBJECT_TO_JSVAL(afile);
    JS_DefinePropertyWithTinyId(cx, ctor, CURRENTDIR_PROPERTY, 0, vp,
                JS_PropertyStub, file_currentDirSetter,
                JSPROP_ENUMERATE | JSPROP_READONLY );

    if(initStandardStreams){
        /* Code to create stdin, stdout, and stderr. Insert in the appropriate place. */
    }
    separator[0] = FILESEPARATOR;
    separator[1] = '\0';
    vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, separator));
    JS_DefinePropertyWithTinyId(cx, ctor, SEPARATOR_PROPERTY, 0, vp,
                JS_PropertyStub, JS_PropertyStub,
                JSPROP_ENUMERATE | JSPROP_READONLY );
    return file;
}
