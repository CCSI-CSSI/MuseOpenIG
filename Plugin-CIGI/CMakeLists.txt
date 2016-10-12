set( LIB_NAME OpenIG-Plugin-CIGI )
set( TARGET_OTHER_FILES DataFiles/libIgPlugin-CIGI.so.xml )

set( _IgPluginCIGI
    igplugincigi.cpp
    ${TARGET_OTHER_FILES}
)

add_library( ${LIB_NAME} SHARED
    ${_IgPluginCIGI}
)

include_directories(${Boost_INCLUDE_DIRS} ${CCL_INCLUDE_DIR})

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG-Engine
    OpenIG-Networking
    ${BOOST_LIBRARIES}
    cigicl
#    ${CCL_LIBRARIES}
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin CIGI" )

include( PluginInstall REQUIRED )

if (APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-CIGI.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-CIGI.dylib.xml)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-CIGI.so.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-CIGI.dll.xml)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-CIGI.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-CIGI.so.xml)
endif()
