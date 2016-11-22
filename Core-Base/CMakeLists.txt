SET( LIB_NAME OpenIG-Base )
SET( HEADER_PATH ${PROJECT_SOURCE_DIR}/Core-Base )
ADD_DEFINITIONS( -DIGCORE_LIBRARY )

SET( LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/Animation.h
    ${HEADER_PATH}/Types.h
    ${HEADER_PATH}/Commands.h
    ${HEADER_PATH}/Config.h
    ${HEADER_PATH}/Configuration.h
    ${HEADER_PATH}/Export.h
    ${HEADER_PATH}/FileSystem.h
    ${HEADER_PATH}/IDPool.h
    ${HEADER_PATH}/IGCore.h
    ${HEADER_PATH}/ImageGenerator.h
    ${HEADER_PATH}/Mathematics.h
    ${HEADER_PATH}/StringUtils.h    
)

SET( _IgCoreSourceFiles
    Animation.cpp
    Commands.cpp
    Configuration.cpp
    FileSystem.cpp
    IDPool.cpp
    ImageGenerator.cpp
    Mathematics.cpp
    StringUtils.cpp    
)

ADD_LIBRARY( ${LIB_NAME} SHARED
        ${LIB_PUBLIC_HEADERS}
        ${_IgCoreSourceFiles}
)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

TARGET_LINK_LIBRARIES( ${LIB_NAME}
        ${OSG_LIBRARIES}
	${OPENGL_LIBRARY}
	${Boost_LIBRARIES}
)

SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Core Base" )
SET_TARGET_PROPERTIES (${target} PROPERTIES DEFINE_SYMBOL  "IGCORE_LIBRARY")

INCLUDE( ModuleInstall REQUIRED )

INSTALL(
    FILES       ${PROJECT_SOURCE_DIR}/SDK-Readme.txt
    DESTINATION doc/
    COMPONENT libopenig-dev
)