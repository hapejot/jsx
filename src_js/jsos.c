#include "js.h"
#include <windows.h>
#ifndef BUFSIZE
#define BUFSIZE 500
#endif

/*
 * Define a JS object called "OS".
 */
enum os_tinyid {
    OS_SYSNAME, OS_NODENAME, OS_RELEASE, OS_VERSION, OS_MACHINE
};

static JSPropertySpec os_props[] = {
    {"sysname",         OS_SYSNAME, JSPROP_ENUMERATE},
    {"nodename",        OS_NODENAME,JSPROP_ENUMERATE},
    {"release",         OS_RELEASE, JSPROP_ENUMERATE},
    {"version",         OS_VERSION, JSPROP_ENUMERATE},
    {"machine",         OS_MACHINE, JSPROP_ENUMERATE},
    {0}
};

static JSString *  os_sysname       = NULL;
static JSString *  os_nodename      = NULL;
static JSString *  os_release       = NULL;
static JSString *  os_version       = NULL;
static JSString *  os_machine       = NULL;

static JSBool
os_item(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
    *rval = OBJECT_TO_JSVAL(obj);
    if (argc != 0)
        JS_SetCallReturnValue2(cx, argv[0]);
    return JS_TRUE;
}

static JSFunctionSpec os_methods[] = {
    {"item",            os_item,       0},
    {0}
};

static JSBool os_noisy;    /* whether to be noisy when finalizing it */

static JSBool
os_addProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    if (os_noisy) {
        fprintf(gOutFile, "adding its property %s,",
               JS_GetStringBytes(JS_ValueToString(cx, id)));
        fprintf(gOutFile, " initial value %s\n",
               JS_GetStringBytes(JS_ValueToString(cx, *vp)));
    }
    return JS_TRUE;
}

static JSBool
os_delProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    if (os_noisy) {
        fprintf(gOutFile, "deleting its property %s,",
               JS_GetStringBytes(JS_ValueToString(cx, id)));
        fprintf(gOutFile, " current value %s\n",
               JS_GetStringBytes(JS_ValueToString(cx, *vp)));
    }
    return JS_TRUE;
}

static JSBool
os_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    switch( JSVAL_TO_INT(id) )
    {
    case OS_SYSNAME:
        *vp = STRING_TO_JSVAL(os_sysname);
        break;
    case OS_NODENAME:
        *vp = STRING_TO_JSVAL(os_nodename);
        break;
    case OS_RELEASE:
        *vp = STRING_TO_JSVAL(os_release);
        break;
    case OS_VERSION:
        *vp = STRING_TO_JSVAL(os_version);
        break;
    case OS_MACHINE:
        *vp = STRING_TO_JSVAL(os_machine);
        break;
    }
    return JS_TRUE;
}

static JSBool
os_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    return JS_FALSE;
}

static JSBool
os_enumerate(JSContext *cx, JSObject *obj)
{
    return JS_TRUE;
}

static JSBool
os_resolve(JSContext *cx, JSObject *obj, jsval id, uintN flags,
            JSObject **objp)
{
    return JS_TRUE;
}

static JSBool
os_convert(JSContext *cx, JSObject *obj, JSType type, jsval *vp)
{
    return JS_TRUE;
}

static void
os_finalize(JSContext *cx, JSObject *obj)
{
}




static JSClass os_class = {
    "OS", JSCLASS_NEW_RESOLVE,
    os_addProperty,  os_delProperty,  os_getProperty,  os_setProperty,
    os_enumerate,    (JSResolveOp)os_resolve,
    os_convert,      os_finalize
};

static JSBool       os_constructor 
(
    JSContext*      p_cx, 
    JSObject*       p_obj, 
    uintN           p_argc, 
    jsval*          p_argv, 
    jsval*          p_result)
{
    return JS_TRUE;
}


JSBool
jsos_init_class(
    JSContext*      cx,
    JSObject*       glob)
{
    JSObject*       it;
    
    it = JS_InitClass( cx, glob, NULL,
            &os_class,
            os_constructor, 0,
            os_props,
            os_methods,
            NULL,
            NULL );

    return JS_TRUE;
}


