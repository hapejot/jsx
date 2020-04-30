#include "platform.h"
#include "js.h"

#define JSNET_LOCALSERVICE      0
#define JSNET_LOCALADDRESS      1
#define JSNET_LOCALHOST         2
#define JSNET_REMOTESERVICE     3
#define JSNET_REMOTEADDRESS     4
#define JSNET_REMOTEHOST        5
#define JSNET_TYPE              6
#define JSNET_BLOCKING          7

static JSClass      socket_class;

struct socket {
    int             is_tcp;
    int             is_server;
    unsigned char*  buffer;
    jsuint          bufsize;
    jsuint          len;
    void *          handle;
    char *          lservice;
    char *          lhost;
    struct sockaddr lhost_addr;
    size_t          lhost_addrlen;
    char *          rservice;
    char *          rhost;
    struct sockaddr rhost_addr;
    size_t          rhost_addrlen;
};

static JSBool       socket_resolve_host(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    int             p_which     /* remote == 0, or local != 0 */
    )
{
    JSBool          status  = JS_TRUE;
    struct socket * sock    = JS_GetInstancePrivate(p_cx, p_obj, &socket_class, NULL);
    struct addrinfo hints;
    struct addrinfo*res     = NULL;
    char*           host;
    char*           service;

    /* try to decode an address of the aaa.bbb.ccc.ddd form. */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
                           
    if( p_which )
    {
        host = sock->lhost;
        service = sock->lservice;
    }
    else
    {
        host = sock->rhost;
        service = sock->rservice;
    }
    status = getaddrinfo( host, service, &hints, &res );
    if( status )
    {
        fprintf( stderr, "getaddrinfo(%s,%s) returned %d\n", host, service, status );
        SOCKET_ERROR_REPORT();
        status = JS_TRUE;
    }
    else
        status = JS_TRUE;
    if( status )
    {
        char        hostname[NI_MAXHOST]        = "";
        char        servicename[NI_MAXSERV]     = "";

        if( p_which )
        {
            memcpy(&sock->lhost_addr, res->ai_addr, res->ai_addrlen );
            sock->lhost_addrlen = res->ai_addrlen;
        }
        else
        {
            memcpy(&sock->rhost_addr, res->ai_addr, res->ai_addrlen );
            sock->rhost_addrlen = res->ai_addrlen;
        }

        status = getnameinfo( 
                res->ai_addr, res->ai_addrlen, 
                hostname, sizeof(hostname),
                servicename, sizeof(servicename), NI_NUMERICHOST| NI_NUMERICSERV  );
    }
    return status;
}

static JSBool       socket_open         
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result)
{
    JSBool          status  = JS_TRUE;
    struct socket * sock;
    struct sockaddr_in   
                    sa;
    struct hostent* hp;
    int             addr_len;
    unsigned long   ip_addr;

    sock = JS_GetInstancePrivate(p_cx, p_obj, &socket_class, NULL);


    if( sock->is_tcp )
    {
        socket_resolve_host( p_cx, p_obj, 0 );
        sock->handle = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
        if( sock->is_server )
        {
            int     on      = 1;
            setsockopt(sock->handle, SOL_SOCKET, SO_REUSEADDR, (char *) & on, sizeof(on));
            status = (0 == bind( sock->handle, &sock->rhost_addr, sock->rhost_addrlen ));
            status = listen( sock->handle, 10 );
            fprintf( stderr, "listen(%p)->%d\n", sock->handle, status );
            if( status == 0 )
                status = JS_TRUE;
        }
        else
            status = (0 == connect( sock->handle, &sock->rhost_addr, sock->rhost_addrlen ));
    }
    else
    {
        socket_resolve_host( p_cx, p_obj, 0 );
        socket_resolve_host( p_cx, p_obj, 1 );
        sock->handle = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    }
    if( !status )
        SOCKET_ERROR_REPORT();
    return status;
}

/*
 * socket_close
 *      Close the socket, if connected. The socket should also being closed
 *      during finalizing the object.
 */
