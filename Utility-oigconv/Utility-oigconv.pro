TEMPLATE = app

TARGET = oigconv

CONFIG += console silent warn_off
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += oigconv.cpp OrientationConverter.cpp

HEADERS += OrientationConverter.h

include(deployment.pri)
qtcAddDeployment()

LIBS += -losg -losgDB -losgViewer -losgGA -lOpenThreads -losgUtil -losgSim

INCLUDEPATH += ../
DEPENDPATH += ../

OTHER_FILES += CMakeLists.txt
DISTFILES += CMakeLists.txt

unix {
    DESTDIR = /usr/local/bin

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    INCLUDEPATH += /usr/lib64
    DEPENDPATH += /usr/lib64

    # library version number files
    exists( "../openig_version.pri" ) {

    include( "../openig_version.pri" )
        isEmpty( VERSION ){ !build_pass:error($$basename(_PRO_FILE_) -- bad or undefined VERSION variable inside file openig_version.pri)
    } else {
        !build_pass:message($$basename(_PRO_FILE_) -- Set version info to: $$VERSION)
    }

    }
    else { !build_pass:error($$basename(_PRO_FILE_) -- could not find pri library version file openig_version.pri) }

    # end of library version number files
}

win32-g++:QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL
win32-g++:LIBS += -lstdc++.dll

win32 {
    OPENIGBUILD = $$(OPENIG_BUILD)
    isEmpty (OPENIGBUILD) {
        OPENIGBUILD = $$IN_PWD/..
    }
    DESTDIR = $$OPENIGBUILD/bin

    OSGROOT = $$(OSG_ROOT)
    isEmpty(OSGROOT) {
        !build_pass:message($$basename(_PRO_FILE_) -- \"OpenSceneGraph\" not detected...)
    }
    else {
        !build_pass:message($$basename(_PRO_FILE_) -- \"OpenSceneGraph\" detected in \"$$OSGROOT\")
        INCLUDEPATH += $$OSGROOT/include
        LIBS += -L$$OSGROOT/lib
    }
    OSGBUILD = $$(OSG_BUILD)
    isEmpty(OSGBUILD) {
        !build_pass:message($$basename(_PRO_FILE_) -- \"OpenSceneGraph build\" not detected...)
    }
    else {
        !build_pass:message($$basename(_PRO_FILE_) -- \"OpenSceneGraph build\" detected in \"$$OSGBUILD\")
        DEPENDPATH += $$OSGBUILD/lib
        INCLUDEPATH += $$OSGBUILD/include
        LIBS += -L$$OSGBUILD/lib
    }

}
