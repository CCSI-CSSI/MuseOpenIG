set( LIB_NAME IgPlugin-Triton )
set( TARGET_OTHER_FILES ConfigFiles/libIgPlugin-Triton.so.xml
                        ConfigFiles/libIgPlugin-Triton.so.windows.xml )

add_library( ${LIB_NAME} SHARED
    igplugintriton.cpp
    TritonDrawable.h
    TritonDrawable.cpp
    PlanarReflection.h
    PlanarReflection.cpp
    ${TARGET_OTHER_FILES}
)

include_directories(
        ${OPENSCENEGRAPH_INCLUDE_DIR}
        ${OPENGL_INCLUDE_DIR}
        ${TRITON_INCLUDE_DIR}		
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    debug ${TRITON_LIBRARY_DEBUG} optimized ${TRITON_LIBRARY_RELEASE}    
    OpenIG	
)

if (APPLE)
target_link_libraries( ${LIB_NAME}
    fftss
    ${OPENGL_LIBRARY}
)

endif()
set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin ${LIB_NAME}" )

include( PluginInstall REQUIRED )

if (APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/ConfigFiles/libIgPlugin-Triton.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libIgPlugin-Triton.dylib.xml)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/ConfigFiles/libIgPlugin-Triton.so.windows.xml DESTINATION ${INSTALL_BINDIR} RENAME IgPlugin-Triton.dll.xml)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/ConfigFiles/libIgPlugin-Triton.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libIgPlugin-Triton.so.xml)
endif()

