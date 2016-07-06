# - Config file for the OpenIG package
# It defines the following variables
#  OPENIG_INCLUDE_DIR  - include directorie for OpenIG
#  OPENIG_LIBRARIES    - libraries to link against

MACRO( FIND_OPENIG_INCLUDE THIS_OPENIG_INCLUDE_DIR THIS_OPENIG_INCLUDE_FILE )
    UNSET( ${THIS_OPENIG_INCLUDE_DIR} )
    MARK_AS_ADVANCED( ${THIS_OPENIG_INCLUDE_DIR} )
    FIND_PATH( ${THIS_OPENIG_INCLUDE_DIR} ${THIS_OPENIG_INCLUDE_FILE}
        PATHS
            ${OPENIG_ROOT}
            $ENV{OPENIG_ROOT}
            ${OPENIG_SOURCE_DIR}
            $ENV{OPENIG_SOURCE_DIR}
            /usr/local
			/usr/local/include
			/usr/include
            /usr
            /sw/ # Fink
            /opt/local # DarwinPorts
            /opt/csw # Blastwave
            /opt
            "C:/Program Files/OpenIG"
            "C:/Program Files (x86)/OpenIG"
            ~/Library/Frameworks
            /Library/Frameworks
        PATH_SUFFIXES
            include
            .
    )
ENDMACRO( FIND_OPENIG_INCLUDE THIS_OPENIG_INCLUDE_DIR THIS_OPENIG_INCLUDE_FILE )

MACRO( FIND_OPENIG_LIBRARY MYLIBRARY MYLIBRARYNAME )
   # UNSET( ${MYLIBRARY} CACHE )
    MARK_AS_ADVANCED( ${MYLIBRARY} )
    FIND_LIBRARY( ${MYLIBRARY}
        NAMES
            ${MYLIBRARYNAME}
        PATHS
			${OPENIG_INSTALL_DIR}/lib
            ${OPENIG_ROOT}
            $ENV{OPENIG_ROOT}
            ${OPENIG_BUILD_DIR}
            $ENV{OPENIG_BUILD_DIR}
            ~/Library/Frameworks
            /Library/Frameworks
			/usr/local/lib
            /usr/local/lib64
            /usr/lib
            /sw/lib
            /opt/local/lib
            /opt/csw/lib
            /opt/lib
            /usr/freeware/lib64
            /usr/local
            /usr
            /sw
            /opt/local
            /opt/csw
            /opt
            "C:/Program Files/OpenIG"
            "C:/Program Files (x86)/OpenIG"
            /usr/freeware/lib64
        PATH_SUFFIXES
            lib
            .
    )
    
#    message( STATUS ${MYLIBRARYNAME} )
    IF( ${MYLIBRARY} )
        SET( OPENIG_LIBRARIES ${OPENIG_LIBRARIES}
            ${${MYLIBRARY}}
        )
    ENDIF( ${MYLIBRARY} )    
ENDMACRO(FIND_OPENIG_LIBRARY LIBRARY LIBRARYNAME)
 
# Compute paths
set(OPENIG_INSTALL_DIR "" CACHE PATH "OpenIG install directory")

if (NOT "${OPENIG_INSTALL_DIR}" STREQUAL "")
	set(CMAKE_INSTALL_PREFIX "${OPENIG_INSTALL_DIR}" CACHE PATH "" FORCE)

	set(OPENIG_INCLUDE_DIR "${OPENIG_INSTALL_DIR}/include" CACHE PATH "OpeinIG Headers Directory" FORCE)

	FIND_OPENIG_LIBRARY( OPENIG_LIBRARY_BASE OpenIG-Base )
	FIND_OPENIG_LIBRARY( OPENIG_LIBRARY_PLUGINBASE OpenIG-PluginBase )
	FIND_OPENIG_LIBRARY( OPENIG_LIBRARY_ENGINE OpenIG-Engine )
	FIND_OPENIG_LIBRARY( OPENIG_LIBRARY_NETWORKING OpenIG-Networking )
	FIND_OPENIG_LIBRARY( OPENIG_LIBRARY_PROTOCOL OpenIG-Protocol)
else()
	FIND_OPENIG_INCLUDE( OPENIG_INCLUDE_DIR OpenIG-Base/imagegenerator.h )

	FIND_OPENIG_LIBRARY( OPENIG_LIBRARY_BASE OpenIG-Base )
	FIND_OPENIG_LIBRARY( OPENIG_LIBRARY_PLUGINBASE OpenIG-PluginBase )
	FIND_OPENIG_LIBRARY( OPENIG_LIBRARY_ENGINE OpenIG-Engine )
	FIND_OPENIG_LIBRARY( OPENIG_LIBRARY_NETWORKING OpenIG-Networking )
	FIND_OPENIG_LIBRARY( OPENIG_LIBRARY_PROTOCOL OpenIG-Protocol)
endif()
 
