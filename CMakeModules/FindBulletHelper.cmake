
SET( _bulletComponents
    BULLET_DYNAMICS BULLET_COLLISION BULLET_MATH BULLET_SOFTBODY
)

SET( _defaultBulletLocations
    "C:/Program Files/BULLET_PHYSICS"
)

# Macro to force the stock FindBullet.cmake script to
# search again for Bullet.
MACRO( unFindBullet )
#    MESSAGE( STATUS "In unFindBullet" )

    UNSET( BULLET_LIBRARIES CACHE )
    UNSET( BULLET_INCLUDE_DIR CACHE )
    UNSET( BULLET_INCLUDE_DIRS CACHE )
    foreach( currentLibIn ${_bulletComponents} )
        string( TOUPPER ${currentLibIn} currentLib )
        UNSET( "${currentLib}_INCLUDE_DIR" CACHE )
        UNSET( "${currentLib}_LIBRARIES" CACHE )
        UNSET( "${currentLib}_LIBRARY" CACHE )
        UNSET( "${currentLib}_LIBRARY_DEBUG" CACHE )
    endforeach() 
endmacro( unFindBullet )


# What type of Bullet should we look for? This is a combo box
# supporting three types of Bullet installations:
#  * Default installation (in which the stock FindBullet.cmake script does all the work)
#  * Alternate Install Location (user must SET the BulletInstallLocation variable)
#  * Source And Build Tree (user must supply both the BulletSourceRoot and BulletBuildRoot variables)
SET( BulletInstallType "Default Installation" CACHE STRING "Type of Bullet install: 'Default Installation', 'Alternate Install Location', or 'Source And Build Tree'." )
SET_property( CACHE BulletInstallType PROPERTY STRINGS "Default Installation" "Alternate Install Location" "Source And Build Tree" )

# We need to detect when the user changes the Bullet install type
# or any of the related directory variables, so that we'll know
# to call unFindBullet() and force the stock Bullet script to search
# again. To do this, we save the last SET value of these variables
# in the CMake cache as internal (hidden) variables.
IF( NOT DEFINED _lastBulletInstallType )
    SET( _lastBulletInstallType "empty" CACHE INTERNAL "" )
ENDIF()
IF( NOT DEFINED _lastBulletInstallLocation )
    SET( _lastBulletInstallLocation "empty" CACHE INTERNAL "" )
ENDIF()
IF( NOT DEFINED _lastBulletSourceRoot )
    SET( _lastBulletSourceRoot "empty" CACHE INTERNAL "" )
ENDIF()
IF( NOT DEFINED _lastBulletBuildRoot )
    SET( _lastBulletBuildRoot "empty" CACHE INTERNAL "" )
ENDIF()

IF( NOT DEFINED BulletInstallLocation )
    SET( BulletInstallLocation "Please specIFy" )
ENDIF()
IF( NOT DEFINED BulletSourceRoot )
    SET( BulletSourceRoot "Please specIFy" )
ENDIF()
IF( NOT DEFINED BulletBuildRoot )
    SET( BulletBuildRoot "Please specIFy" )
ENDIF()


# If the user has changed the Bullet install type combo box
# (or it's a clean cache), then SET or UNSET the our related
# Bullet directory search variables.
IF( NOT ( ${BulletInstallType} STREQUAL ${_lastBulletInstallType} ) )
#    MESSAGE( STATUS "NOT ( ${BulletInstallType} STREQUAL ${_lastBulletInstallType} )" )

    IF( BulletInstallType STREQUAL "Default Installation" )
        # Remove our helper variables and tell the stock script to search again.
        UNSET( BulletInstallLocation CACHE )
        UNSET( BulletSourceRoot CACHE )
        UNSET( BulletBuildRoot CACHE )
    ELSEIF( BulletInstallType STREQUAL "Alternate Install Location" )
        # Enable just the BulletInstallLocation helper variable.
        SET( BulletInstallLocation "Please specIFy" CACHE PATH "Root directory where Bullet is installed" )
        UNSET( BulletSourceRoot CACHE )
        UNSET( BulletBuildRoot CACHE )
    ELSEIF( BulletInstallType STREQUAL "Source And Build Tree" )
        # Enable the BulletSourceRoot and BulletBuildRoot helper variables.
        UNSET( BulletInstallLocation CACHE )
        SET( BulletSourceRoot "Please specIFy" CACHE PATH "Root directory of Bullet source tree" )
        SET( BulletBuildRoot "Please specIFy" CACHE PATH "Root directory of Bullet build tree" )
    ENDIF()
ENDIF()

# Library suffix needed on Windows to find libraries in build tree.
SET( _bulletLibraryPathSuffix "" )
SET( _bulletLibraryPathSuffixDebug "" )

# Suffix needed to find headers in a source tree.
SET( _bulletSourceSuffix "" )

# Look for conditions that require us to find Bullet again.
SET( _needToFindBullet FALSE )
IF( BulletInstallType STREQUAL "Default Installation" )
    IF( NOT ( ${BulletInstallType} STREQUAL ${_lastBulletInstallType} ) )
#        MESSAGE( STATUS "Need to find: case A" )
        SET( _needToFindBullet TRUE )
    ENDIF()
ELSEIF( BulletInstallType STREQUAL "Alternate Install Location" )
    IF( NOT ( "${BulletInstallLocation}" STREQUAL "${_lastBulletInstallLocation}" ) )
