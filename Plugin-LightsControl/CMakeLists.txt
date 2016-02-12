set( LIB_NAME OpenIG-Plugin-LightsControl )

add_library( ${LIB_NAME} SHARED
    igpluginlightscontrol.cpp
    DataFiles/master.flt.osg.lighting.xml
)

include_directories(
	${Boost_INCLUDE_DIRS}	
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG-Engine
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
