set( LIB_NAME OpenIG-Engine )
set( HEADER_PATH ${PROJECT_SOURCE_DIR}/Core-OpenIG )
add_definitions( -DOPENIG_LIBRARY )

FILE (GLOB SHADER_FILES
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/logz_ps.glsl
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/logz_vs.glsl
)

SOURCE_GROUP("Shaders" FILES ${SHADER_FILES})


set( LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/config.h
    ${HEADER_PATH}/export.h
    ${HEADER_PATH}/keypad.h
    ${HEADER_PATH}/openig.h
	${HEADER_PATH}/renderbins.h
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
	${SHADER_FILES}
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG-Base
    OpenIG-PluginBase
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Core OpenIG" )
set_target_properties (${target} PROPERTIES DEFINE_SYMBOL  "OPENIG_LIBRARY")

include( ModuleInstall REQUIRED )

if(WIN32)
    SET(INSTALL_BINDIR bin)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/logz_ps.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/logz_vs.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/logz_ps.glsl DESTINATION /usr/local/openig/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/logz_vs.glsl DESTINATION /usr/local/openig/resources/shaders)
endif()

