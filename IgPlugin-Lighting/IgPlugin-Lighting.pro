#-------------------------------------------------
#
# Project created by QtCreator 2015-03-05T11:20:12
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent

TARGET = IgPlugin-Lighting
TEMPLATE = lib

DEFINES += IGPLUGINLIGHTING_LIBRARY

SOURCES += igpluginlighting.cpp

DESTDIR = /usr/local/lib/igplugins

QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL

HEADERS +=

unix {
    target.path = /usr/lib
    INSTALLS += target
}

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgShadow -lIgCore -lIgPluginCore -losgUtil
INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include

INCLUDEPATH += ../
DEPENDPATH += ../

INCLUDEPATH += /usr/local/lib64
DEPENDPATH += /usr/local/lib64

OTHER_FILES += \
    libIgPlugin-Lighting.so.xml

FILE = $${PWD}/libIgPlugin-Lighting.so.xml
DDIR = $${DESTDIR}

mac: DDIR = $${DESTDIR}/libIgPlugin-Lighting.dylib.xml

QMAKE_POST_LINK = $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

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

    LIBS += -L$$OPENIGBUILD/lib
    LIBS += -lstdc++.dll

    FILE = $${PWD}/libIgPlugin-Lighting.so.xml
    DFILE = $${DLLDESTDIR}/IgPlugin-Lighting.dll.xml

    win32:FILE ~= s,/,\\,g
    win32:DFILE ~= s,/,\\,g

    QMAKE_POST_LINK = $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)
}
