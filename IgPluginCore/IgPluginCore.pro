#-------------------------------------------------
#
# Project created by QtCreator 2015-01-11T11:38:11
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent

TARGET = IgPluginCore
TEMPLATE = lib

DESTDIR = /usr/local/lib

QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL

DEFINES += IGPLUGINCORE_LIBRARY

SOURCES += \
    pluginhost.cpp

HEADERS += igplugincore.h \
    plugin.h \
    pluginhost.h \
    plugincontext.h \
    config.h \
    export.h \
    pluginoperation.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}


INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include

INCLUDEPATH += ../
DEPENDPATH += ../

INCLUDEPATH += /usr/lib64
DEPENDPATH += /usr/lib64

LIBS += -losg -losgDB -losgViewer -lOpenThreads -lIgCore -lboost_filesystem -lboost_system

unix:mac {
    LIBS += -lIgCore -lboost_filesystem -lboost_system
}

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
    BOOSTROOT = $$(BOOST_ROOT)
    isEmpty(BOOSTROOT) {
        message(\"boost\" not detected...)
    }
    else {
        message(\"boost\" detected in \"$$BOOSTROOT\")
        LIBS += -L$$BOOSTROOT/stage/lib
        INCLUDEPATH += $$BOOSTROOT/
    }
    OPENIGBUILD = $$(OPENIG_BUILD)
    isEmpty (OPENIGBUILD) {
        OPENIGBUILD = $$IN_PWD/..
    }
    DESTDIR = $$OPENIGBUILD/lib
    DLLDESTDIR = $$OPENIGBUILD/bin

    LIBS += -L$$OPENIGBUILD/lib
    LIBS += -lIgCore -lboost_filesystem -lboost_system -lstdc++.dll
}
