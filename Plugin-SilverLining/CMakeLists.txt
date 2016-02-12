set( LIB_NAME OpenIG-Plugin-SilverLining )
set( TARGET_OTHER_FILES DataFiles/libIgPlugin-SilverLining.so.xml
                        DataFiles/libIgPlugin-SilverLining.so.windows.xml
						DataFiles/UserFunctions.glsl
						DataFiles/UserFunctions-frag.glsl
)

add_library( ${LIB_NAME} SHARED
    AtmosphereReference.h
    CloudsDrawable.h
    SkyDrawable.h
    CloudsDrawable.cpp
    SkyDrawable.cpp
    igpluginsilverlining.cpp
    ${TARGET_OTHER_FILES}
)

include_directories(
        ${OPENSCENEGRAPH_INCLUDE_DIR}
        ${OPENGL_INCLUDE_DIR}
        ${SILVERLINING_INCLUDE_DIR}
		${Boost_INCLUDE_DIRS}
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    debug ${SILVERLINING_LIBRARY_DEBUG} optimized ${SILVERLINING_LIBRARY_RELEASE}
    ${OPENGL_LIBRARY}
    ${GLU_LIBRARY}
    OpenIG-Engine
	${Boost_LIBRARIES}
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin SilverLining" )

include( PluginInstall REQUIRED )

if (APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-SilverLining.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-SilverLining.dylib.xml)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-SilverLining.so.windows.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-SilverLining.dll.xml)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-SilverLining.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-SilverLining.so.xml)
endif()

#if(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/UserFunctions-frag.glsl DESTINATION $ENV{SILVERLINING_PATH}/Resources/Shaders)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/UserFunctions.glsl DESTINATION $ENV{SILVERLINING_PATH}/Resources/Shaders)
#endif()

if(WIN32)
    SET(INSTALL_BINDIR bin)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/forward_plus_sl_ps.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/forward_plus_sl_ps.glsl DESTINATION /usr/local/openig/resources/shaders)
endif()