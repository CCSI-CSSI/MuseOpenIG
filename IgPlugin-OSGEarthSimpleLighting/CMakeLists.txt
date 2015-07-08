set( LIB_NAME IgPlugin-OSGEarthSimpleLighting )

add_library( ${LIB_NAME} SHARED
    igpluginosgearthsimplelighting.cpp    
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG
	${OSGEARTH_LIBRARY}
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin ${LIB_NAME}" )

include( PluginInstall REQUIRED )

