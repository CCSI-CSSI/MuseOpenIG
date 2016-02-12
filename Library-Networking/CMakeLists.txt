set( LIB_NAME OpenIG-Networking )
set( HEADER_PATH ${PROJECT_SOURCE_DIR}/Library-Networking )
add_definitions( -DIGLIBNETWORKING_LIBRARY )

set( LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/export.h
	${HEADER_PATH}/buffer.h		
	${HEADER_PATH}/network.h
	${HEADER_PATH}/packet.h
	${HEADER_PATH}/udpnetwork.h
	${HEADER_PATH}/parser.h
	${HEADER_PATH}/tcpserver.h
	${HEADER_PATH}/tcpclient.h
	${HEADER_PATH}/error.h
	${HEADER_PATH}/factory.h
)

set( _IgLibNetworkingSourceFiles
    buffer.cpp
	network.cpp
	packet.cpp
	udpnetwork.cpp
	tcpserver.cpp
	tcpclient.cpp
	factory.cpp
)

add_library( ${LIB_NAME} SHARED
    ${LIB_PUBLIC_HEADERS}
    ${_IgLibNetworkingSourceFiles}
)

include_directories(
	${Boost_INCLUDE_DIRS}	
)

target_link_libraries( ${LIB_NAME}    
	${Boost_LIBRARIES}
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Library Networking" )
set_target_properties (${target} PROPERTIES DEFINE_SYMBOL  "IGLIBNETWORKING_LIBRARY")

include( ModuleInstall REQUIRED )

