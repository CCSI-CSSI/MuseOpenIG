set( LIB_NAME OpenIG )
set( HEADER_PATH ${PROJECT_SOURCE_DIR}/${LIB_NAME} )
add_definitions( -DOPENIG_LIBRARY )

set( LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/config.h
    ${HEADER_PATH}/export.h
    ${HEADER_PATH}/keypad.h
    ${HEADER_PATH}/openig.h
)

set( _OpenIGSourceFiles
    commands.cpp
    help.cpp
    keypad.cpp
    lights.cpp
    openig.cpp
    splash.cpp
    terminal.cpp
	effects.cpp
)

add_library( ${LIB_NAME} SHARED
    ${LIB_PUBLIC_HEADERS}
    ${_OpenIGSourceFiles}
    openig.xml
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    IgCore
    IgPluginCore
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Core ${LIB_NAME}" )
set_target_properties (${target} PROPERTIES DEFINE_SYMBOL  "OPENIG_LIBRARY")

include( ModuleInstall REQUIRED )

