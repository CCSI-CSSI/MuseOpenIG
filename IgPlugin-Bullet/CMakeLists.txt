set( LIB_NAME IgPlugin-Bullet )

set( _IgPluginBullet
    igpluginbullet.cpp
)

add_library( ${LIB_NAME} SHARED
    ${_IgPluginBullet}
)

include_directories(	
	${BULLET_INCLUDE_DIR}
	${OSGBULLET_INCLUDE_DIR}	
)

target_link_libraries( ${LIB_NAME}
	IgLib-Bullet
	OpenIG
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin ${LIB_NAME}" )

include( PluginInstall REQUIRED )
