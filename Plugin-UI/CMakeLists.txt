set( LIB_NAME OpenIG-Plugin-UI )
set( TARGET_OTHER_FILES DataFiles/libIgPlugin-UI.so.xml DataFiles/libIgPlugin-UI.so.windows.xml)
set( IgPluginUIFiles 
	igpluginui.cpp 
	MYGUIManager.h 
	MYGUIManager.cpp 
	FileSelectionDialog.h
	FileSelectionDialog.cpp
	ColourPanel.h
	ColourPanel.cpp
	${TARGET_OTHER_FILES}
)

add_library( ${LIB_NAME} SHARED ${IgPluginUIFiles}  )

include_directories( ${MYGUI_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} )

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    OpenIG-Engine
	${MYGUI_LIBRARY}
	${MYGUI_OPENGL_LIBRARY}
	${OPENGL_LIBRARY}
	${BOOST_LIBRARIES}
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin UI" )

include( PluginInstall REQUIRED )

if (APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-UI.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-UI.dylib.xml)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-UI.so.windows.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-UI.dll.xml)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-UI.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-UI.so.xml)
endif()
