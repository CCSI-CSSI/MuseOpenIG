add_definitions( -DOPENIG_SDK )
set( APP_NAME openig-client-tqserver )

set( TARGET_SRC_FILES main.cpp )

add_executable( ${APP_NAME} ${TARGET_SRC_FILES} )

include_directories(
	${Boost_INCLUDE_DIRS}	
	${OPENIG_INCLUDE_DIR}	
	${OSG_INCLUDE_DIRS}	
)

target_link_libraries( ${APP_NAME}
    ${OSG_LIBRARIES}
	${OPENIG_LIBRARIES}
    ${Boost_LIBRARIES}
	${OPENGL_LIBRARY}
)

install(
    TARGETS ${APP_NAME}
    RUNTIME DESTINATION bin/openig-client COMPONENT openig-client
)

set_target_properties( ${APP_NAME} PROPERTIES PROJECT_LABEL "Application OpenIG-TerrainQueryServer" )

SET(INSTALL_INCDIR include)
SET(INSTALL_BINDIR bin/openig-client)
IF(WIN32)
    SET(INSTALL_LIBDIR bin)
    SET(INSTALL_ARCHIVEDIR lib)
ELSE()
    SET(INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
    SET(INSTALL_ARCHIVEDIR ${CMAKE_INSTALL_LIBDIR})
ENDIF()

