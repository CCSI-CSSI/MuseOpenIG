set( LIB_NAME IgPlugin-SilverLining )

add_library( ${LIB_NAME} SHARED
    AtmosphereReference.h
    CloudsDrawable.h
    SkyDrawable.h
    CloudsDrawable.cpp
    SkyDrawable.cpp
    igpluginsilverlining.cpp
    libIgPlugin-SilverLining.so.xml
)

include_directories(
        ${OPENSCENEGRAPH_INCLUDE_DIR}
        ${OPENGL_INCLUDE_DIR}
        ${SILVERLINING_INCLUDE_DIR}
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    debug ${SILVERLINING_LIBRARY_DEBUG} optimized ${SILVERLINING_LIBRARY_RELEASE}
    ${OPENGL_LIBRARY}
    ${GLU_LIBRARY}
    OpenIG
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin ${LIB_NAME}" )

include( PluginInstall REQUIRED )

if (APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/libIgPlugin-SilverLining.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libIgPlugin-SilverLining.dylib.xml)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/libIgPlugin-SilverLining.so.xml DESTINATION ${INSTALL_BINDIR} RENAME IgPlugin-SilverLining.dll.xml)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/libIgPlugin-SilverLining.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libIgPlugin-SilverLining.so.xml)
endif()
