cmake_minimum_required( VERSION 2.8.8 )
project( OpenIG )

set( OPENIG_VERSION 2.0.3 )
set( OPENIG_MAJOR_VERSION 2 )
set( OPENIG_MINOR_VERSION 0 )
set( OPENIG_SUB_VERSION 3 )

set( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}" )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib )
if( WIN32 )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
    add_definitions( -D_WIN32_WINNT=0x0501 )
else()
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} )
endif()

set(DEF_INSTALL_CMAKE_DIR cmake)
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")

IF( WIN32 AND MSVC )
    OPTION( OPENIG_WIN32_USE_MP "Build with multiple processes (/MP option; VS 2005 and above)." ON )
    MARK_AS_ADVANCED( OPENIG_WIN32_USE_MP )
    IF( OPENIG_WIN32_USE_MP )
        SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP" )
    ENDIF( OPENIG_WIN32_USE_MP )
ENDIF( WIN32 AND MSVC )

if(CMAKE_COMPILER_IS_GNUCXX)
    include(GNUInstallDirs)
endif(CMAKE_COMPILER_IS_GNUCXX)

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR lib)
endif(NOT DEFINED CMAKE_INSTALL_LIBDIR)

if (NOT "$ENV{OPENIG_BUILD}" STREQUAL "")
    SET(CMAKE_INSTALL_PREFIX "$ENV{OPENIG_BUILD}" CACHE PATH "OpeinIG install prefix" FORCE)
endif()

include( FindOSGHelper )
include( FindBoostHelper )
include( FindSilverLiningHelper )
include( FindOsgEarthHelper )
include( FindBulletHelper )
include( FindOSGBullet )
include( FindCstShare )
include( FindMersive )
#include( FindJRMSensors )
include( FindCCL )

find_package(OpenGL)
find_package(Triton)
#To enable CUDA GPU culling, you must uncomment the next line
#and ensure that you have set the XML directive: "UseGPUAcceleration"
#to "yes" in your OpenIG openig.xml config/setup file.
#
#find_package(CUDA)

#If you plan to use Intel TBB, set TBB_ROOT_DIR as appropriate
#find_package(TBB)

INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}
    ${OSG_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/Core-Base
    ${PROJECT_SOURCE_DIR}/Core-PluginBase
    ${PROJECT_SOURCE_DIR}/Core-OpenIG
    ${PROJECT_SOURCE_DIR}/Core-Utils
)

ADD_SUBDIRECTORY( Core-Base )
ADD_SUBDIRECTORY( Core-PluginBase )
ADD_SUBDIRECTORY( Core-OpenIG )
ADD_SUBDIRECTORY( Core-Utils )

ADD_SUBDIRECTORY( Application-A320 )

ADD_SUBDIRECTORY( Utility-veggen )
ADD_SUBDIRECTORY( Utility-vegviewer )
ADD_SUBDIRECTORY( Utility-oigconv )

ADD_SUBDIRECTORY( Plugin-Animation )
ADD_SUBDIRECTORY( Plugin-GPUVegetation )
ADD_SUBDIRECTORY( Plugin-ForwardPlusLighting )
ADD_SUBDIRECTORY( Plugin-ModelComposition )
ADD_SUBDIRECTORY( Plugin-SkyDome )
ADD_SUBDIRECTORY( Plugin-VDBOffset )
ADD_SUBDIRECTORY( Plugin-FBXAnimation )
ADD_SUBDIRECTORY( Plugin-OSGParticleEffects )
ADD_SUBDIRECTORY( Plugin-LightsControl )
ADD_SUBDIRECTORY( Plugin-Networking )
ADD_SUBDIRECTORY( Plugin-SimpleLighting)

ADD_SUBDIRECTORY( Library-Graphics )
ADD_SUBDIRECTORY( Library-Networking )
ADD_SUBDIRECTORY( Library-Protocol )

IF( MERSIVE_FOUND )
 ADD_SUBDIRECTORY( Plugin-Mersive )
ENDIF( MERSIVE_FOUND )

IF( CSTSHARE_FOUND )
    ADD_SUBDIRECTORY( Plugin-Muse )
ENDIF( CSTSHARE_FOUND )

IF( SILVERLINING_FOUND )
    ADD_SUBDIRECTORY( Plugin-SilverLining )
ENDIF( SILVERLINING_FOUND )

IF( TRITON_FOUND )
    ADD_SUBDIRECTORY( Plugin-Triton )
ENDIF( TRITON_FOUND )

IF( JRMSENSORS_FOUND )
    ADD_SUBDIRECTORY( Plugin-JRMSensors )
ENDIF( JRMSENSORS_FOUND )

IF( CCL_FOUND )
    ADD_SUBDIRECTORY( Plugin-CIGI )
ENDIF( CCL_FOUND )

IF( OSGEARTH_FOUND )
    INCLUDE_DIRECTORIES(
        ${OSGEARTH_INCLUDE_DIR}
    )
    ADD_SUBDIRECTORY( Application-Earth )
    ADD_SUBDIRECTORY( Plugin-OSGEarthSimpleLighting )
ENDIF( OSGEARTH_FOUND )

IF( BULLET_FOUND AND OSGBULLET_FOUND )
    ADD_SUBDIRECTORY( Library-Bullet )
    ADD_SUBDIRECTORY( Plugin-Bullet )
    ADD_SUBDIRECTORY( Application-Bullet)
ENDIF( BULLET_FOUND AND OSGBULLET_FOUND)

FIND_PATH(MYGUI_INCLUDE_DIR MYGUI/MyGUI.h)
FIND_LIBRARY(MYGUI_LIBRARY NAMES MyGUIEngine)
FIND_LIBRARY(MYGUI_OPENGL_LIBRARY NAMES MyGUI.OpenGLPlatform)

IF(MYGUI_INCLUDE_DIR AND MYGUI_LIBRARY)
    ADD_SUBDIRECTORY( Plugin-UI )
ENDIF(MYGUI_INCLUDE_DIR AND MYGUI_LIBRARY)

install(EXPORT openig-targets DESTINATION ${CMAKE_INSTALL_LIBDIR})

export(PACKAGE OpenIG)

#file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")

configure_file(OpenIGConfig.cmake.in "${PROJECT_BINARY_DIR}/OpenIGConfig.cmake" @ONLY)
set(CONF_INCLUDE_DIRS "\${OPENIG_CMAKE_DIR}/${REL_INCLUDE_DIR}")

configure_file(OpenIGConfig.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/OpenIGConfig.cmake" @ONLY)

configure_file(OpenIGConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/OpenIGConfigVersion.cmake" @ONLY)


install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/OpenIGConfig.cmake"
  "${PROJECT_BINARY_DIR}/OpenIGConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

install(EXPORT openig-targets DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
if( WIN32 )
  ADD_CUSTOM_TARGET(UNINSTALL
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
else()
  ADD_CUSTOM_TARGET(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
endif()

