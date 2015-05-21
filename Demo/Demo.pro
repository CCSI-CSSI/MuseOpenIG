TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

TARGET = openig-demo

SOURCES += main.cpp

include(deployment.pri)
qtcAddDeployment()

DESTDIR = /usr/local/bin

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgGA -losgText -lOpenIG -lIgCore

INCLUDEPATH += /usr/local/include
DEPENDPATH += /usr/local/include

INCLUDEPATH += ../
DEPENDPATH += ../

INCLUDEPATH += /usr/local/lib64
DEPENDPATH += /usr/local/lib64

INCLUDEPATH += /usr/lib64
DEPENDPATH += /usr/lib64

OTHER_FILES += \
    default.txt

unix:!mac {
    LIBS += -lboost_filesystem -lboost_system -lX11
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

    OPENIGBUILD = $$(OPENIG_BUILD)
    isEmpty (OPENIGBUILD) {
        OPENIGBUILD = $$IN_PWD/..
    }
    DESTDIR = $$OPENIGBUILD/bin

    LIBS += -L$$OPENIGBUILD/lib -lstdc++.dll

    FILE = $${PWD}/default.txt
    DDIR = $$DESTDIR/demo

    win32:FILE ~= s,/,\\,g
    win32:DDIR ~= s,/,\\,g

    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    FILE = $${PWD}/flightpath.path
    DDIR = $$DESTDIR/demo

    win32:FILE ~= s,/,\\,g
    win32:DDIR ~= s,/,\\,g

    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

}

DISTFILES += \
    default.txt \
    flightpath.bin \
    flightpath.path

