set( LIB_NAME OpenIG-Plugin-FBXAnimation )

set( _IgFBXPluginAnimation
    igpluginfbxanimation.cpp
)

add_library( ${LIB_NAME} SHARED
    ${_IgFBXPluginAnimation}
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG-Engine
	OpenIG-PluginBase
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin FBXAnimation" )

include( PluginInstall REQUIRED )
