#-------------------------------------------------
#
# Project created by QtCreator 2015-03-29T20:22:26
#
#-------------------------------------------------

QT -= core gui

CONFIG += silent

TARGET = OpenIG-Plugin-OSGEarthSimpleLighting
TEMPLATE = lib

DEFINES += IGPLUGINOSGEARTHSIMPLELIGHTING_LIBRARY

SOURCES += igpluginosgearthsimplelighting.cpp

HEADERS +=

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgShadow\
        -lOpenIG-Engine -lOpenIG-Base -lOpenIG-PluginBase

INCLUDEPATH += ../
DEPENDPATH += ../

OTHER_FILES += CMakeLists.txt
DISTFILES += CMakeLists.txt

unix {
    !mac:contains(QMAKE_HOST.arch, x86_64):{
        DESTDIR = /usr/local/lib64/plugins
        target.path = /usr/local/lib64/plugins
    } else {
        DESTDIR = /usr/local/lib/plugins
        target.path = /usr/local/lib/plugins
    }
    !build_pass:message($$basename(_PRO_FILE_) Lib will be installed into $$DESTDIR)

    INSTALLS += target

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    LIBS += -losgEarth -losgEarthUtil

    # library version number files
    exists( "../openig_version.pri" ) {

	include( "../openig_version.pri" )
        isEmpty( VERSION ){ !build_pass:error($$basename(_PRO_FILE_) -- bad or undefined VERSION variable inside file openig_version.pri)
	} else {
        !build_pass:message($$basename(_PRO_FILE_) -- Set version info to: $$VERSION)
	}

    }
    else { !build_pass:error($$basename(_PRO_FILE_) -- could not find pri library version file openig_version.pri) }

    # end of library version number files
}


win32-g++:QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL

win32 {

    OSGROOT = $$(OSG_ROOT)
    isEmpty(OSGROOT) {
        !build_pass:message($$basename(_PRO_FILE_) -- \"OpenSceneGraph\" not detected...)
    }
    else {
        !build_pass:message($$basename(_PRO_FILE_) -- \"OpenSceneGraph\" detected in \"$$OSGROOT\")
        INCLUDEPATH += $$OSGROOT/include
            LIBS += -L$$OSGROOT/lib
    }
    isEmpty(OSGROOT) {
        OSGBUILD = $$(OSG_BUILD)
        isEmpty(OSGBUILD) {
            !build_pass:message($$basename(_PRO_FILE_) -- \"OpenSceneGraph build\" not detected...)
        }
        else {
            !build_pass:message($$basename(_PRO_FILE_) -- \"OpenSceneGraph build\" detected in \"$$OSGBUILD\")
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
        !build_pass:message($$basename(_PRO_FILE_) -- \"osgEarth\" not detected...)
    }
    else {
        !build_pass:message($$basename(_PRO_FILE_) -- \"osgEarth\" detected in \"$$OSGEARTHROOT\")
        INCLUDEPATH += $$OSGEARTHROOT\include
        LIBS += -L$$OSGEARTHROOT
        LIBS += -L$$OSGEARTHROOT\lib
        LIBS += -losgEarth -losgEarthUtil
    }

# did not find osgEarth yet, check for build env var...
    isEmpty(OSGEARTHROOT) {
        OSGEARTHBUILD = $$(OSGEARTH_BUILD)
        isEmpty(OSGEARTHBUILD) {
            !build_pass:message($$basename(_PRO_FILE_) -- \"osgEarth build\" not detected...)
        }
        else {
           !build_pass:message($$basename(_PRO_FILE_) -- \"osgEarth build\" detected in \"$$OSGEARTHBUILD\")
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
    !build_pass:message($$basename(_PRO_FILE_) -- \"openig build\" detected in \"$$OPENIGBUILD\")
    DESTDIR = $$OPENIGBUILD/lib
    DLLDESTDIR = $$OPENIGBUILD/bin/plugins

    LIBS += -L$$OPENIGBUILD/lib
}
