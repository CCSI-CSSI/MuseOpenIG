# Locate CIGI Class Library.
#
# This script defines:
#   CCL_FOUND, set to 1 if found
#   CCL_LIBRARIES
#   CCL_INCLUDE_DIR
#


SET( CCL_BUILD_DIR "" CACHE PATH "If using ccl out of a source tree, specify the build directory." )
SET( CCL_SOURCE_DIR "" CACHE PATH "If using ccl out of a source tree, specify the root of the source tree." )
SET( CCL_ROOT "" CACHE PATH "Specify non-standard ccl install directory. It is the parent of the include and lib dirs." )

MACRO( FIND_CCL_INCLUDE THIS_CCL_INCLUDE_DIR THIS_CCL_INCLUDE_FILE )
    UNSET( ${THIS_CCL_INCLUDE_DIR} )
    MARK_AS_ADVANCED( ${THIS_CCL_INCLUDE_DIR} )
    FIND_PATH( ${THIS_CCL_INCLUDE_DIR} ${THIS_CCL_INCLUDE_FILE}
        PATHS
            ${CCL_ROOT}
            $ENV{CCL_ROOT}
            ${CCL_SOURCE_DIR}
            $ENV{CCL_SOURCE_DIR}
            /usr/local
            /usr
            /sw/ # Fink
            /opt/local # DarwinPorts
            /opt/csw # Blastwave
            /opt
            "C:/Program Files/ccl"
            "C:/Program Files (x86)/ccl"
            ~/Library/Frameworks
            /Library/Frameworks
        PATH_SUFFIXES
            include
            .
    )
ENDMACRO( FIND_CCL_INCLUDE THIS_CCL_INCLUDE_DIR THIS_CCL_INCLUDE_FILE )

FIND_CCL_INCLUDE( CCL_INCLUDE_DIR cigicl/CigiAllPackets.h )
message( STATUS ${CCL_INCLUDE_DIR} )

MACRO( FIND_CCL_LIBRARY MYLIBRARY MYLIBRARYNAME )
   # UNSET( ${MYLIBRARY} CACHE )
   # UNSET( ${MYLIBRARY}_debug CACHE )
    MARK_AS_ADVANCED( ${MYLIBRARY} )
    MARK_AS_ADVANCED( ${MYLIBRARY}_debug )
    FIND_LIBRARY( ${MYLIBRARY}
        NAMES
            ${MYLIBRARYNAME}
        PATHS
            ${CCL_ROOT}
            $ENV{CCL_ROOT}
            ${CCL_BUILD_DIR}
            $ENV{CCL_BUILD_DIR}
            ~/Library/Frameworks
            /Library/Frameworks
            /usr/local
            /usr
            /sw
            /opt/local
            /opt/csw
            /opt
            "C:/Program Files/ccl"
            "C:/Program Files (x86)/ccl"
            /usr/freeware/lib64
        PATH_SUFFIXES
            lib
            .
    )
    FIND_LIBRARY( ${MYLIBRARY}_debug
        NAMES
            ${MYLIBRARYNAME}d
        PATHS
            ${CCL_ROOT}
            $ENV{CCL_ROOT}
            ${CCL_BUILD_DIR}
            $ENV{CCL_BUILD_DIR}
            ~/Library/Frameworks
            /Library/Frameworks
            /usr/local
            /usr
            /sw
            /opt/local
            /opt/csw
            /opt
            "C:/Program Files/ccl"
            "C:/Program Files (x86)/ccl"
            /usr/freeware/lib64
        PATH_SUFFIXES
            lib
            .
    )
#    message( STATUS ${${MYLIBRARY}} ${${MYLIBRARY}_debug} )
#    message( STATUS ${MYLIBRARYNAME} )
    IF( ${MYLIBRARY} )
        SET( CCL_LIBRARIES ${CCL_LIBRARIES}
            "optimized" ${${MYLIBRARY}}
        )
    ENDIF( ${MYLIBRARY} )
    IF( ${MYLIBRARY}_debug )
        SET( CCL_LIBRARIES ${CCL_LIBRARIES}
            "debug" ${${MYLIBRARY}_debug}
        )
    ENDIF( ${MYLIBRARY}_debug )
ENDMACRO(FIND_CCL_LIBRARY LIBRARY LIBRARYNAME)

FIND_CCL_LIBRARY( CCL_LIBRARY cigicl )

SET( CCL_FOUND 0 )
IF( CCL_LIBRARIES AND CCL_INCLUDE_DIR )
    SET( CCL_FOUND 1 )
ENDIF( CCL_LIBRARIES AND CCL_INCLUDE_DIR )