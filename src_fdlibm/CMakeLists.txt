

file( GLOB SOURCES *.c )
set( CMAKE_C_FLAGS "-D_IEEE_LIBM" )
add_library(fdlibm STATIC ${SOURCES} )
target_compile_definitions(fdlibm PUBLIC EXPORT)
## set_target_properties(fdlibm PROPERTIES LINK_FLAGS "/NODEFAULTLIB")

install( TARGETS fdlibm
         RUNTIME DESTINATION bin )

