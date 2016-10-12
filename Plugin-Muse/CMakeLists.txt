set( LIB_NAME OpenIG-Plugin-Muse )

set( _IgPluginMuse
    igpluginmuse.cpp
    movingmodelmanager.cpp
    movingmodelmanager.h
    miscellaneous.cpp
    miscellaneous.h
    scenemodel.h
)

include_directories(
    /usr/local/muse/core/inc
    /usr/local/muse/libs/inc
    /usr/local/include/Public_Headers
)

add_library( ${LIB_NAME} SHARED
    ${_IgPluginMuse}
)

if(WIN32)
    target_link_libraries( ${LIB_NAME}
    OpenIG-Engine
    OpenIG-Base
    OpenIG-Networking
    OpenIG-Protocol
    ${OSG_LIBRARIES}
    )
else()
    target_link_libraries( ${LIB_NAME}
    OpenIG-Engine
    OpenIG-Base
    OpenIG-Networking
    OpenIG-Protocol
    ${OSG_LIBRARIES}
    CstShare
    QtUtil
    )
endif()


set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin MUSE" )

include( PluginInstall REQUIRED )

if (APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Muse.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-Muse.dylib.xml)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Muse.so.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-Muse.dll.xml)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Muse.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-Muse.so.xml)
endif()
