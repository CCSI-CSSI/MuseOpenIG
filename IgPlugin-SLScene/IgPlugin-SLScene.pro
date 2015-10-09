#-------------------------------------------------
#
# Project created by QtCreator 2015-01-28T11:29:59
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent

TARGET = IgPlugin-SLScene
TEMPLATE = lib

DEFINES += IGPLUGINSLSCENE_LIBRARY

SOURCES += igpluginslscene.cpp

HEADERS +=

LIBS += -losg -losgDB -losgViewer -lIgPluginCore -lCstShare -lOpenThreads -lIgCore

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
    message($$TARGET Lib will be installed into $$DESTDIR)
    INSTALLS += target

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/local/muse/core/inc
    DEPENDPATH += /usr/local/muse/core/inc

    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    # library version number files
    exists( "../openig_version.pri" ) {

	include( "../openig_version.pri" )
        isEmpty( VERSION ){ error( "$$TARGET -- bad or undefined VERSION variable inside file openig_version.pri" )
	} else {
        message( "$$TARGET -- Set version info to: $$VERSION" )
	}

    }
    else { error( "$$TARGET -- could not find pri library version file openig_version.pri" ) }

    # end of library version number files
}

#In event we add Windows support will put this here...
#win32-g++:QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL
#win32-g++:LIBS += -lstdc++.dll