static JSBool       socket_close        
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result)
{
    JSBool          status  = JS_TRUE;
    struct socket * sock    = JS_GetInstancePrivate(p_cx, p_obj, &socket_class, NULL);

    shutdown( sock->handle, SHUT_RDWR );
    sock->handle = -1;

    return status;
}


static JSBool       socket_read
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result)
{
    JSBool          status  = JS_TRUE;
    struct socket * sock    = JS_GetInstancePrivate(p_cx, p_obj, &socket_class, NULL);
    extern JSClass  js_ArrayClass;
    jsuint          n;
    jsuint          i;
    jsval           v;
    JSObject*       a;
    int             k;

    n = JSVAL_TO_INT( p_argv[0] );
    if( n > sock->bufsize )
    {
        sock->bufsize = n;
        sock->buffer = JS_realloc( p_cx, sock->buffer, sock->bufsize );
    }
    if( sock->is_tcp )
    {
        k = recv( sock->handle, sock->buffer, n, 0 );
#ifdef WIN32
        if( k == INVALID_SOCKET 
                && WSAGetLastError() != WSAEWOULDBLOCK )
        {
            SOCKET_ERROR_REPORT();
            status = JS_FALSE;
        }
#endif
    }
    else
    {
        int     len = sock->rhost_addrlen;
        int     err;

        k = recvfrom( sock->handle, sock->buffer, n, 0,
                &sock->rhost_addr, &len  );
        if( k < 0 )
        {
#ifdef WIN32
            if( WSAGetLastError() != WSAEWOULDBLOCK )
            {
                SOCKET_ERROR_REPORT();
                status = JS_FALSE;
            }
            else
#endif
                *p_result = NULL;
        }
    }
    if( k >= 0 )
    {
        a = js_ConstructObject( p_cx, &js_ArrayClass, NULL, NULL, 0, NULL);
        for( i = 0; i < k; i++ )
        {
            v = INT_TO_JSVAL(sock->buffer[i]);
            JS_SetElement( p_cx, a, i, &v );
        }
        *p_result = OBJECT_TO_JSVAL(a);
    }

    return status;
}

static JSBool       socket_accept
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result)
{
    JSBool          status  = JS_TRUE;
    struct socket * sock    = JS_GetInstancePrivate(
                                    p_cx, p_obj, 
                                    &socket_class, NULL);
    jsuint          n;
    jsuint          i;
    jsval           v;
    JSObject*       a;
    int             k;

    if( sock->is_tcp && sock->is_server )
    {
        struct socket *     csock;
        struct sockaddr     addr;
        socklen_t           addrlen     = sizeof(addr);
        void *              k;

        k = accept( sock->handle, &addr, &addrlen );
#ifdef WIN32
        if( k == INVALID_SOCKET  
                && WSAGetLastError() != WSAEWOULDBLOCK )
        {
            SOCKET_ERROR_REPORT();
            status = JS_FALSE;
        }
#endif
        if( (int)k >= 0 )
        {
            fprintf( stderr, "accept(%p) -> %p\n", sock->handle, k );
            a = js_ConstructObject( p_cx, &socket_class, NULL, NULL, 0, NULL);
            csock = JS_GetInstancePrivate( p_cx, a, &socket_class, NULL );
            csock->rhost_addr = addr;
            csock->rhost_addrlen = addrlen;
            csock->handle = k;
            *p_result = OBJECT_TO_JSVAL(a);
        }
        else
        {
            status = JS_FALSE;
            switch( errno )
            {
                case EINVAL:
                    fprintf( stderr, "accept(%p) without previous listen call.\n", sock->handle );
                    break;
                default:
                    perror("accept");
            }
        }
    }

    return status;
}

