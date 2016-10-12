set( LIB_NAME OpenIG-Plugin-LightsControl )

FILE (GLOB SHADER_FILES
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/lightpoint_ps.glsl
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/lightpoint_vs.glsl	
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/sprite_bb_gs.glsl
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/sprite_bb_ps.glsl
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/sprite_bb_vs.glsl
)

SOURCE_GROUP("Shaders" FILES ${SHADER_FILES})

add_library( ${LIB_NAME} SHARED
    igpluginlightscontrol.cpp
    DataFiles/master.flt.osg.lighting.xml
	${SHADER_FILES}
)

include_directories(
	${Boost_INCLUDE_DIRS}	
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG-Engine
	OpenIG-Utils
	${Boost_LIBRARIES}
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin LightsControl" )

include( PluginInstall REQUIRED )

FILE(GLOB resources_shaders "${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/*.glsl")
FILE(GLOB resources_textures "${CMAKE_CURRENT_LIST_DIR}/../Resources/textures/*.*")

SET(INSTALL_BINDIR bin)
if (WIN32)
	install(FILES  ${resources_shaders} DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	install(FILES  ${resources_textures} DESTINATION ${INSTALL_BINDIR}/resources/textures)
else()
	install(FILES  ${resources_shaders} DESTINATION /usr/local/openig/resources/shaders)
	install(FILES  ${resources_textures} DESTINATION /usr/local/openig/resources/textures)
endif()
if(WIN32)
    SET(INSTALL_BINDIR bin)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/lightpoint_ps.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/lightpoint_vs.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/sprite_bb_gs.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/sprite_bb_ps.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/sprite_bb_vs.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
else()	
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/lightpoint_ps.glsl DESTINATION /usr/local/openig/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/lightpoint_vs.glsl DESTINATION /usr/local/openig/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/sprite_bb_gs.glsl DESTINATION /usr/local/openig/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/sprite_bb_ps.glsl DESTINATION /usr/local/openig/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/sprite_bb_vs.glsl DESTINATION /usr/local/openig/resources/shaders)
endif()


