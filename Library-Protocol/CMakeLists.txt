set( LIB_NAME OpenIG-Protocol )
set( HEADER_PATH ${PROJECT_SOURCE_DIR}/Library-Protocol)
add_definitions( -DIGLIBPROTOCOL_LIBRARY )

set( LIB_PUBLIC_HEADERS
	${HEADER_PATH}/export.h
    ${HEADER_PATH}/opcodes.h	
	${HEADER_PATH}/header.h	
	${HEADER_PATH}/entitystate.h	
	${HEADER_PATH}/camera.h	
	${HEADER_PATH}/hot.h
	${HEADER_PATH}/hotresponse.h
	${HEADER_PATH}/los.h
	${HEADER_PATH}/losresponse.h
	${HEADER_PATH}/tod.h
	${HEADER_PATH}/command.h
	${HEADER_PATH}/lightstate.h
)

set( LIB_SOURCE
    header.cpp
	camera.cpp
	entitystate.cpp
	hot.cpp
	hotresponse.cpp
	los.cpp
	losresponse.cpp
	tod.cpp
	command.cpp
	lightstate.cpp
	)
	
add_library( ${LIB_NAME} SHARED
	${LIB_PUBLIC_HEADERS}
    ${LIB_SOURCE}
)

include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}    
	OpenIG-Networking
	${BOOST_LIBRARIES}
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Library Protocol" )
set_target_properties (${target} PROPERTIES DEFINE_SYMBOL  "IGLIBPROTOCOL_LIBRARY")

include( ModuleInstall REQUIRED )

