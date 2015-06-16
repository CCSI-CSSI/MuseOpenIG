set( LIB_NAME IgCore )
set( HEADER_PATH ${PROJECT_SOURCE_DIR}/${LIB_NAME} )
add_definitions( -DIGCORE_LIBRARY )

set( LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/animation.h
    ${HEADER_PATH}/attributes.h
    ${HEADER_PATH}/commands.h
    ${HEADER_PATH}/config.h
    ${HEADER_PATH}/configuration.h
    ${HEADER_PATH}/export.h
    ${HEADER_PATH}/globalidgenerator.h
    ${HEADER_PATH}/igcore.h
    ${HEADER_PATH}/imagegenerator.h
    ${HEADER_PATH}/mathematics.h
    ${HEADER_PATH}/stringutils.h
)

set( _IgCoreSourceFiles
    animation.cpp
    commands.cpp
    configuration.cpp
    globalidgenerator.cpp
    imagegenerator.cpp
    mathematics.cpp
    stringutils.cpp
)

add_library( ${LIB_NAME} SHARED
    ${LIB_PUBLIC_HEADERS}
    ${_IgCoreSourceFiles}
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Core ${LIB_NAME}" )
set_target_properties (${target} PROPERTIES DEFINE_SYMBOL  "IGCORE_LIBRARY")

include( ModuleInstall REQUIRED )
