set( LIB_NAME OpenIG-Plugin-Triton )
set( TARGET_OTHER_FILES DataFiles/libIgPlugin-Triton.so.xml
                        DataFiles/libIgPlugin-Triton.so.windows.xml )

FILE (GLOB SHADER_FILES 
	../Resources/shaders/Triton/user-functions.glsl
	../Resources/shaders/Triton/user-vert-functions.glsl
	../Resources/shaders/Triton/forward_plus_triton_ps.glsl
)

SOURCE_GROUP(Shaders FILES ${SHADER_FILES})

add_library( ${LIB_NAME} SHARED
    igplugintriton.cpp
    TritonDrawable.h
    TritonDrawable.cpp
    PlanarReflection.h
    PlanarReflection.cpp
    ${SHADER_FILES}
    ${TARGET_OTHER_FILES}
)

include_directories(
        ${OPENSCENEGRAPH_INCLUDE_DIR}
        ${OPENGL_INCLUDE_DIR}
        ${TRITON_INCLUDE_DIR}	
	${Boost_INCLUDE_DIRS}		
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    debug ${TRITON_LIBRARY_DEBUG} optimized ${TRITON_LIBRARY_RELEASE}    
    OpenIG-Engine
    ${Boost_LIBRARIES}
)

if ( APPLE OR ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
target_link_libraries( ${LIB_NAME}
    fftss
    ${OPENGL_LIBRARY}
)

endif()
set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin Triton" )

include( PluginInstall REQUIRED )

if (APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Triton.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-Triton.dylib.xml)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Triton.so.windows.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-Triton.dll.xml)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Triton.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-Triton.so.xml)
endif()

#if(WIN32)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/Triton/user-functions.glsl DESTINATION $ENV{TRITON_PATH}/Resources)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/Triton/user-vert-functions.glsl DESTINATION $ENV{TRITON_PATH}/Resources)
#endif()

if(WIN32)
    SET(INSTALL_BINDIR bin)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/Triton/forward_plus_triton_ps.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/Triton/forward_plus_triton_ps.glsl DESTINATION /usr/local/openig/resources/shaders)
endif()
