#-------------------------------------------------
#
# Project created by QtCreator 2015-01-12T22:24:17
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent

TARGET = OpenIG
TEMPLATE = lib

DESTDIR = /usr/local/lib

QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL

DEFINES += OPENIG_LIBRARY
DEFINES += LINUX

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

unix {
    target.path = /usr/lib
    INSTALLS += target
}

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgGA -losgText -losgShadow -lIgCore -lIgPluginCore

FILE = $${PWD}/openig.xml
DDIR = $${DESTDIR}/igdata

QMAKE_POST_LINK = $$QMAKE_CHK_DIR_EXISTS $$quote($$DDIR) $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
QMAKE_POST_LINK = $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

FILE = $${PWD}/OpenIG-Splash.jpg
DDIR = $${DESTDIR}/igdata

QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include

INCLUDEPATH += /usr/lib64
DEPENDPATH += /usr/lib64

INCLUDEPATH += ../
DEPENDPATH += ../

OTHER_FILES += \
    openig.xml

FILE = $${PWD}/openig.xml
DDIR = $${DESTDIR}

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
    LIBS += -lIgCore -lstdc++.dll

    FILE = $${PWD}/openig.xml
    DFILE = $${OPENIGBUILD}/bin/igdata/openig.xml
    DDIR = $${OPENIGBUILD}/bin/igdata

    win32:FILE ~= s,/,\\,g
    win32:DFILE ~= s,/,\\,g
    win32:DDIR ~= s,/,\\,g

    QMAKE_POST_LINK = $$QMAKE_CHK_DIR_EXISTS $$quote($$DDIR) $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)

    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    FILE = $${PWD}/OpenIG-Splash.jpg
    DFILE = $${OPENIGBUILD}/bin/igdata/OpenIG-Splash.jpg

    win32:FILE ~= s,/,\\,g
    win32:DFILE ~= s,/,\\,g

    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)
}

DISTFILES += \
    OpenIG-Splash.jpg

