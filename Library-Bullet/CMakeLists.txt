set( LIB_NAME OpenIG-Bullet )
set( HEADER_PATH ${PROJECT_SOURCE_DIR}/Library-Bullet )
add_definitions( -DIGLIBBULLET_LIBRARY )

set( LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/export.h
	${HEADER_PATH}/bulletmanager.h
	${HEADER_PATH}/configreader.h	
	${HEADER_PATH}/vehicle.h	
)

set( _IgLibBulletSourceFiles
    bulletmanager.cpp
	configreader.cpp
	vehicle.cpp
)

add_library( ${LIB_NAME} SHARED
    ${LIB_PUBLIC_HEADERS}
    ${_IgLibBulletSourceFiles}
)

include_directories(
	${Boost_INCLUDE_DIRS}
	${BULLET_INCLUDE_DIR}
	${OSGBULLET_INCLUDE_DIR}
)

target_link_libraries( ${LIB_NAME}    
	${OSG_LIBRARIES}
	${OSGBULLET_LIBRARIES}
	${BULLET_LIBRARIES}
	${Boost_LIBRARIES}
	OpenIG-Base
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Library Bullet" )
set_target_properties (${target} PROPERTIES DEFINE_SYMBOL  "IGLIBBULLET_LIBRARY")

include( ModuleInstall REQUIRED )
