set( LIB_NAME OpenIG-Plugin-SimpleLighting )

FILE (GLOB SHADER_FILES
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/simplelighting_vs.glsl
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/simplelighting_ps.glsl	
)

SOURCE_GROUP("Shaders" FILES ${SHADER_FILES})


add_library( ${LIB_NAME} SHARED
    igpluginsimplelighting.cpp
    DataFiles/libIgPlugin-SimpleLighting.so.xml
	${SHADER_FILES}
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG-Engine
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin SimpleLighting" )

include( PluginInstall REQUIRED )

if (APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-SimpleLighting.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-SimpleLighting.dylib.xml)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-SimpleLighting.so.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-SimpleLighting.dll.xml)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-SimpleLighting.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-SimpleLighting.so.xml)
endif()

if(WIN32)
    SET(INSTALL_BINDIR bin)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/simplelighting_ps.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/simplelighting_vs.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/simplelighting_vs.glsl DESTINATION /usr/local/openig/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/simplelighting_ps.glsl DESTINATION /usr/local/openig/resources/shaders)
endif()
