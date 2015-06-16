set( LIB_NAME IgPlugin-LightPoints )

add_library( ${LIB_NAME} SHARED
    LightPointDrawable.h
    LightPointSpriteDrawable.h
    lightpointnode.h
    LightPointDrawable.cpp
    LightPointSpriteDrawable.cpp
    igpluginlightpoints.cpp
)

if (OPENGL_FOUND)
    if(APPLE)
       set(CMAKE_CXX_LINK_FLAGS "-framework OpenGL")
    endif(APPLE)
    include_directories(${OPENGL_INCLUDE_DIR})
endif (OPENGL_FOUND)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG
    ${OPENGL_LIBRARIES}
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin ${LIB_NAME}" )

include( PluginInstall REQUIRED )
