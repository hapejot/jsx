cmake_minimum_required( VERSION 2.8 )

## Required software
#find_package( <<<name>>> REQUIRED )

## Sources and headers
### include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ../include )

## Find all source files to be compiled into object files
## file( GLOB SOURCES *.c )
set( SOURCES   
                jsapi.c
                jsnet.c
##              jsits.c
                jsscript.c
                jsarray.c
                jsbool.c
                jsobj.c
                jsstr.c
                jsregexp.c
                jsdate.c
                jsexn.c
                jsfun.c
                jsmath.c
                jsnum.c
                jsarena.c
                jsdhash.c
                jsprf.c
                jsatom.c
                jsgc.c
                jsinterp.c
                jscntxt.c
                jsdtoa.c
                jsopcode.c       
                jsemit.c   
                jsscan.c
                jsparse.c
                jsscope.c
                jsfile.c
                jshash.c
                jsdbgapi.c
                jsxdrapi.c
                jslog2.c
                jslong.c
                prmjtime.c
)


##set( CMAKE_C_FLAGS "-DBUILDNO=${BUILDNO} -DJS_ARENAMETER" )
set( CMAKE_C_FLAGS "-DBUILDNO=${BUILDNO}" )
##set( CMAKE_C_FLAGS_DEBUG "-DDEBUG" )
##set( CMAKE_C_FLAGS_RELEASE "" )



## 1. Compile...

add_library(js SHARED ${SOURCES} )
target_compile_definitions(js PUBLIC EXPORT_JS_API)
target_link_libraries( js PUBLIC Ws2_32 fdlibm )

