set( LIB_NAME OpenIG-Plugin-Muse )

set( _IgPluginMuse
    igpluginmuse.cpp
)

include_directories(
    /usr/local/muse/core/inc
)

add_library( ${LIB_NAME} SHARED
    ${_IgPluginMuse}
)

if(WIN32)
    target_link_libraries( ${LIB_NAME}
    OpenIG-Engine
    ${OSG_LIBRARIES}
    )
else()
    target_link_libraries( ${LIB_NAME}
    OpenIG-Engine
    ${OSG_LIBRARIES}
    CstShare
    )
endif()


set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin MUSE" )

include( PluginInstall REQUIRED )

