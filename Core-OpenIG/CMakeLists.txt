SET( LIB_NAME OpenIG-Engine )
SET( HEADER_PATH ${PROJECT_SOURCE_DIR}/Core-OpenIG )
ADD_DEFINITIONS( -DOPENIG_LIBRARY )

FILE (GLOB SHADER_FILES
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/logz_ps.glsl
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/logz_vs.glsl
)

SOURCE_GROUP("Shaders" FILES ${SHADER_FILES})


SET( LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/Config.h
    ${HEADER_PATH}/Export.h
    ${HEADER_PATH}/Keypad.h
    ${HEADER_PATH}/Engine.h
	${HEADER_PATH}/RenderBins.h
)

SET( _OpenIGSourceFiles
    Commands.cpp
    Help.cpp
    Keypad.cpp
    Lights.cpp
    Engine.cpp
    Splash.cpp
    Terminal.cpp
	Effects.cpp
)

ADD_LIBRARY( ${LIB_NAME} SHARED
    ${LIB_PUBLIC_HEADERS}
    ${_OpenIGSourceFiles}
    openig.xml
	${SHADER_FILES}
)

TARGET_LINK_LIBRARIES( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG-Base
    OpenIG-PluginBase
)

SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Core OpenIG" )
SET_TARGET_PROPERTIES (${target} PROPERTIES DEFINE_SYMBOL  "OPENIG_LIBRARY")

INCLUDE( ModuleInstall REQUIRED )

IF(WIN32)
    SET(INSTALL_BINDIR bin)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/logz_ps.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/logz_vs.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
ELSE()
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/logz_ps.glsl DESTINATION /usr/local/openig/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/logz_vs.glsl DESTINATION /usr/local/openig/resources/shaders)
ENDIF()

