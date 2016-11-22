SET( LIB_NAME OpenIG-Plugin-Triton )
SET( TARGET_OTHER_FILES DataFiles/libIgPlugin-Triton.so.xml
                        DataFiles/libIgPlugin-Triton.so.windows.xml )

FILE (GLOB SHADER_FILES 
	../Resources/shaders/Triton/user-functions.glsl
	../Resources/shaders/Triton/user-vert-functions.glsl
	../Resources/shaders/Triton/forward_plus_triton_ps.glsl
)

SOURCE_GROUP(Shaders FILES ${SHADER_FILES})

ADD_LIBRARY( ${LIB_NAME} SHARED
    IGPluginTriton.cpp
    TritonDrawable.h
    TritonDrawable.cpp
    PlanarReflection.h
    PlanarReflection.cpp
    ${SHADER_FILES}
    ${TARGET_OTHER_FILES}
)

INCLUDE_DIRECTORIES(
        ${OPENSCENEGRAPH_INCLUDE_DIR}
        ${OPENGL_INCLUDE_DIR}
        ${TRITON_INCLUDE_DIR}	
	${Boost_INCLUDE_DIRS}		
)

TARGET_LINK_LIBRARIES( ${LIB_NAME}
    ${OSG_LIBRARIES}
    debug ${TRITON_LIBRARY_DEBUG} optimized ${TRITON_LIBRARY_RELEASE}    
    OpenIG-Engine
	OpenIG-Utils
    ${Boost_LIBRARIES}
)

IF ( APPLE OR ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
TARGET_LINK_LIBRARIES( ${LIB_NAME}
    fftss
    ${OPENGL_LIBRARY}
)

ENDIF()
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin Triton" )

INCLUDE( PluginInstall REQUIRED )

IF (APPLE)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Triton.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-Triton.dylib.xml)
ELSEIF(WIN32)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Triton.so.windows.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-Triton.dll.xml)
ELSE()
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Triton.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-Triton.so.xml)
ENDIF()

#IF(WIN32)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/Triton/user-functions.glsl DESTINATION $ENV{TRITON_PATH}/Resources)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/Triton/user-vert-functions.glsl DESTINATION $ENV{TRITON_PATH}/Resources)
#ENDIF()

IF(WIN32)
    SET(INSTALL_BINDIR bin)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/Triton/forward_plus_triton_ps.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
ELSE()
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/Triton/forward_plus_triton_ps.glsl DESTINATION /usr/local/openig/resources/shaders)
ENDIF()
