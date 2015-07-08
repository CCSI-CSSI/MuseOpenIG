
set( _osgEarthComponents
    osgEarth osgEarthFeatures osgEarthUtil osgEarthQt osgEarthSymbology osgEarthAnnotation
)

set( _defaultosgEarthLocations
    "C:/Program Files/osgEarth"
)

macro( unFindosgEarth )
#    message( STATUS "In unFindosgEarth" )

    unset( OSGEARTH_LIBRARIES CACHE )
    unset( OSGEARTH_INCLUDE_DIR CACHE )
    unset( OSGEARTH_INCLUDE_DIRS CACHE )
    foreach( currentLibIn ${_osgEarthComponents} )
        string( TOUPPER ${currentLibIn} currentLib )
        unset( "${currentLib}_INCLUDE_DIR" CACHE )
        unset( "${currentLib}_LIBRARIES" CACHE )
        unset( "${currentLib}_LIBRARY" CACHE )
        unset( "${currentLib}_LIBRARY_DEBUG" CACHE )
    endforeach() 
endmacro( unFindosgEarth )

set( osgEarthInstallType "Default Installation" CACHE STRING "Type of osgEarth install: 'Default Installation', 'Alternate Install Location', or 'Source And Build Tree'." )
set_property( CACHE osgEarthInstallType PROPERTY STRINGS "Default Installation" "Alternate Install Location" "Source And Build Tree" )

if( NOT DEFINED _lastosgEarthInstallType )
    set( _lastosgEarthInstallType "empty" CACHE INTERNAL "" )
endif()
if( NOT DEFINED _lastosgEarthInstallLocation )
    set( _lastosgEarthInstallLocation "empty" CACHE INTERNAL "" )
endif()
if( NOT DEFINED _lastosgEarthSourceRoot )
    set( _lastosgEarthSourceRoot "empty" CACHE INTERNAL "" )
endif()
if( NOT DEFINED _lastosgEarthBuildRoot )
    set( _lastosgEarthBuildRoot "empty" CACHE INTERNAL "" )
endif()

if( NOT DEFINED osgEarthInstallLocation )
    set( osgEarthInstallLocation "Please specify" )
endif()
if( NOT DEFINED osgEarthSourceRoot )
    set( osgEarthSourceRoot "Please specify" )
endif()
if( NOT DEFINED osgEarthBuildRoot )
    set( osgEarthBuildRoot "Please specify" )
endif()

if( NOT ( ${osgEarthInstallType} STREQUAL ${_lastosgEarthInstallType} ) )
#    message( STATUS "NOT ( ${osgEarthInstallType} STREQUAL ${_lastosgEarthInstallType} )" )

    if( osgEarthInstallType STREQUAL "Default Installation" )
        # Remove our helper variables and tell the stock script to search again.
        unset( osgEarthInstallLocation CACHE )
        unset( osgEarthSourceRoot CACHE )
        unset( osgEarthBuildRoot CACHE )
    elseif( osgEarthInstallType STREQUAL "Alternate Install Location" )
        # Enable just the osgEarthInstallLocation helper variable.
        set( osgEarthInstallLocation "Please specify" CACHE PATH "Root directory where osgEarth is installed" )
        unset( osgEarthSourceRoot CACHE )
        unset( osgEarthBuildRoot CACHE )
    elseif( osgEarthInstallType STREQUAL "Source And Build Tree" )
        # Enable the osgEarthSourceRoot and osgEarthBuildRoot helper variables.
        unset( osgEarthInstallLocation CACHE )
        set( osgEarthSourceRoot "Please specify" CACHE PATH "Root directory of osgEarth source tree" )
        set( osgEarthBuildRoot "Please specify" CACHE PATH "Root directory of osgEarth build tree" )
    endif()
endif()

# Library suffix needed on Windows to find libraries in build tree.
set( _osgEarthLibraryPathSuffix "" )
set( _osgEarthLibraryPathSuffixDebug "" )

# Suffix needed to find headers in a source tree.
set( _osgEarthSourceSuffix "" )

# Look for conditions that require us to find osgEarth again.
set( _needToFindosgEarth FALSE )
if( osgEarthInstallType STREQUAL "Default Installation" )
    if( NOT ( ${osgEarthInstallType} STREQUAL ${_lastosgEarthInstallType} ) )
#        message( STATUS "Need to find: case A" )
        set( _needToFindosgEarth TRUE )
    endif()
elseif( osgEarthInstallType STREQUAL "Alternate Install Location" )
    if( NOT ( "${osgEarthInstallLocation}" STREQUAL "${_lastosgEarthInstallLocation}" ) )
#        message( STATUS "Need to find: case B" )
        set( _needToFindosgEarth TRUE )
    endif()
elseif( osgEarthInstallType STREQUAL "Source And Build Tree" )
    if( ( NOT ( "${osgEarthSourceRoot}" STREQUAL "${_lastosgEarthSourceRoot}" ) ) OR
        ( NOT ( "${osgEarthBuildRoot}" STREQUAL "${_lastosgEarthBuildRoot}" ) ) )