static JSBool       socket_write
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result)
{
    JSBool          status  = JS_TRUE;
    struct socket * sock    = JS_GetInstancePrivate(p_cx, p_obj, &socket_class, NULL);
    int             j;
    jsuint          k;
    int             total   = 0;


    for( j = 0; j < p_argc; j++ )
    {
        if( JSVAL_IS_OBJECT(p_argv[j]) 
                && JS_IsArrayObject(p_cx, JSVAL_TO_OBJECT(p_argv[j])) )
        {
            jsuint      n;
            jsuint      i;
            jsval       v;

            JS_GetArrayLength( p_cx, JSVAL_TO_OBJECT(p_argv[0]), &n );
            if( n > sock->bufsize )
            {
                sock->bufsize = n;
                sock->buffer = JS_realloc( p_cx, sock->buffer, sock->bufsize );
            }
            for( i = 0; i < n; i++ )
            {
                JS_GetElement( p_cx, JSVAL_TO_OBJECT( p_argv[0]), i, &v );
                sock->buffer[i] = JSVAL_TO_INT(v);
            }
            if( sock->is_tcp )
                k = send( sock->handle, sock->buffer, n, 0 );
            else
            {
                k = sendto( sock->handle, sock->buffer, n, 0, 
                            &sock->rhost_addr, sock->rhost_addrlen );
            }
            if( k >= 0 )
                total += k;
        }
        else if( JSVAL_IS_STRING(p_argv[j]) )
        {
            char *      buf = JS_GetStringBytes(JSVAL_TO_STRING(p_argv[j]));
            jsuint      n = strlen(buf);

            if( sock->is_tcp )
                k = send( sock->handle, buf, n, 0 );
            else
            {
                k = sendto( sock->handle, buf, n, 0, 
                            &sock->rhost_addr, sock->rhost_addrlen );
            }
            if( k >= 0 )
                total += k;
        }
    }

    if( status == JS_TRUE )
        *p_result = INT_TO_JSVAL(total);

    return status;
}

static JSBool       socket_constructor 
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result)
{
    struct socket * sock;
    JSString *      str;

    sock            = JS_malloc( p_cx, sizeof(struct socket) );
    memset( sock, 0, sizeof(struct socket) );
    sock->is_tcp    = 1;
    sock->is_server = 0;
    sock->len       = 0;
    sock->bufsize   = 4096;
    sock->buffer    = JS_malloc( p_cx, sock->bufsize );

    JS_SetPrivate( p_cx, p_obj, sock );

    if( p_argc > 0 )
        sock->rhost = JS_GetStringBytes(JS_ValueToString( p_cx, p_argv[0]) );
    if( p_argc > 1 )
        sock->rservice = JS_GetStringBytes(JS_ValueToString( p_cx, p_argv[1]) );

    return JS_TRUE;
}

static JSBool       socket_get_property
(
    JSContext *     p_cx, 
    JSObject *      p_obj, 
    jsval           p_id, 
    jsval *         p_vp)
{
    JSBool          status  = JS_TRUE;
    jsint           id;
    struct socket * sock    = JS_GetInstancePrivate(p_cx, p_obj, &socket_class, NULL);
    id = JSVAL_TO_INT(p_id);
    switch( id )
    {
    case JSNET_LOCALSERVICE:
        *p_vp = STRING_TO_JSVAL(JS_NewStringCopyZ(p_cx, sock->lservice));
        break;
    case JSNET_LOCALHOST:
        *p_vp = STRING_TO_JSVAL(JS_NewStringCopyZ(p_cx, sock->lhost));
        break;

    case JSNET_REMOTESERVICE:
        *p_vp = STRING_TO_JSVAL(JS_NewStringCopyZ(p_cx, sock->rservice));
        break;
    case JSNET_REMOTEHOST:
        *p_vp = STRING_TO_JSVAL(JS_NewStringCopyZ(p_cx, sock->rhost));
        break;

    case JSNET_TYPE: // type
        if( sock->is_tcp )
            if( sock->is_server )
                *p_vp = STRING_TO_JSVAL(JS_NewStringCopyZ(p_cx, "tcpserver"));
            else
                *p_vp = STRING_TO_JSVAL(JS_NewStringCopyZ(p_cx, "tcp"));
        else
            *p_vp = STRING_TO_JSVAL(JS_NewStringCopyZ(p_cx, "udp"));
        break;
    }
    return status;
}

