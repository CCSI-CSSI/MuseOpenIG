set( LIB_NAME OpenIG-Plugin-Client )
add_definitions( -DOPENIG_SDK )

set( SourceFiles
    main.cpp
)

add_library( ${LIB_NAME} SHARED    
    ${SourceFiles}
	DataFiles/libIgPlugin-Client.so.xml
)

include_directories(
	${Boost_INCLUDE_DIRS}	
	${OPENIG_INCLUDE_DIR}	
	${OSG_INCLUDE_DIRS}	
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
	${OPENIG_LIBRARIES}
    ${Boost_LIBRARIES}
	${OPENGL_LIBRARY}
)

set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin OpenIG-Plugin" )

include( PluginInstall REQUIRED )

if (APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Client.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-Client.dylib.xml)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Client.so.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-Client.dll.xml)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-Client.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-Client.so.xml)
endif()