#        message( STATUS "Need to find: cade C" )
        set( _needToFindosgEarth TRUE )
    endif()
    set( _osgEarthSourceSuffix "/src" )
    if( WIN32 )
        set( _osgEarthLibraryPathSuffix "/lib/Release" )
        set( _osgEarthLibraryPathSuffixDebug "/lib/Debug" )
    endif()
endif()
if( _needToFindosgEarth )
    unFindosgEarth()
    set( _lastosgEarthInstallType ${osgEarthInstallType} CACHE INTERNAL "" FORCE )
    set( _lastosgEarthInstallLocation ${osgEarthInstallLocation} CACHE INTERNAL "" FORCE )
    set( _lastosgEarthSourceRoot ${osgEarthSourceRoot} CACHE INTERNAL "" FORCE )
    set( _lastosgEarthBuildRoot ${osgEarthBuildRoot} CACHE INTERNAL "" FORCE )
endif()



# Look for conditions that require us to find OSG again.
set( _needToFindosgEarth FALSE )
if( osgEarthInstallType STREQUAL "Default Installation" )
    if( NOT ( ${osgEarthInstallType} STREQUAL ${_lastosgEarthInstallType} ) )
#        message( STATUS "Need to find: case A" )
        set( _needToFindosgEarth TRUE )
    endif()
elseif( osgEarthInstallType STREQUAL "Alternate Install Location" )
    if( NOT ( "${osgEarthInstallLocation}" STREQUAL "${_lastosgEarthInstallLocation}" ) )
#        message( STATUS "Need to find: case B" )
        set( _needToFindosgEarth TRUE )
    endif()
elseif( osgEarthInstallType STREQUAL "Source And Build Tree" )
    if( ( NOT ( "${osgEarthSourceRoot}" STREQUAL "${_lastosgEarthSourceRoot}" ) ) OR
        ( NOT ( "${osgEarthBuildRoot}" STREQUAL "${_lastosgEarthBuildRoot}" ) ) )
#        message( STATUS "Need to find: cade C" )
        set( _needToFindosgEarth TRUE )
    endif()
endif()
if( _needToFindosgEarth )
    unFindosgEarth()
    set( _lastosgEarthInstallType ${osgEarthInstallType} CACHE INTERNAL "" FORCE )
    set( _lastosgEarthInstallLocation ${osgEarthInstallLocation} CACHE INTERNAL "" FORCE )
    set( _lastosgEarthSourceRoot ${osgEarthSourceRoot} CACHE INTERNAL "" FORCE )
    set( _lastosgEarthBuildRoot ${osgEarthBuildRoot} CACHE INTERNAL "" FORCE )
endif()



# Save internal variables for later restoration
set( CMAKE_PREFIX_PATH_SAVE ${CMAKE_PREFIX_PATH} )
set( CMAKE_LIBRARY_PATH_SAVE ${CMAKE_LIBRARY_PATH} )

set( CMAKE_PREFIX_PATH
    ${osgEarthInstallLocation}
    ${osgEarthSourceRoot}
    ${osgEarthBuildRoot} )
set( CMAKE_LIBRARY_PATH
    ${osgEarthInstallLocation}
    ${osgEarthBuildRoot} )

set( osgEarth_DEBUG 0 )
set( osgEarth_MARK_AS_ADVANCED 1 )
find_package( osgEarth COMPONENTS ${_osgEarthComponents} )
if( OSGEARTH_FOUND )
    foreach( currentLibIn ${_osgComponents} )
        string( TOUPPER ${currentLibIn} currentLib )
        string( TOUPPER ${currentLibIn}d currentLibDebug )
        list( APPEND _tempLibraries ${${currentLib}_LIBRARIES} ${${currentLibDebug}_LIBRARIES} )
        list( APPEND _tempLibrariesDebug ${${currentLibDebug}_LIBRARIES} )
        list( APPEND _tempIncludeDirs ${${currentLib}_INCLUDE_DIR} )
    endforeach()
    list( REMOVE_DUPLICATES _tempIncludeDirs )
    set( OSGEARTH_LIBRARIES ${_tempLibraries} )
    set( OSGEARTH_LIBRARIES_DEBUG ${_tempLibrariesDebug} )
    set( OSGEARTH_INCLUDE_DIRS ${_tempIncludeDirs} )

    if( osgEarthInstallType STREQUAL "Source And Build Tree" )
        # Hm, the OSGBuildRoot seems to be left out of the include path,
        # even though that's where the Config headers are located. Add it:
        set( OSGEARTH_INCLUDE_DIRS ${OSGEARTH_INCLUDE_DIRS} "${osgEarthBuildRoot}/include" )
    endif()
endif()


# Restore internal variables
set( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH_SAVE} )
set( CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH_SAVE} )




