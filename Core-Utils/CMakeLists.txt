set( LIB_NAME OpenIG-Utils )
set( HEADER_PATH ${PROJECT_SOURCE_DIR}/Core-Utils )
add_definitions( -DIGCOREUTILS_LIBRARY )

set( LIB_PUBLIC_HEADERS    
    ${HEADER_PATH}/config.h    
    ${HEADER_PATH}/export.h    
    ${HEADER_PATH}/glerrorutils.h
    ${HEADER_PATH}/tbo.h
    ${HEADER_PATH}/framelogging.h
	${HEADER_PATH}/shaderutils.h
	${HEADER_PATH}/texturecache.h
)

set( _IgCoreUtilsSourceFiles    
    glerrorutils.cpp
	shaderutils.cpp
    tbo.cpp
	texturecache.cpp
)

add_library( ${LIB_NAME} SHARED
        ${LIB_PUBLIC_HEADERS}
        ${_IgCoreUtilsSourceFiles}
)

include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
	${OPENGL_LIBRARY}
	${Boost_LIBRARIES}
	OpenIG-Graphics
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Core Utils" )
set_target_properties (${target} PROPERTIES DEFINE_SYMBOL  "IGCOREUTILS_LIBRARY")

include( ModuleInstall REQUIRED )
