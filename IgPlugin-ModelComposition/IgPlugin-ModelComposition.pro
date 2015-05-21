#-------------------------------------------------
#
# Project created by QtCreator 2015-03-15T21:26:59
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent

TARGET = IgPlugin-ModelComposition
TEMPLATE = lib

DEFINES += IGPLUGINMODELCOMPOSITION_LIBRARY

SOURCES += igpluginmodelcomposition.cpp \
    LightPointDrawable.cpp \
    LightPointSpriteDrawable.cpp

DESTDIR = /usr/local/lib/igplugins

QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL

HEADERS += \
    LightPointDrawable.h \
    lightpointnode.h \
    LightPointSpriteDrawable.h

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgShadow -losgSim -losgUtil -lIgCore -lIgPluginCore

FILE = $${PWD}/libIgPlugin-ModelComposition.so.xml
DDIR = $${DESTDIR}

mac: DDIR = $${DESTDIR}/libIgPlugin-ModelComposition.dylib.xml

QMAKE_POST_LINK = $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

unix {
    target.path = /usr/lib
    INSTALLS += target
}

unix:!mac {
LIBS += -lGL -lGLU
}

win32:!mac{
 LIBS += -lopengl32 -lglu32
}

mac: LIBS += -framework openGL

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

    LIBS += -L$$OPENIGBUILD/lib
    LIBS += -lstdc++.dll

    FILE = $${PWD}/libIgPlugin-ModelComposition.so.xml
    DFILE = $${DLLDESTDIR}/IgPlugin-ModelComposition.dll.xml

    win32:FILE ~= s,/,\\,g
    win32:DFILE ~= s,/,\\,g

    QMAKE_POST_LINK = $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)
}

OTHER_FILES += \
    a320.obj.xml \
    libIgPlugin-ModelComposition.so.xml
