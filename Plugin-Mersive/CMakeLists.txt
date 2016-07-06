set( LIB_NAME OpenIG-Plugin-Mersive )

set( _IgPluginMersive
    igpluginmersive.cpp
    MersiveOsgWarperCallback.cpp
    IniFile.cpp
    fgmersivelib.h
)

include_directories(
    /mersive/include
)
add_library( ${LIB_NAME} SHARED
    ${_IgPluginMersive}
    Plugin-Mersive.pro
    DataFiles/libIgPlugin-Mersive.so.xml
    README_OPENSCENEGRAPH_PATCH.txt
)

if(WIN32)
    target_link_libraries( ${LIB_NAME}
    OpenIG-Engine
    ${OSG_LIBRARIES}
    jpeg
    MersiveCalibrator
    EwbRemoteRendererDeviceProxy
    remoterenderer
    )
else()
    target_link_libraries( ${LIB_NAME}
    OpenIG-Engine
    ${OSG_LIBRARIES}
    CstShare
    jpeg
    /mersive/lib/libMersiveCalibrator.a
    /mersive/lib/libEwbRemoteRendererDeviceProxy.so
    /mersive/lib/libremoterenderer.so
    )
endif()


set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin Mersive" )

include( PluginInstall REQUIRED )

if (APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Mersive.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-Mersive.dylib.xml)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Mersive.so.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-Mersive.dll.xml)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Mersive.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-Mersive.so.xml)
endif()

