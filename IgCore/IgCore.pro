#-------------------------------------------------
#
# Project created by QtCreator 2015-01-11T13:46:53
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent

DESTDIR = /usr/local/lib

TARGET = IgCore
TEMPLATE = lib

QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL

DEFINES += IGCORE_LIBRARY

SOURCES += \
    commands.cpp \
    stringutils.cpp \
    mathematics.cpp \
    configuration.cpp \
    imagegenerator.cpp \
    animation.cpp \
    globalidgenerator.cpp

HEADERS += igcore.h \
    export.h \
    config.h \
    imagegenerator.h \
    commands.h \
    stringutils.h \
    mathematics.h \
    attributes.h \
    configuration.h \
    animation.h \
    globalidgenerator.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += ../
DEPENDPATH += ../

INCLUDEPATH += /usr/local/lib64
DEPENDPATH += /usr/local/lib64

LIBS += -losg -losgDB -losgViewer -lOpenThreads

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

    LIBS += -lstdc++.dll

}
