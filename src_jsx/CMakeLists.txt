cmake_minimum_required( VERSION 2.8 )

## Required software
#find_package( <<<name>>> REQUIRED )

## Sources and headers
### include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( 	../include 
			../include/${CMAKE_SYSTEM_NAME} )


set( CMAKE_C_FLAGS "-m32 -DBUILDNO=${BUILDNO}" )
##set( CMAKE_C_FLAGS_DEBUG "" )
##set( CMAKE_C_FLAGS_RELEASE "" )

## Source file containing the "main" function
set( MAIN_SOURCES 
            main.c
            jsinit.c 
            jshell_process.c
            jshell_print.c
            jshell_system.c
            jshell_error_reporter.c
        )
## Specify a name for the generated executable file
set( MAIN_EXE_NAME jsx )


## 1. Compile...

add_executable( ${MAIN_EXE_NAME} ${MAIN_SOURCES}                                 
              )

#if (LINUX)
set (MATH_LIB m)
#endif (LINUX)

## 2. Link...
target_link_libraries( ${MAIN_EXE_NAME} PUBLIC js ${MATH_LIB})

## 3. Install...
install( TARGETS ${MAIN_EXE_NAME}
         RUNTIME DESTINATION bin )

