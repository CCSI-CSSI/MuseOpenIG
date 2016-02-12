set( LIB_NAME OpenIG-Plugin-SkyDome )
set( TARGET_SRC_FILES igpluginskydome.cpp )
set( TARGET_OTHER_FILES DataFiles/libIgPlugin-SkyDome.so.xml
                        DataFiles/libIgPlugin-SkyDome.so.windows.xml )

add_library( ${LIB_NAME} SHARED
             ${TARGET_SRC_FILES}
             ${TARGET_OTHER_FILES} )

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG-Engine
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin SkyDome" )

include( PluginInstall REQUIRED )

if (APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-SkyDome.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-SkyDome.dylib.xml)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-SkyDome.so.windows.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-SkyDome.dll.xml)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-SkyDome.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-SkyDome.so.xml)
endif()
