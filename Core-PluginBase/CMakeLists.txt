set( LIB_NAME OpenIG-PluginBase )
set( HEADER_PATH ${PROJECT_SOURCE_DIR}/Core-PluginBase )
add_definitions( -DIGPLUGINCORE_LIBRARY )

set( LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/config.h
    ${HEADER_PATH}/export.h
    ${HEADER_PATH}/igplugincore.h
    ${HEADER_PATH}/plugin.h
    ${HEADER_PATH}/plugincontext.h
    ${HEADER_PATH}/pluginhost.h
    ${HEADER_PATH}/pluginoperation.h
)

set( _IgPluginCoreSourceFiles
    pluginhost.cpp
)

include_directories(${Boost_INCLUDE_DIRS})

add_library( ${LIB_NAME} SHARED
    ${LIB_PUBLIC_HEADERS}
    ${_IgPluginCoreSourceFiles}
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    ${Boost_LIBRARIES}
    OpenIG-Base
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Core PluginBase" )
set_target_properties (${target} PROPERTIES DEFINE_SYMBOL  "IGPLUGINCORE_LIBRARY")

include( ModuleInstall REQUIRED )
