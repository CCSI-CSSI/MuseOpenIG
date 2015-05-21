#-------------------------------------------------
#
# Project created by QtCreator 2015-03-03T17:38:56
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent

TARGET = IgPlugin-GPUVegetation
TEMPLATE = lib

DEFINES += IGPLUGINGPUVEGETATION_LIBRARY

SOURCES += igplugingpuvegetation.cpp

QMAKE_CXXFLAGS += -fpermissive -shared-libgcc

DESTDIR = /usr/local/lib/igplugins

HEADERS +=

unix {
    target.path = /usr/lib
    INSTALLS += target
}

LIBS += -losg -losgDB -losgViewer -lIgPluginCore -lIgCore -lOpenThreads

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include

INCLUDEPATH += ../
DEPENDPATH += ../

INCLUDEPATH += /usr/local/lib64
DEPENDPATH += /usr/local/lib64

INCLUDEPATH += /usr/local/lib
DEPENDPATH += /usr/local/lib

OTHER_FILES += \
    Readme.txt

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
