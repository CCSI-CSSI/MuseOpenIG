#-------------------------------------------------
#
# Project created by QtCreator 2015-03-29T20:22:26
#
#-------------------------------------------------

QT -= core gui

CONFIG += silent

TARGET = IgPlugin-OSGEarthSimpleLighting
TEMPLATE = lib

DEFINES += IGPLUGINOSGEARTHSIMPLELIGHTING_LIBRARY

SOURCES += igpluginosgearthsimplelighting.cpp

HEADERS +=

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgShadow -lIgCore -lIgPluginCore

INCLUDEPATH += ../
DEPENDPATH += ../

unix {
    !mac:contains(QMAKE_HOST.arch, x86_64):{
        DESTDIR = /usr/local/lib64/igplugins
        target.path = /usr/local/lib64/igplugins
    } else {
        DESTDIR = /usr/local/lib/igplugins
        target.path = /usr/local/lib/igplugins
    }
    message(Libs will be installed into $$DESTDIR)

    INSTALLS += target

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    LIBS += -losgEarth -losgEarthUtil

    # library version number files
    exists( "../openig_version.pri" ) {

	include( "../openig_version.pri" )
	isEmpty( VERSION ){ error( "bad or undefined VERSION variable inside file openig_version.pri" )
	} else {
	message( "Set version info to: $$VERSION" )
	}

    }
    else { error( "could not find pri library version file openig_version.pri" ) }

    # end of library version number files
}


win32-g++:QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL

win32 {

    OSGROOT = $$(OSG_ROOT)
    isEmpty(OSGROOT) {
        message(\"OpenSceneGraph\" not detected...)
    }
    else {
        message(\"OpenSceneGraph\" detected in \"$$OSGROOT\")
        INCLUDEPATH += $$OSGROOT/include
            LIBS += -L$$OSGROOT/lib
    }
    isEmpty(OSGROOT) {
        OSGBUILD = $$(OSG_BUILD)
        isEmpty(OSGBUILD) {
            message(\"OpenSceneGraph build\" not detected...)
        }
        else {
            message(\"OpenSceneGraph build\" detected in \"$$OSGBUILD\")
            DEPENDPATH += $$OSGBUILD/lib
            INCLUDEPATH += $$OSGBUILD/include
            LIBS += -L$$OSGBUILD/lib
        }
    }

    exists( C:/Program Files/OSGEARTH ) {
        OSGEARTH_ROOT = C:\Program Files\OSGEARTH
    }
    OSGEARTHROOT = $$(OSGEARTH_ROOT)
    isEmpty(OSGEARTHROOT) {
        message(\"osgEarth\" not detected...)
    }
    else {
        message(\"osgEarth\" detected in \"$$OSGEARTHROOT\")
        INCLUDEPATH += $$OSGEARTHROOT\include
        LIBS += -L$$OSGEARTHROOT
        LIBS += -L$$OSGEARTHROOT\lib
        LIBS += -losgEarth -losgEarthUtil
    }

# did not find osgEarth yet, check for build env var...
    isEmpty(OSGEARTHROOT) {
        OSGEARTHBUILD = $$(OSGEARTH_BUILD)
        isEmpty(OSGEARTHBUILD) {
            message(\"osgEarth build\" not detected...)
        }
        else {
            message(\"osgEarth build\" detected in \"$$OSGEARTHBUILD\")
            DEPENDPATH += $$OSGEARTHBUILD
            INCLUDEPATH += $$OSGBUILD\include
            LIBS += -L$$OSGEARTHBUILD\BUILD\lib\release
            LIBS += -losgEarth -losgEarthUtil
        }
    }

    OPENIGBUILD = $$(OPENIG_BUILD)
    isEmpty (OPENIGBUILD) {
        OPENIGBUILD = $$IN_PWD/..
    }
    message(\"openig build\" detected in \"$$OPENIGBUILD\")
    DESTDIR = $$OPENIGBUILD/lib
    DLLDESTDIR = $$OPENIGBUILD/bin/igplugins

    LIBS += -L$$OPENIGBUILD/lib
}