static JSBool       socket_set_property
(
    JSContext *     p_cx, 
    JSObject *      p_obj, 
    jsval           p_id, 
    jsval *         p_vp)
{
    JSBool          status  = JS_TRUE;
    struct socket * sock    = JS_GetInstancePrivate(p_cx, p_obj, &socket_class, NULL);
    int             id;

    id = JSVAL_TO_INT( p_id );
    switch( id )
    {
    case JSNET_LOCALSERVICE:
        sock->lservice = JS_GetStringBytes(JS_ValueToString(p_cx, *p_vp) );
        break;
    case JSNET_LOCALADDRESS:
    case JSNET_LOCALHOST:
        sock->lhost = JS_GetStringBytes(JS_ValueToString(p_cx, *p_vp) );
        break;
    case JSNET_REMOTESERVICE:
        sock->rservice = JS_GetStringBytes(JS_ValueToString(p_cx, *p_vp) );
        break;
    case JSNET_REMOTEADDRESS:
    case JSNET_REMOTEHOST:
        sock->rhost = JS_GetStringBytes(JS_ValueToString(p_cx, *p_vp) );
        break;
    case JSNET_TYPE:
        if( JSVAL_IS_STRING(*p_vp) )
        {
            char *  str;

            str = JS_GetStringBytes(JSVAL_TO_STRING(*p_vp));
            if( strcmp(str, "udp") == 0 )
                sock->is_tcp = 0;
            else if( strcmp(str, "tcpserver") == 0 )
            {
                sock->is_tcp = 1;
                sock->is_server = 1;
            }
        }
        else
            status = JS_FALSE;
        break;
    case JSNET_BLOCKING:
        {
            JSBool  f;
            u_long  nonblocking;

            JS_ValueToBoolean( p_cx, *p_vp, &f);
            nonblocking = f ? 0 : 1;
#ifdef WIN32
            ioctlsocket( sock->handle, FIONBIO, &nonblocking );
#else
            ioctl( sock->handle, FIONBIO, &nonblocking );
#endif
        }
        break;
    }
    return status;
}

static void
socket_finalize
(
    JSContext *     p_cx,
    JSObject *      p_obj)
{
//  struct socket * sock    = JS_GetInstancePrivate(p_cx, p_obj, &socket_class, NULL);
    // JS_free( p_cx, sock );
}


static JSPropertySpec socket_props[] = {
    {"localService",    JSNET_LOCALSERVICE,     JSPROP_ENUMERATE},
    {"localAddress",    JSNET_LOCALADDRESS,     JSPROP_ENUMERATE},
    {"localHost",       JSNET_LOCALHOST,        JSPROP_ENUMERATE},
    {"remoteService",   JSNET_REMOTESERVICE,    JSPROP_ENUMERATE},
    {"remoteAddress",   JSNET_REMOTEADDRESS,    JSPROP_ENUMERATE},
    {"remoteHost",      JSNET_REMOTEHOST,       JSPROP_ENUMERATE},
    {"type",            JSNET_TYPE,             JSPROP_ENUMERATE},
    {"blocking",        JSNET_BLOCKING,         JSPROP_ENUMERATE},
    {0}
};


static JSFunctionSpec socket_methods[] = {
    {"open",        socket_open,    0},
    {"close",       socket_close,   0},
    {"read",        socket_read,    0},
    {"write",       socket_write,   0},
    {"accept",      socket_accept,  0},
    {0}
};

JSBool
net_property_stub(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    return JS_TRUE;
}

JSBool
net_enumerate_stub(JSContext *cx, JSObject *obj)
{
    return JS_TRUE;
}

JSBool
net_resolve_stub(JSContext *cx, JSObject *obj, jsval id)
{
    return JS_TRUE;
}

JSBool
net_convert_stub(JSContext *cx, JSObject *obj, JSType type, jsval *vp)
{
    return js_TryValueOf(cx, obj, type, vp);
} 


static JSClass socket_class = {
    "Socket", 
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub,
    socket_get_property,  
    socket_set_property,
    JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,
    socket_finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};


JSBool js_net_initialize(
    JSContext *         p_cx,
    JSObject *          p_obj
)
{
    JSObject *          this;

#ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif

    JS_InitClass( p_cx, p_obj, NULL, 
            &socket_class, 
            socket_constructor, 0, 
            socket_props, 
            socket_methods,
            NULL,
            NULL );

    SOCKET_INIT;
}
