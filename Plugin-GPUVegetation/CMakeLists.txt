SET( LIB_NAME OpenIG-Plugin-GPUVegetation )

SET (SOURCE_FILES 
	IGPluginGPUVegetation.cpp
)

FILE (GLOB SHADER_FILES 
	./shaders/vegetation_vs.glsl
	./shaders/vegetation_gs.glsl
	./shaders/vegetation_ps.glsl
)

SOURCE_GROUP(Shaders FILES ${SHADER_FILES})

ADD_LIBRARY( ${LIB_NAME} SHARED
    ${SOURCE_FILES}
	${SHADER_FILES}
    DataFiles/libIgPlugin-GPUVegetation.so.xml
    DataFiles/Readme.txt
)

TARGET_LINK_LIBRARIES( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG-Engine
)

SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin GPUVegetation" )

INCLUDE( PluginInstall REQUIRED )

IF (APPLE)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-GPUVegetation.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-GPUVegetation.dylib.xml)
ELSEIF(WIN32)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-GPUVegetation.so.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-GPUVegetation.dll.xml)
ELSE()
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-GPUVegetation.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-GPUVegetation.so.xml)
ENDIF()

SET(INSTALL_BINDIR bin)
IF (WIN32)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/shaders/vegetation_vs.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/shaders/vegetation_gs.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/shaders/vegetation_ps.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
ELSE()
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/shaders/vegetation_vs.glsl DESTINATION /usr/local/openig/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/shaders/vegetation_gs.glsl DESTINATION /usr/local/openig/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/shaders/vegetation_ps.glsl DESTINATION /usr/local/openig/resources/shaders)
ENDIF()
