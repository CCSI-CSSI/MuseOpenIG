set( APP_NAME openig-demo )

add_executable( ${APP_NAME} main.cpp )

target_link_libraries( ${APP_NAME}
    ${OSG_LIBRARIES}
    OpenIG
)

install(
    TARGETS ${APP_NAME}
    RUNTIME DESTINATION bin COMPONENT openig
)

set_target_properties( ${APP_NAME} PROPERTIES PROJECT_LABEL "Application ${APP_NAME}" )

SET(INSTALL_INCDIR include)
SET(INSTALL_BINDIR bin)
IF(WIN32)
    SET(INSTALL_LIBDIR bin)
    SET(INSTALL_ARCHIVEDIR lib)
ELSE()
    SET(INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
    SET(INSTALL_ARCHIVEDIR ${CMAKE_INSTALL_LIBDIR})
ENDIF()

install(FILES ${CMAKE_CURRENT_LIST_DIR}/flightpath.path DESTINATION ${INSTALL_BINDIR}/demo)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/default.txt DESTINATION ${INSTALL_BINDIR})
