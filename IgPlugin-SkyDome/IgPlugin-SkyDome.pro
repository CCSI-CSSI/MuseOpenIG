#-------------------------------------------------
#
# Project created by QtCreator 2015-02-04T13:03:21
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent

TARGET = IgPlugin-SkyDome
TEMPLATE = lib

DEFINES += IGPLUGINSKYDOME_LIBRARY

QMAKE_CXXFLAGS += -fpermissive -shared-libgcc

SOURCES += igpluginskydome.cpp

DESTDIR = /usr/local/lib/igplugins

HEADERS +=

unix {
    target.path = /usr/lib
    INSTALLS += target
}

CONFIG -= warn_on silent

LIBS += -losg -losgDB -losgViewer -losgParticle -lIgPluginCore -lIgCore -lOpenThreads

FILE = $${PWD}/libIgPlugin-SkyDome.so.xml
DDIR = $${DESTDIR}

mac: DDIR = $${DESTDIR}/libIgPlugin-SkyDome.dylib.xml

QMAKE_POST_LINK = $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include

INCLUDEPATH += ../
DEPENDPATH += ../

INCLUDEPATH += /usr/local/lib64
DEPENDPATH += /usr/local/lib64

OTHER_FILES += \
    libIgPlugin-SkyDome.so.xml \
    skydome.osgb.tar.gz

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

    FILE = $${PWD}/libIgPlugin-SkyDome.so.xml
    DFILE = $${DLLDESTDIR}/IgPlugin-SkyDome.dll.xml

    win32:FILE ~= s,/,\\,g
    win32:DFILE ~= s,/,\\,g

    QMAKE_POST_LINK = $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)


}
