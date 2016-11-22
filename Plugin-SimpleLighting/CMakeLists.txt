SET( LIB_NAME OpenIG-Plugin-SimpleLighting )

FILE (GLOB SHADER_FILES
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/simplelighting_vs.glsl
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/simplelighting_ps.glsl	
)

SOURCE_GROUP("Shaders" FILES ${SHADER_FILES})


ADD_LIBRARY( ${LIB_NAME} SHARED
    IGPluginSimpleLighting.cpp
    DataFiles/libIgPlugin-SimpleLighting.so.xml
	${SHADER_FILES}
)

TARGET_LINK_LIBRARIES( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG-Engine
)

SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin SimpleLighting" )

INCLUDE( PluginInstall REQUIRED )

IF (APPLE)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-SimpleLighting.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-SimpleLighting.dylib.xml)
ELSEIF(WIN32)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-SimpleLighting.so.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-SimpleLighting.dll.xml)
ELSE()
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-SimpleLighting.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-SimpleLighting.so.xml)
ENDIF()

IF(WIN32)
    SET(INSTALL_BINDIR bin)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/simplelighting_ps.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/simplelighting_vs.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
ELSE()
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/simplelighting_vs.glsl DESTINATION /usr/local/openig/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/simplelighting_ps.glsl DESTINATION /usr/local/openig/resources/shaders)
ENDIF()
