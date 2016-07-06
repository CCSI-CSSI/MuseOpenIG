set( APP_NAME oigconv )

add_executable( ${APP_NAME} oigconv.cpp OrientationConverter.cpp OrientationConverter.h )

target_link_libraries( ${APP_NAME}
    ${OSG_LIBRARIES}
	OpenIG-Base
)

install(
    TARGETS ${APP_NAME}
    RUNTIME DESTINATION bin COMPONENT openig
)

set_target_properties( ${APP_NAME} PROPERTIES PROJECT_LABEL "Utility ${APP_NAME}" )
