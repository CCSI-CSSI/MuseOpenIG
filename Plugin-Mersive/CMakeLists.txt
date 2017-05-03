SET( LIB_NAME OpenIG-Plugin-Mersive )

SET( _IgPluginMersive
    IGPluginMersive.cpp
    MersiveOsgWarperCallback.cpp
    IniFile.cpp
    fgmersivelib.h
)

INCLUDE_DIRECTORIES(
    /mersive/include
)
ADD_LIBRARY( ${LIB_NAME} SHARED
    ${_IgPluginMersive}
    Plugin-Mersive.pro
    DataFiles/libIgPlugin-Mersive.so.xml
    README_OPENSCENEGRAPH_PATCH.txt
)

IF(WIN32)
    TARGET_LINK_LIBRARIES( ${LIB_NAME}
    OpenIG-Engine
    ${OSG_LIBRARIES}
    jpeg
    MersiveCalibrator
    EwbRemoteRendererDeviceProxy
    remoterenderer
    )
ELSE()
    TARGET_LINK_LIBRARIES( ${LIB_NAME}
    OpenIG-Engine
    ${OSG_LIBRARIES}
#    CstShare
    jpeg
    /mersive/lib/libMersiveCalibrator.a
    /mersive/lib/libEwbRemoteRendererDeviceProxy.so
    /mersive/lib/libremoterenderer.so
    )
ENDIF()


SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin Mersive" )

INCLUDE( PluginInstall REQUIRED )

IF (APPLE)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Mersive.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-Mersive.dylib.xml)
ELSEIF(WIN32)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Mersive.so.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-Mersive.dll.xml)
ELSE()
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Mersive.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-Mersive.so.xml)
ENDIF()