static void set_version_and_release(
    JSContext*      cx)
{
    OSVERSIONINFOEX     osvi;
    BOOL                bOsVersionInfoEx;
    char                version[300]    = "";

    // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
    // If that fails, try using the OSVERSIONINFO structure.

    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
    {
        osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
            return;
    }

    switch (osvi.dwPlatformId)
    {
        // Test for the Windows NT product family.
        case VER_PLATFORM_WIN32_NT:

         // Test for the specific product family.
            if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
                sprintf( version, "Microsoft Windows Server 2003 family, ");

             if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
                sprintf( version, "Microsoft Windows XP ");

            if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
                sprintf( version, "Microsoft Windows 2000 ");

            if ( osvi.dwMajorVersion <= 4 )
                sprintf( version, "Microsoft Windows NT ");

             // Test for specific product on Windows NT 4.0 SP6 and later.
            if( bOsVersionInfoEx )
            {
                // Test for the workstation type.
                if ( osvi.wProductType == VER_NT_WORKSTATION )
                {
                   if( osvi.dwMajorVersion == 4 )
                      sprintf( version+strlen(version), "Workstation 4.0 " );
                   else if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
                      sprintf( version+strlen(version), "Home Edition " );
                   else
                      sprintf( version+strlen(version), "Professional " );
                }
                
                // Test for the server type.
                else if ( osvi.wProductType == VER_NT_SERVER )
                {
                    if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
                    {
                        if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                            sprintf ( version+strlen(version), "Datacenter Edition " );
                        else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                            sprintf ( version+strlen(version), "Enterprise Edition " );
                        else if ( osvi.wSuiteMask == VER_SUITE_BLADE )
                            sprintf ( version+strlen(version), "Web Edition " );
                        else
                            sprintf ( version+strlen(version), "Standard Edition " );
                    }
                    else if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
                    {
                        if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                            sprintf ( version+strlen(version), "Datacenter Server " );
                        else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                            sprintf ( version+strlen(version), "Advanced Server " );
                        else
                            sprintf ( version+strlen(version), "Server " );
                    }
                    else  // Windows NT 4.0 
                    {
                        if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                            sprintf( version+strlen(version), "Server 4.0, Enterprise Edition " );
                        else
                            sprintf( version+strlen(version), "Server 4.0 " );
                   }
                }
            }
            else  // Test for specific product on Windows NT 4.0 SP5 and earlier
            {
                HKEY hKey;
                char szProductType[BUFSIZE];
                DWORD dwBufLen=BUFSIZE;
                LONG lRet;

                lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            "SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
                            0, KEY_QUERY_VALUE, &hKey );
                if( lRet != ERROR_SUCCESS )
                            return;

                lRet = RegQueryValueEx( hKey, "ProductType", NULL, NULL,
                            (LPBYTE) szProductType, &dwBufLen);
                if( (lRet != ERROR_SUCCESS) || (dwBufLen > BUFSIZE) )
                            return;

                RegCloseKey( hKey );

                if ( lstrcmpi( "WINNT", szProductType) == 0 )
                    sprintf( version+strlen(version), "Workstation " );
                if ( lstrcmpi( "LANMANNT", szProductType) == 0 )
                    sprintf( version+strlen(version), "Server " );
                if ( lstrcmpi( "SERVERNT", szProductType) == 0 )
                    sprintf( version+strlen(version), "Advanced Server " );

                sprintf( version+strlen(version), "%d.%d ", osvi.dwMajorVersion, osvi.dwMinorVersion );
            }

            // Display service pack (if any) and build number.

            if( osvi.dwMajorVersion == 4 && 
                lstrcmpi( osvi.szCSDVersion, "Service Pack 6" ) == 0 )
            {
                HKEY hKey;
                LONG lRet;

                // Test for SP6 versus SP6a.
                lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009",
                            0, KEY_QUERY_VALUE, &hKey );
                if( lRet == ERROR_SUCCESS )
                            sprintf( version+strlen(version), "Service Pack 6a (Build %d)", osvi.dwBuildNumber & 0xFFFF );         
                else // Windows NT 4.0 prior to SP6a
                {
                    sprintf( version+strlen(version), "%s (Build %d)", osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
                }

                RegCloseKey( hKey );
            }
            else // Windows NT 3.51 and earlier or Windows 2000 and later
            {
                sprintf( version+strlen(version), "%s (Build %d)",
                            osvi.szCSDVersion,
                            osvi.dwBuildNumber & 0xFFFF);
            }
            break;

        // Test for the Windows 95 product family.
        case VER_PLATFORM_WIN32_WINDOWS:

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
            {
                sprintf (version+strlen(version), "Microsoft Windows 95 ");
                if ( osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B' )
                    sprintf(version+strlen(version), "OSR2 " );
            } 

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
            {
                sprintf (version+strlen(version), "Microsoft Windows 98 ");
                if ( osvi.szCSDVersion[1] == 'A' )
                    sprintf(version+strlen(version), "SE " );
            } 

            if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
            {
                sprintf (version+strlen(version), "Microsoft Windows Millennium Edition");
            } 
            break;

        case VER_PLATFORM_WIN32s:
            sprintf ( version, "Microsoft Win32s");
            break;
    }

    os_release = JS_NewStringCopyZ( cx, version );
    sprintf( version, "%d.%d.%d", 
                            osvi.dwMajorVersion,
                            osvi.dwMinorVersion,
                            osvi.dwBuildNumber & 0xFFFF);
    os_version = JS_NewStringCopyZ( cx, version );
}

set_nodename( 
    JSContext*      cx )
{
    char            name[200];
    int             n           = sizeof(name);

    GetComputerName( name, &n );

    os_nodename = JS_NewStringCopyN( cx, name, n );
}

JSBool
jsos_initialize(
    JSContext*      cx,
    JSObject*       glob)
{
    JSObject*       it;
    JSBool          status          = JS_TRUE;

    set_version_and_release( cx );
    set_nodename( cx );
    os_sysname = JS_NewStringCopyZ( cx, "win32" );
    os_machine = JS_NewStringCopyZ( cx, "i386" );

    it = JS_DefineObject(cx, glob, "OS", &os_class, NULL, 0);
    if (!it)
        status = JS_FALSE;
    if (status && !JS_DefineProperties(cx, it, os_props))
        status = JS_FALSE;
    if (status && !JS_DefineFunctions(cx, it, os_methods))
        status = JS_FALSE;
    return status;
}
