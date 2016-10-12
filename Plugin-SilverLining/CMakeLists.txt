set( LIB_NAME OpenIG-Plugin-SilverLining )
set( TARGET_OTHER_FILES DataFiles/libIgPlugin-SilverLining.so.xml
                        DataFiles/libIgPlugin-SilverLining.so.windows.xml
)

FILE (GLOB SHADER_FILES_FPLZ 
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/SilverLining/WithForwardPlusAndLogZ/UserFunctions-frag.glsl
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/SilverLining/WithForwardPlusAndLogZ/UserFunctions.glsl
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/SilverLining/WithForwardPlusAndLogZ/forward_plus_sl_ps.glsl
)

SOURCE_GROUP("Shaders WithForwardPlusAndLogZ" FILES ${SHADER_FILES_FPLZ})

FILE (GLOB SHADER_FILES_SL 
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/SilverLining/WithSimpleLighting/UserFunctions-frag.glsl
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/SilverLining/WithSimpleLighting/UserFunctions.glsl	
)

SOURCE_GROUP("Shaders WithSimpleLighting" FILES ${SHADER_FILES_SL})

FILE (GLOB SHADER_FILES_ORIGINAL
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/SilverLining/Original/UserFunctions-frag.glsl
	${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/SilverLining/Original/UserFunctions.glsl	
)

SOURCE_GROUP("Shaders Original" FILES ${SHADER_FILES_ORIGINAL})

add_library( ${LIB_NAME} SHARED
    AtmosphereReference.h
    CloudsDrawable.h
    SkyDrawable.h
    CloudsDrawable.cpp
    SkyDrawable.cpp
    igpluginsilverlining.cpp
    ${TARGET_OTHER_FILES}
	${SHADER_FILES_FPLZ}
	${SHADER_FILES_SL}
	${SHADER_FILES_ORIGINAL}
)

include_directories(
        ${OPENSCENEGRAPH_INCLUDE_DIR}
        ${OPENGL_INCLUDE_DIR}
        ${SILVERLINING_INCLUDE_DIR}
		${Boost_INCLUDE_DIRS}
)

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    debug ${SILVERLINING_LIBRARY_DEBUG} optimized ${SILVERLINING_LIBRARY_RELEASE}
    ${OPENGL_LIBRARY}
    ${GLU_LIBRARY}
    OpenIG-Engine
	OpenIG-Utils
	${Boost_LIBRARIES}
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin SilverLining" )

include( PluginInstall REQUIRED )

if (APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-SilverLining.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-SilverLining.dylib.xml)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-SilverLining.so.windows.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-SilverLining.dll.xml)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-SilverLining.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-SilverLining.so.xml)
endif()

#Shaders for F+ and LogZ on Windows and Linux, currently we do not support F+ on the APPLE
if(NOT APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/SilverLining/WithForwardPlusAndLogZ/UserFunctions-frag.glsl DESTINATION $ENV{SILVERLINING_PATH}/Resources/Shaders)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/SilverLining/WithForwardPlusAndLogZ/UserFunctions.glsl DESTINATION $ENV{SILVERLINING_PATH}/Resources/Shaders)
endif()

#Original SL shaders are supported on APPLE, WIN32 and Linux
#for now we will install these on the APPLE by default until F+ is supported there
if(APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/SilverLining/Original/UserFunctions-frag.glsl DESTINATION $ENV{SILVERLINING_PATH}/Resources/Shaders)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/SilverLining/Original/UserFunctions.glsl DESTINATION $ENV{SILVERLINING_PATH}/Resources/Shaders)
endif()

if(WIN32)
    SET(INSTALL_BINDIR bin)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/SilverLining/WithForwardPlusAndLogZ/forward_plus_sl_ps.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/SilverLining/WithForwardPlusAndLogZ/forward_plus_sl_ps.glsl DESTINATION /usr/local/openig/resources/shaders)
endif()
