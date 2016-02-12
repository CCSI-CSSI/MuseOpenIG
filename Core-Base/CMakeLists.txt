set( LIB_NAME OpenIG-Base )
set( HEADER_PATH ${PROJECT_SOURCE_DIR}/Core-Base )
add_definitions( -DIGCORE_LIBRARY )

set( LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/animation.h
    ${HEADER_PATH}/attributes.h
    ${HEADER_PATH}/commands.h
    ${HEADER_PATH}/config.h
    ${HEADER_PATH}/configuration.h
    ${HEADER_PATH}/export.h
    ${HEADER_PATH}/filesystem.h
    ${HEADER_PATH}/globalidgenerator.h
    ${HEADER_PATH}/igcore.h
    ${HEADER_PATH}/imagegenerator.h
    ${HEADER_PATH}/mathematics.h
    ${HEADER_PATH}/stringutils.h
    ${HEADER_PATH}/glerrorutils.h
    ${HEADER_PATH}/tbo.h
    ${HEADER_PATH}/framelogging.h
	${HEADER_PATH}/shaderutils.h
	${HEADER_PATH}/texturecache.h
)

set( _IgCoreSourceFiles
    animation.cpp
    commands.cpp
    configuration.cpp
    filesystem.cpp
    globalidgenerator.cpp
    imagegenerator.cpp
    mathematics.cpp
    stringutils.cpp
    glerrorutils.cpp
	shaderutils.cpp
    tbo.cpp
	texturecache.cpp
)

add_library( ${LIB_NAME} SHARED
        ${LIB_PUBLIC_HEADERS}
        ${_IgCoreSourceFiles}
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
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Core Base" )
set_target_properties (${target} PROPERTIES DEFINE_SYMBOL  "IGCORE_LIBRARY")

include( ModuleInstall REQUIRED )

INSTALL(
    FILES       ${PROJECT_SOURCE_DIR}/SDK-Readme.txt
    DESTINATION doc/
    COMPONENT libopenig-dev
)