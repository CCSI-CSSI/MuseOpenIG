#-------------------------------------------------
#
# Project created by QtCreator 2015-01-12T22:24:17
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent

TARGET = OpenIG
TEMPLATE = lib

DEFINES += OPENIG_LIBRARY

SOURCES += \
    openig.cpp \
    terminal.cpp \
    commands.cpp \
    lights.cpp \
    keypad.cpp \
    help.cpp \
    splash.cpp

HEADERS += openig.h \
    config.h \
    export.h \
    keypad.h

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgGA -losgText -losgShadow -lIgCore -lIgPluginCore

INCLUDEPATH += ../
DEPENDPATH += ../

unix {
    DEFINES += LINUX
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

    FILE = $${PWD}/openig.xml
    DDIR = $${DESTDIR}/igdata
    DFILE = $${DESTDIR}/igdata/openig.xml

    QMAKE_POST_LINK = test -d $$quote($$DDIR) || $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DFILE) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    FILE = $${PWD}/OpenIG-Splash.jpg
    DDIR = $${DESTDIR}/igdata
    DFILE = $${DESTDIR}/igdata/OpenIG-Splash.jpg

    QMAKE_POST_LINK += test -e $$quote($$DFILE) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/lib64
    DEPENDPATH += /usr/lib64

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
win32-g++:LIBS += -lstdc++.dll

win32 {
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

    OPENIGBUILD = $$(OPENIG_BUILD)
    isEmpty (OPENIGBUILD) {
        OPENIGBUILD = $$IN_PWD/..
    }
    DESTDIR = $$OPENIGBUILD/lib
    DLLDESTDIR = $$OPENIGBUILD/bin

    LIBS += -L$$OPENIGBUILD/lib

    FILE = $${PWD}/openig.xml
    DFILE = $${OPENIGBUILD}/bin/igdata/openig.xml
    DDIR = $${OPENIGBUILD}/bin/igdata

    FILE ~= s,/,\\,g
    DFILE ~= s,/,\\,g
    DDIR ~= s,/,\\,g

    QMAKE_POST_LINK = if not exist  $$quote($$DDIR) $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += if not exist $$quote($$DFILE) copy /y $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    FILE = $${PWD}/OpenIG-Splash.jpg
    DFILE = $${OPENIGBUILD}/bin/igdata/

    FILE ~= s,/,\\,g
    DFILE ~= s,/,\\,g

    QMAKE_POST_LINK += if not exist $$quote($$DFILE/OpenIG-Splash.jpg) copy /y $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)
}

OTHER_FILES += openig.xml

DISTFILES += OpenIG-Splash.jpg
