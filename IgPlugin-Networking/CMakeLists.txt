set( LIB_NAME IgPlugin-Networking )
set( TARGET_OTHER_FILES ConfigFiles/libIgPlugin-Networking.so.xml )

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
    OpenIG
	IgLib-Networking
	${BOOST_LIBRARIES}
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin ${LIB_NAME}" )

include( PluginInstall REQUIRED )

if (APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/ConfigFiles/libIgPlugin-Networking.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libIgPlugin-Networking.dylib.xml)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/ConfigFiles/libIgPlugin-Networking.so.xml DESTINATION ${INSTALL_BINDIR} RENAME IgPlugin-Networking.dll.xml)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/ConfigFiles/libIgPlugin-Networking.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libIgPlugin-Networking.so.xml)
endif()
