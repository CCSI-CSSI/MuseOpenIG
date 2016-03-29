#-------------------------------------------------
#
# Project created by QtCreator 2015-01-28T11:29:59
#
#-------------------------------------------------

QT       -= core gui
QT       += xml network

TARGET = OpenIG-Plugin-Mersive
TEMPLATE = lib

DEFINES += IGPLUGINMERSIVE_LIBRARY

SOURCES += igpluginmersive.cpp

HEADERS +=

LIBS += -losg -losgDB -losgViewer -lCstShare -lOpenThreads -lOpenIG-Engine

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
    message($$TARGET Lib will be installed into $$DESTDIR)
    INSTALLS += target

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/local/muse/core/inc
    DEPENDPATH += /usr/local/muse/core/inc

    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

##mersive
    exists( /mersive/lib/libMersive* ) {
         message( "Configuring Plugin-Mersive for build..." )
         DEFINES += COMPILE_MERSIVE
         INCLUDEPATH += /mersive/include
         LIBS += -ljpeg -lMersiveCalibrator -lEwbRemoteRendererDeviceProxy -lremoterenderer -L/mersive/lib/
         SOURCES += MersiveOsgWarperCallback.cpp \
                    IniFile.cpp
         HEADERS += fgmersivelib.h
    }

    FILE = $${PWD}/DataFiles/libIgPlugin-Mersive.so.xml
    DDIR = $${DESTDIR}/libOpenIG-Plugin-Mersive.so.xml

    mac:DDIR = $${DESTDIR}/libOpenIG-Plugin-Mersive.dylib.xml

    QMAKE_DISTCLEAN += $$DDIR

    QMAKE_POST_LINK =  test -d $$quote($$DESTDIR) || $$QMAKE_MKDIR $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DDIR) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

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

#win32-g++:QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL
#win32-g++:LIBS += -lstdc++.dll
#In event we add Windows support will put this here...
win32 {
     message( "Currently there is no Windows support for $$TARGET" )
}
