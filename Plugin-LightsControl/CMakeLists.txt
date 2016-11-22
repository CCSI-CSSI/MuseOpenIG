SET( LIB_NAME OpenIG-Plugin-LightsControl )

FILE (GLOB SHADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/lightpoint_ps.glsl
    ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/lightpoint_vs.glsl
    ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/sprite_bb_gs.glsl
    ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/sprite_bb_ps.glsl
    ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/sprite_bb_vs.glsl
)

SOURCE_GROUP("Shaders" FILES ${SHADER_FILES})
SET (SOURCE_FILES IGPluginLightsControl.cpp)

ADD_LIBRARY( ${LIB_NAME} SHARED
        ${SOURCE_FILES}
        DataFiles/master.flt.osg.lighting.xml
    ${SHADER_FILES}
)

INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
)

TARGET_LINK_LIBRARIES( ${LIB_NAME}
        ${OSG_LIBRARIES}
        OpenIG-Engine
    OpenIG-Utils
    ${Boost_LIBRARIES}
)

SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin LightsControl" )

INCLUDE( PluginInstall REQUIRED )

#FILE(GLOB resources_shaders "${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/*.glsl")
FILE(GLOB resources_textures "${CMAKE_CURRENT_LIST_DIR}/../Resources/textures/*.*")

SET(INSTALL_BINDIR bin)
IF (WIN32)
#	INSTALL(FILES  ${resources_shaders} DESTINATION ${INSTALL_BINDIR}/resources/shaders)
        INSTALL(FILES  ${resources_textures} DESTINATION ${INSTALL_BINDIR}/resources/textures)
ELSE()
#	INSTALL(FILES  ${resources_shaders} DESTINATION /usr/local/openig/resources/shaders)
        INSTALL(FILES  ${resources_textures} DESTINATION /usr/local/openig/resources/textures)
ENDIF()
IF(WIN32)
    SET(INSTALL_BINDIR bin)
        INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/lightpoint_ps.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/lightpoint_vs.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/sprite_bb_gs.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/sprite_bb_ps.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/sprite_bb_vs.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
ELSE()
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/lightpoint_ps.glsl DESTINATION /usr/local/openig/resources/shaders)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/lightpoint_vs.glsl DESTINATION /usr/local/openig/resources/shaders)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/sprite_bb_gs.glsl DESTINATION /usr/local/openig/resources/shaders)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/sprite_bb_ps.glsl DESTINATION /usr/local/openig/resources/shaders)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/sprite_bb_vs.glsl DESTINATION /usr/local/openig/resources/shaders)
ENDIF()


