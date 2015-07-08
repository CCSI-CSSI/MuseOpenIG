set( LIB_NAME IgPlugin-FBXAnimation )

set( _IgFBXPluginAnimation
    igpluginfbxanimation.cpp
)

add_library( ${LIB_NAME} SHARED
    ${_IgFBXPluginAnimation}
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG
	IgPluginCore
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin ${LIB_NAME}" )

include( PluginInstall REQUIRED )
