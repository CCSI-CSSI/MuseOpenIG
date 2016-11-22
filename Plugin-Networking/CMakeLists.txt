SET( LIB_NAME OpenIG-Plugin-Networking )
SET( TARGET_OTHER_FILES DataFiles/libIgPlugin-Networking.so.xml )

SET( PluginNetworkingSource
     IGPluginNetworking.cpp
)
     
SET( _IgPluginNetworking
    ${PluginNetworkingSource}
	${TARGET_OTHER_FILES}
)

ADD_LIBRARY( ${LIB_NAME} SHARED
    ${_IgPluginNetworking}
)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

TARGET_LINK_LIBRARIES( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG-Engine
	OpenIG-Networking
	OpenIG-Protocol
	${BOOST_LIBRARIES}
)

SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin Networking" )

INCLUDE( PluginInstall REQUIRED )

IF (APPLE)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Networking.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-Networking.dylib.xml)
ELSEIF(WIN32)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Networking.so.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-Networking.dll.xml)
ELSE()
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Networking.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-Networking.so.xml)
ENDIF()
