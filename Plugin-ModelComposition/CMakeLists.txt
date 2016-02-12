set( LIB_NAME OpenIG-Plugin-ModelComposition )
set( TARGET_OTHER_FILES 
	DataFiles/libIgPlugin-ModelComposition.so.xml
	DataFiles/a320.ive.xml
)

add_library( ${LIB_NAME} SHARED
    igpluginmodelcomposition.cpp
    ${TARGET_OTHER_FILES}
)

if (OPENGL_FOUND)
    if(APPLE)
       set(CMAKE_CXX_LINK_FLAGS "-framework OpenGL")
    endif(APPLE)
    include_directories(${OPENGL_INCLUDE_DIR})
endif (OPENGL_FOUND)

include_directories(
	${Boost_INCLUDE_DIRS}	
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG-Engine
    ${OPENGL_LIBRARIES}
	${Boost_LIBRARIES}
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin ModelComposition" )

include( PluginInstall REQUIRED )

if (APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-ModelComposition.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-ModelComposition.dylib.xml)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-ModelComposition.so.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-ModelComposition.dll.xml)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-ModelComposition.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-ModelComposition.so.xml)
endif()
