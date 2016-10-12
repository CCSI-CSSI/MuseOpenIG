set( LIB_NAME OpenIG-Plugin-ForwardPlusLighting )


set (HEADER_FILES
	dummylight.h
	forwardpluscullvisitor.h
	forwardplusengine.h
	forwardpluslightimplementationcallback.h
	osgtofputils.h
	lightmanagerstateattribute.h
)

set (SOURCE_FILES 
	dummylight.cpp
	forwardpluscullvisitor.cpp
	forwardplusengine.cpp
	forwardpluslightimplementationcallback.cpp
	igpluginforwardpluslighting.cpp
	lightmanagerstateattribute.cpp
)

FILE (GLOB SHADER_FILES 
	../Resources/shaders/forwardplus_vs.glsl
	../Resources/shaders/forwardplus_ps.glsl
	../Resources/shaders/forwardplus_math.glsl
	../Resources/shaders/forwardplus_preamble.glsl
	../Resources/shaders/shadow_vs.glsl
	../Resources/shaders/shadow_ps.glsl
	../Resources/shaders/lighting_math.glsl
)

SOURCE_GROUP(Shaders FILES ${SHADER_FILES})

add_library( ${LIB_NAME} SHARED
	${HEADER_FILES} 
	${SOURCE_FILES}
	${SHADER_FILES}
	Plugin-ForwardPlusLighting.pro
	DataFiles/libIgPlugin-ForwardPlusLighting.so.xml
)

include_directories(
		${Boost_INCLUDE_DIRS}
)
#$ENV{VLD_ROOT}/include

target_link_libraries( ${LIB_NAME}
    ${OSG_LIBRARIES}
    ${OPENGL_LIBRARY}
    ${GLU_LIBRARY}
    OpenIG-Engine
	OpenIG-Graphics
	OpenIG-Utils
	${Boost_LIBRARIES}
)
#$ENV{VLD_ROOT}/lib/Win64/vld.lib

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin ForwardPlusLighting" )

include( PluginInstall REQUIRED )

if (APPLE)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-ForwardPlusLighting.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-ForwardPlusLighting.dylib.xml)
elseif(WIN32)
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-ForwardPlusLighting.so.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-ForwardPlusLighting.dll.xml)
else()
    install(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-ForwardPlusLighting.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-ForwardPlusLighting.so.xml)
endif()

SET(INSTALL_BINDIR bin)
if (WIN32)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/forwardplus_vs.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/forwardplus_ps.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/forwardplus_math.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/forwardplus_preamble.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/shadow_vs.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/shadow_ps.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/lighting_math.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
else()
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/forwardplus_vs.glsl DESTINATION /usr/local/openig/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/forwardplus_ps.glsl DESTINATION /usr/local/openig/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/forwardplus_math.glsl DESTINATION /usr/local/openig/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/forwardplus_preamble.glsl DESTINATION /usr/local/openig/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/shadow_vs.glsl DESTINATION /usr/local/openig/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/shadow_ps.glsl DESTINATION /usr/local/openig/resources/shaders)
	install(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/lighting_math.glsl DESTINATION /usr/local/openig/resources/shaders)
endif()
