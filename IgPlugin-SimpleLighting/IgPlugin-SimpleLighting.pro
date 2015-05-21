#-------------------------------------------------
#
# Project created by QtCreator 2015-02-17T22:54:05
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent

TARGET = IgPlugin-SimpleLighting
TEMPLATE = lib

DESTDIR = /usr/local/lib/igplugins

QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL

DEFINES += IGPLUGINSIMPLELIGHTING_LIBRARY

SOURCES += igpluginsimplelighting.cpp

HEADERS +=

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgShadow -lIgCore -lIgPluginCore

unix {
    target.path = /usr/lib
    INSTALLS += target
}

FILE = $${PWD}/libIgPlugin-SimpleLighting.so.xml
DDIR = $${DESTDIR}

mac: DDIR = $${DESTDIR}/libIgPlugin-SimpleLighting.dylib.xml

QMAKE_POST_LINK = $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include

INCLUDEPATH += ../
DEPENDPATH += ../

INCLUDEPATH += /usr/local/lib64
DEPENDPATH += /usr/local/lib64

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
    DLLDESTDIR = $$OPENIGBUILD/bin/igplugins

    LIBS += -L$$OPENIGBUILD/lib -lstdc++.dll

    FILE = $${PWD}/libIgPlugin-SimpleLighting.so.xml
    DFILE = $${DLLDESTDIR}/IgPlugin-SimpleLighting.dll.xml

    win32:FILE ~= s,/,\\,g
    win32:DFILE ~= s,/,\\,g

    QMAKE_POST_LINK = $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)
}

OTHER_FILES += \
    libIgPlugin-SimpleLighting.so.xml

