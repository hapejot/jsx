#ifndef _PLATFORM_H
#define _PLATFORM_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <locale.h>
#include <dirent.h>



#define PLATFORM_IS_LINUX   1
#define PLATFORM_IS_UNIX    1
#define PLATFORM_IS_WIN32   0
#define PLATFORM_IS_SOLARIS 0

#define O_BINARY            0

typedef long long           INT64;
typedef unsigned long long  UINT64;
typedef long                INT32;
typedef unsigned long       UINT32;
typedef short               INT16;
typedef unsigned short      UINT16;
typedef char                INT8;
typedef unsigned char       UINT8;
typedef unsigned char       CHAR;

typedef INT32               INT;
typedef UINT32              UINT;

#define SCCSID(file,id) static char sccsid##file[] = ("@(#) (c) PJA " BUILDNO " "  id )
#define OK          0
#define RC_OK       0
#define ERROR       -1
#define RC_ERROR    -1
#define ENDSTR      '\0'

#define IN 
#define OUT
#define INOUT

#define FILESEPARATOR   '/'
#define PATHSEP         ':'
#define FILESEPARATOR2  '\0'
#define CURRENT_DIR     "/"


#define SOCKET_INIT  

#define SOCKET_ERROR_REPORT() \
{ \
    fprintf(stderr, "%s(%d):socket error: %d\n", __FILE__, __LINE__, errno ); \
}



#define JSXDIRENT	struct dirent
#define JSXDIR		DIR*

static JSXDIR jsx_find_first_file( char* name, JSXDIRENT* ent )
{
	JSXDIR  d = opendir( name );
	JSXDIRENT* r;
	readdir_r( d, ent, &r);
	return d;
}

static int jsx_find_next_file( JSXDIR dir, JSXDIRENT* ent )
{
	JSXDIRENT* r;
	readdir_r( dir, ent, &r);
	return (r != NULL);
}

static char* jsx_get_filename( JSXDIRENT* ent )
{
	return ent->d_name;
}

static void jsx_find_close( JSXDIR dir )
{
	closedir(dir);
}

#define VA_COPY va_copy

#define XP_UNIX
#define _X86    1

#endif
