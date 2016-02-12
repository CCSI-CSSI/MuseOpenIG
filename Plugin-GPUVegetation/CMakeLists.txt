set( LIB_NAME OpenIG-Plugin-GPUVegetation )

set (SOURCE_FILES 
	igplugingpuvegetation.cpp
)

FILE (GLOB SHADER_FILES 
	./shaders/vegetation_vs.glsl
	./shaders/vegetation_gs.glsl
	./shaders/vegetation_ps.glsl
)

SOURCE_GROUP(Shaders FILES ${SHADER_FILES})

add_library( ${LIB_NAME} SHARED
    ${SOURCE_FILES}
	${SHADER_FILES}
    DataFiles/libIgPlugin-GPUVegetation.so.xml
    DataFiles/Readme.txt
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG-Engine
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin GPUVegetation" )

include( PluginInstall REQUIRED )

if (APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-GPUVegetation.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-GPUVegetation.dylib.xml)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-GPUVegetation.so.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-GPUVegetation.dll.xml)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-GPUVegetation.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-GPUVegetation.so.xml)
endif()

SET(INSTALL_BINDIR bin)
if (WIN32)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/shaders/vegetation_vs.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/shaders/vegetation_gs.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/shaders/vegetation_ps.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
else()
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/shaders/vegetation_vs.glsl DESTINATION /usr/local/openig/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/shaders/vegetation_gs.glsl DESTINATION /usr/local/openig/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/shaders/vegetation_ps.glsl DESTINATION /usr/local/openig/resources/shaders)
endif()
