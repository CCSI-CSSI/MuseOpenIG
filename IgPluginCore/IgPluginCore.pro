#-------------------------------------------------
#
# Project created by QtCreator 2015-01-11T11:38:11
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent

TARGET = IgPluginCore
TEMPLATE = lib

DEFINES += IGPLUGINCORE_LIBRARY

SOURCES += pluginhost.cpp

HEADERS += igplugincore.h \
    plugin.h \
    pluginhost.h \
    plugincontext.h \
    config.h \
    export.h \
    pluginoperation.h

INCLUDEPATH += ../
DEPENDPATH += ../

LIBS += -losg -losgDB -losgViewer -lOpenThreads -lIgCore

unix {
    !mac:contains(QMAKE_HOST.arch, x86_64):{
        DESTDIR = /usr/local/lib64
        target.path = /usr/local/lib64
        message(Libs will be installed into /usr/local/lib64)
    } else {
        DESTDIR = /usr/local/lib
        target.path = /usr/local/lib
        message(Libs will be installed into /usr/local/lib)
    }

    INSTALLS += target

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/lib64
    DEPENDPATH += /usr/lib64

    LIBS += -lboost_filesystem -lboost_system

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
    win32-g++:QMAKE_CXXFLAGS += -march=i686

    OSGROOT = $$(OSG_ROOT)
    isEmpty(OSGROOT) {
        message(\"OpenSceneGraph\" not detected...)
    }
    else {
        message(\"OpenSceneGraph\" detected in \"$$OSGROOT\")
        INCLUDEPATH += $$OSGROOT/include
    }
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

    BOOSTROOT = E:\OpenSceneGraph-3.3.7\3rdparty
    isEmpty(BOOSTROOT) {
        message(\"boost\" not detected...)
    }
    else {
        win32-g++ {
        message(\"boost\" detected in \"$$BOOSTROOT\")
        LIBS += -L$$BOOSTROOT\lib -llibboost_filesystem -llibboost_system
        INCLUDEPATH += $$BOOSTROOT\include\
        } else {
        message(\"boost\" detected in \"$$BOOSTROOT\")
	LIBS += -L$$BOOSTROOT\lib -llibboost_filesystem-vc120-mt-1_55 -llibboost_system-vc120-mt-1_55
	INCLUDEPATH += $$BOOSTROOT\include\
        }
    }
    OPENIGBUILD = $$(OPENIG_BUILD)
    isEmpty (OPENIGBUILD) {
	OPENIGBUILD = $$IN_PWD/..
    }
    LIBS += -L$$OPENIGBUILD/lib

    DESTDIR = $$OPENIGBUILD/lib
    DLLDESTDIR = $$OPENIGBUILD/bin
}
