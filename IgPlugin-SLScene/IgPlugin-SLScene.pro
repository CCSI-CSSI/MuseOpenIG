#-------------------------------------------------
#
# Project created by QtCreator 2015-01-28T11:29:59
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent

TARGET = IgPlugin-SLScene
TEMPLATE = lib

DESTDIR = /usr/local/lib/igplugins

QMAKE_CXXFLAGS += -fpermissive -shared-libgcc

DEFINES += IGPLUGINSLSCENE_LIBRARY

SOURCES += igpluginslscene.cpp

HEADERS +=

unix {
    target.path = /usr/lib
    INSTALLS += target
}

LIBS += -losg -losgDB -losgViewer -lIgPluginCore -lCstShare -lOpenThreads -lIgCore

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include

INCLUDEPATH += ../
DEPENDPATH += ../

INCLUDEPATH += /usr/local/muse/core/inc
DEPENDPATH += /usr/local/muse/core/inc

INCLUDEPATH += /usr/local/lib64
DEPENDPATH += /usr/local/lib64
