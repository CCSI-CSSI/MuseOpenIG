set( LIB_NAME OpenIG-Plugin-OSGEarthSimpleLighting )

add_library( ${LIB_NAME} SHARED
    igpluginosgearthsimplelighting.cpp    
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG-Engine
	${OSGEARTH_LIBRARY}
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin OSGEarthSimpleLighting" )

include( PluginInstall REQUIRED )

