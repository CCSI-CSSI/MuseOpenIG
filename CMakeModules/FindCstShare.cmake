# Find Compro's MUSE CSTSHARED SDK
# This module defines
# CSTSHARE_FOUND
# CSTSHARE_INCLUDE_DIR
# On windows:
#   UNSUPPORTED
# On other platforms
#   CSTSHARE_LIBRARY
#

IF (UNIX)
    SET(CSTSHARE_ARCH "linux")
#    MESSAGE(STATUS "Looking for CstShare!!")


    FIND_PATH(CSTSHARE_INCLUDE_DIR cstshareobject.h
        "$ENV{MUSE_ROOT}/core/inc/cstshare"
        "$ENV{MUSE_PATH}/core/inc/cstshare"
        $ENV{MUSE_ROOT}
        ${MUSE_DIR}/core/inc
        $ENV{MUSE_DIR}/core/inc
        $ENV{MUSE_DIR}
        /usr/local/include
        /usr/include
    )

#    IF (CSTSHARE_INCLUDE_DIR)
#        MESSAGE(STATUS "Found CSTSHARE_INCLUDE_DIR: ${CSTSHARE_INCLUDE_DIR}")
#    ELSE (CSTSHARE_INCLUDE_DIR )
#        MESSAGE(STATUS "Did not find CSTSHARE_INCLUDE_DIR: ${CSTSHARE_INCLUDE_DIR}")
#    ENDIF (CSTSHARE_INCLUDE_DIR)

    MACRO(FIND_CSTSHARE_LIBRARY MYLIBRARY MYLIBRARYNAME)

        FIND_LIBRARY(${MYLIBRARY}
        NAMES ${MYLIBRARYNAME}
        PATHS
            ${MUSE_DIR}/lib
            $ENV{MUSE_DIR}/lib
            $ENV{MUSE_DIR}
            $ENV{MUSE_PATH}/lib
            /usr/local/lib
            /usr/local/lib64
            /usr/lib
            /sw/lib
            /opt/local/lib
            /opt/csw/lib
            /opt/lib
            /usr/freeware/lib64
            PATH_SUFFIXES
            ${MUSE_ARCH}
        )

    ENDMACRO(FIND_CSTSHARE_LIBRARY MYLIBRARY MYLIBRARYNAME)

    FIND_CSTSHARE_LIBRARY(CSTSHARE_LIBRARY "CstShare")

    SET(CSTSHARE_FOUND FALSE)
    IF (CSTSHARE_INCLUDE_DIR AND CSTSHARE_LIBRARY)
       SET(CSTSHARE_FOUND TRUE)
    ELSE (CSTSHARE_INCLUDE_DIR AND CSTSHARE_LIBRARY)
       MESSAGE(STATUS "Did not Find: ${CSTSHARE_LIBRARY}")
    ENDIF (CSTSHARE_INCLUDE_DIR AND CSTSHARE_LIBRARY)

    IF (CSTSHARE_FOUND)
       IF (NOT CSTSHARE_FIND_QUIETLY)
          MESSAGE(STATUS "Found CstShare: ${CSTSHARE_LIBRARY}")
       ENDIF (NOT CSTSHARE_FIND_QUIETLY)
    ELSE (CSTSHARE_FOUND)
       IF (CSTSHARE_FIND_REQUIRED)
          MESSAGE(FATAL_ERROR "Could not find ${CSTSHARE_LIBRARY}")
       ENDIF (CSTSHARE_FIND_REQUIRED)
    ENDIF (CSTSHARE_FOUND)

ENDIF (UNIX)