#        MESSAGE( STATUS "Need to find: case B" )
        SET( _needToFindBullet TRUE )
    ENDIF()
ELSEIF( BulletInstallType STREQUAL "Source And Build Tree" )
    IF( ( NOT ( "${BulletSourceRoot}" STREQUAL "${_lastBulletSourceRoot}" ) ) OR
        ( NOT ( "${BulletBuildRoot}" STREQUAL "${_lastBulletBuildRoot}" ) ) )
#        MESSAGE( STATUS "Need to find: cade C" )
        SET( _needToFindBullet TRUE )
    ENDIF()
    SET( _bulletSourceSuffix "/src" )
    IF( WIN32 )
        SET( _bulletLibraryPathSuffix "/lib/Release" )
        SET( _bulletLibraryPathSuffixDebug "/lib/Debug" )
    ENDIF()
ENDIF()
IF( _needToFindBullet )
    unFindBullet()
    SET( _lastBulletInstallType ${BulletInstallType} CACHE INTERNAL "" FORCE )
    SET( _lastBulletInstallLocation ${BulletInstallLocation} CACHE INTERNAL "" FORCE )
    SET( _lastBulletSourceRoot ${BulletSourceRoot} CACHE INTERNAL "" FORCE )
    SET( _lastBulletBuildRoot ${BulletBuildRoot} CACHE INTERNAL "" FORCE )
ENDIF()



# Save internal variables for later restoration
SET( CMAKE_PREFIX_PATH_SAVE ${CMAKE_PREFIX_PATH} )
SET( CMAKE_LIBRARY_PATH_SAVE ${CMAKE_LIBRARY_PATH} )

SET( CMAKE_PREFIX_PATH
    ${BulletInstallLocation}
    ${BulletSourceRoot}
    ${BulletSourceRoot}${_bulletSourceSuffix}
    ${BulletBuildRoot}
)
SET( CMAKE_LIBRARY_PATH
    ${BulletInstallLocation}
    ${BulletBuildRoot}
    ${BulletBuildRoot}${_bulletLibraryPathSuffix}
    ${BulletBuildRoot}${_bulletLibraryPathSuffixDebug}
)
IF( BulletInstallType STREQUAL "Default Installation" )
    list( APPEND CMAKE_PREFIX_PATH
        ${_defaultBulletLocations} )
    list( APPEND CMAKE_LIBRARY_PATH
        ${_defaultBulletLocations} )
ENDIF()


FIND_PACKAGE( Bullet )


# Restore internal variables
SET( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH_SAVE} )
SET( CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH_SAVE} )

# Mark Bullet variables as advanced
MARK_AS_ADVANCED( BULLET_INCLUDE_DIR )


# If we had to look for Bullet, *and* we found it,
# then let's see whether Bullet was built using
# double precision or not...
# By default, bullet is NOT built with double precision.
#
SET( OSGBULLET_USE_DOUBLE_PRECISION FALSE CACHE BOOL "Select to force compiling with -DBT_USE_DOUBLE_PRECISION." )
IF( _needToFindBullet AND BULLET_FOUND )
    MESSAGE( STATUS "Testing Bullet for use of double precision..." )
    SET( _result )
    SET( _buildOut )
    
    # Configure for the correct build type to allow successful VS 2010 links
    # IF Bullet was built release-only.
    IF( BULLET_DYNAMICS_LIBRARY )
        SET( CMAKE_TRY_COMPILE_CONFIGURATION Release )
    ELSE()
        SET( CMAKE_TRY_COMPILE_CONFIGURATION Debug )
    ENDIF()
    
    TRY_COMPILE( _result ${PROJECT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}/CMakeModules/bulletDoublePrecisionTest.cpp
        CMAKE_FLAGS
            "-DINCLUDE_DIRECTORIES:string=${BULLET_INCLUDE_DIRS}"
            "-DLINK_LIBRARIES:string=${BULLET_LIBRARIES}"
        COMPILE_DEFINITIONS
            "-DBT_USE_DOUBLE_PRECISION"
        OUTPUT_VARIABLE _buildOut
    )
    IF( _result )
        MESSAGE( STATUS "Bullet double precision detected. Automatically defining BT_USE_DOUBLE_PRECISION for osgBullet." )
        SET( OSGBULLET_USE_DOUBLE_PRECISION ON CACHE BOOL "" FORCE )
    ELSE()
        # Try it *without* -DBT_USE_DOUBLE_PRECISION to make sure it's single...
        SET( _result )
        SET( _buildOut )
        TRY_COMPILE( _result ${PROJECT_BINARY_DIR}
            ${PROJECT_SOURCE_DIR}/CMakeModules/bulletDoublePrecisionTest.cpp
            CMAKE_FLAGS
                "-DINCLUDE_DIRECTORIES:string=${BULLET_INCLUDE_DIRS}"
                "-DLINK_LIBRARIES:string=${BULLET_LIBRARIES}"
            OUTPUT_VARIABLE _buildOut
        )
        IF( _result )
            MESSAGE( STATUS "Bullet single precision detected. Not defining BT_USE_DOUBLE_PRECISION for osgBullet." )
            SET( OSGBULLET_USE_DOUBLE_PRECISION OFF CACHE BOOL "" FORCE )
        ELSE()
            MESSAGE( WARNING "Unable to determine single or double precision. Contact development staff." )
            MESSAGE( "Build output follows:" )
            MESSAGE( "${_buildOut}" )
        ENDIF()
    ENDIF()
ENDIF()
