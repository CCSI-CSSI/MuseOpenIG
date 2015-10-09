set( LIB_NAME IgPlugin-Lighting )
set( TARGET_OTHER_FILES ConfigFiles/libIgPlugin-Lighting.so.xml )

add_library( ${LIB_NAME} SHARED
    igpluginlighting.cpp
    ${TARGET_OTHER_FILES}
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin ${LIB_NAME}" )

include( PluginInstall REQUIRED )

if (APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/ConfigFiles/libIgPlugin-Lighting.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libIgPlugin-Lighting.dylib.xml)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/ConfigFiles/libIgPlugin-Lighting.so.xml DESTINATION ${INSTALL_BINDIR} RENAME IgPlugin-Lighting.dll.xml)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/ConfigFiles/libIgPlugin-Lighting.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libIgPlugin-Lighting.so.xml)
endif()
