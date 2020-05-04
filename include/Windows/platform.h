#ifndef _PLATFORM_H
#define _PLATFORM_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <wspiapi.h>
#include <windows.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <io.h>
#include <signal.h>
#include <sys/stat.h>

// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif



#define PLATFORM_IS_LINUX   0
#define PLATFORM_IS_UNIX    0
#define PLATFORM_IS_WIN32   1
#define PLATFORM_IS_SOLARIS 0

#define SHUT_RDWR           SD_BOTH

#define STDIN_FILENO        0
#define STDOUT_FILENO       1

typedef INT32               INT;


#define SCCSID(file,id) static char sccsid##file[] = ("@(#) (c) PJA " BUILDNO " "  id )
#define RC_OK       0
#define RC_ERROR    -1
#define ENDSTR      '\0'

#define IN
#define OUT
#define INOUT

#define FILESEPARATOR   '\\'
#define FILESEPARATOR2  ':'
#define PATHSEP         ';'
#define CURRENT_DIR     "C:\\"

#define S_ISREG(m) (m & _S_IFREG)
#define S_ISLNK(m)  0
#define S_ISDIR(m) (m & _S_IFDIR)
#define S_IXUSR 0777777
#define S_IXGRP 0777777
#define S_IXOTH 0777777
#define PATH_MAX    1024
#define R_OK        4
#define lstat stat
#define readlink(a,b,c)

#define JSXDIRENT   WIN32_FIND_DATA
#define JSXDIR      HANDLE

static JSXDIR jsx_find_first_file( char* name, JSXDIRENT* ent )
{
    char    path[MAX_PATH];
    strcpy( path, name );
    strcat( path, "\\*.*");
    return FindFirstFile(path, ent);
}

static int jsx_find_next_file( JSXDIR dir, JSXDIRENT* ent )
{
    return FindNextFile(dir, ent);
}

static char* jsx_get_filename( JSXDIRENT* ent )
{
    return ent->cFileName;
}

static void jsx_find_close( JSXDIR dir )
{
    FindClose(dir);
}


#pragma warning (disable : 4201)


#define SOCKET_INIT  \
{ \
    WSADATA             wsadata; \
    WSAStartup( 0x0101, &wsadata ); \
}

#define SOCKET_ERROR_REPORT() \
{ \
    fprintf(stderr, "%s(%d):socket error: %d\n", __FILE__, __LINE__, WSAGetLastError()); \
}

#define XP_WIN
#define _X86    1

#endif
