set( APP_NAME ig )

add_executable( ${APP_NAME} main.cpp )

target_link_libraries( ${APP_NAME}
    ${OSG_LIBRARIES}
    OpenIG
    ${Boost_LIBRARIES}
)

install(
    TARGETS ${APP_NAME}
    RUNTIME DESTINATION bin COMPONENT openig
)

set_target_properties( ${APP_NAME} PROPERTIES PROJECT_LABEL "Application ${APP_NAME}" )
