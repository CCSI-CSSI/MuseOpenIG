SET( LIB_NAME OpenIG-Plugin-ForwardPlusLighting )

SET (HEADER_FILES
            DummyLight.h
            ForwardPlusCullVisitor.h
            ForwardPlusEngine.h
            ForwardPlusLightImplementationCallback.h
            OSGtoFPUtils.h
            LightManagerStateAttribute.h
)

SET (SOURCE_FILES 
            DummyLight.cpp
            ForwardPlusCullVisitor.cpp
            ForwardPlusEngine.cpp
            ForwardPlusLightImplementationCallback.cpp
            IGPluginForwardPlusLighting.cpp
            LightManagerStateAttribute.cpp
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

ADD_LIBRARY( ${LIB_NAME} SHARED
	${HEADER_FILES} 
	${SOURCE_FILES}
	${SHADER_FILES}
	Plugin-ForwardPlusLighting.pro
	DataFiles/libIgPlugin-ForwardPlusLighting.so.xml
)

INCLUDE_DIRECTORIES(
		${Boost_INCLUDE_DIRS}
)
#$ENV{VLD_ROOT}/include

TARGET_LINK_LIBRARIES( ${LIB_NAME}
    ${OSG_LIBRARIES}
    ${OPENGL_LIBRARY}
    ${GLU_LIBRARY}
    OpenIG-Engine
	OpenIG-Graphics
	OpenIG-Utils
	${Boost_LIBRARIES}
)
#$ENV{VLD_ROOT}/lib/Win64/vld.lib

SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Plugin ForwardPlusLighting" )

INCLUDE( PluginInstall REQUIRED )

IF (APPLE)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-ForwardPlusLighting.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-ForwardPlusLighting.dylib.xml)
ELSEIF(WIN32)
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-ForwardPlusLighting.so.xml DESTINATION ${INSTALL_BINDIR} RENAME OpenIG-Plugin-ForwardPlusLighting.dll.xml)
ELSE()
    INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/DataFiles/libIgPlugin-ForwardPlusLighting.so.xml DESTINATION ${INSTALL_LIBDIR} RENAME libOpenIG-Plugin-ForwardPlusLighting.so.xml)
ENDIF()

SET(INSTALL_BINDIR bin)
IF (WIN32)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/forwardplus_vs.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/forwardplus_ps.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/forwardplus_math.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/forwardplus_preamble.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/shadow_vs.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/shadow_ps.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/lighting_math.glsl DESTINATION ${INSTALL_BINDIR}/resources/shaders)
ELSE()
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/forwardplus_vs.glsl DESTINATION /usr/local/openig/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/forwardplus_ps.glsl DESTINATION /usr/local/openig/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/forwardplus_math.glsl DESTINATION /usr/local/openig/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/forwardplus_preamble.glsl DESTINATION /usr/local/openig/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/shadow_vs.glsl DESTINATION /usr/local/openig/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/shadow_ps.glsl DESTINATION /usr/local/openig/resources/shaders)
	INSTALL(FILES ${CMAKE_CURRENT_LIST_DIR}/../Resources/shaders/lighting_math.glsl DESTINATION /usr/local/openig/resources/shaders)
ENDIF()
