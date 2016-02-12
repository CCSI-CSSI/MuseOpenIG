set( LIB_NAME OpenIG-Plugin-VDBOffset )

add_library( ${LIB_NAME} SHARED
    igpluginvdboffset.cpp
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG-Engine
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin VDBOffset" )

include( PluginInstall REQUIRED )
