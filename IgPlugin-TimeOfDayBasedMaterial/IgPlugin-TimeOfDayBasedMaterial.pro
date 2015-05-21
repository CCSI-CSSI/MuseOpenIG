#-------------------------------------------------
#
# Project created by QtCreator 2015-04-07T20:53:20
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent

TARGET = IgPlugin-TimeOfDayBasedMaterial
TEMPLATE = lib

DEFINES += IGPLUGINTIMEOFDAYBASEDMATERIAL_LIBRARY

SOURCES += igplugintimeofdaybasedmaterial.cpp

DESTDIR = /usr/local/lib/igplugins

QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL

HEADERS +=

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgShadow -losgSim -lIgCore -lIgPluginCore

unix {
    target.path = /usr/lib
    INSTALLS += target
}

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
}

OTHER_FILES += \
    master.flt.osg.xml


