set( LIB_NAME OpenIG-Plugin-Networking )
set( TARGET_OTHER_FILES DataFiles/libIgPlugin-Networking.so.xml )

set( _IgPluginNetworking
    igpluginnetworking.cpp
	${TARGET_OTHER_FILES}
	)

add_library( ${LIB_NAME} SHARED
    ${_IgPluginNetworking}
)

include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG-Engine
	OpenIG-Networking
	${BOOST_LIBRARIES}
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin Networking" )

include( PluginInstall REQUIRED )

if (APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Networking.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-Networking.dylib.xml)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Networking.so.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-Networking.dll.xml)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Networking.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-Networking.so.xml)
endif()
