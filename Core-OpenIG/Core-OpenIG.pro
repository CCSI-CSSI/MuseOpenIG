#-------------------------------------------------
#
# Project created by QtCreator 2015-01-12T22:24:17
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent warn_off

TARGET = OpenIG-Engine
TEMPLATE = lib

DEFINES += OPENIG_LIBRARY

SOURCES +=  Commands.cpp\
            Help.cpp\
            Keypad.cpp\
            Lights.cpp\
            Engine.cpp\
            Splash.cpp\
            Terminal.cpp\
            Effects.cpp

HEADERS +=  Config.h\
            Export.h\
            Keypad.h\
            Engine.h\
            RenderBins.h


INCLUDEPATH += ../
DEPENDPATH += ../

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgGA -losgText -losgUtil\
        -losgShadow -losgSim -losgParticle -lOpenIG-Base -lOpenIG-PluginBase

OTHER_FILES += CMakeLists.txt
DISTFILES += CMakeLists.txt

unix {
    DEFINES += LINUX
    !mac:contains(QMAKE_HOST.arch, x86_64):{
        DESTDIR = /usr/local/lib64
        target.path = /usr/local/lib64
    } else {
        DESTDIR = /usr/local/lib
        target.path = /usr/local/lib
    }
    message($$TARGET Lib will be installed into $$DESTDIR)

    INSTALLS += target

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/lib64
    DEPENDPATH += /usr/lib64

    HFILES   = $$files($${PWD}/*.h)
    HDESTDIR = /usr/local/include/$$TARGET

#   Get the filename(only) list for distclean to remove only the files added from this plugin
#    for(var,HFILES) {
#        distfiles += $$HDESTDIR/$$basename(var)
#    }
#    QMAKE_DISTCLEAN += $$distfiles
    QMAKE_DISTCLEAN += -r $$HDESTDIR

    QMAKE_POST_LINK  = test -d $$quote($$HDESTDIR) || $$QMAKE_MKDIR $$quote($$HDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$HFILES) $$quote($$HDESTDIR) $$escape_expand(\\n\\t)

    SFILES   = $$files($${PWD}/../Resources/shaders/logz*.glsl)
    SDESTDIR = /usr/local/openig/resources/shaders/

#   Get the filename(only) list for distclean to remove only the files added from this plugin
    for(var,SFILES) {
        distfiles += $$SDESTDIR/$$basename(var)
    }
    QMAKE_DISTCLEAN += $$distfiles

    QMAKE_POST_LINK += test -d $$quote($$SDESTDIR) || $$QMAKE_MKDIR $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$SFILES) $$quote($$SDESTDIR) $$escape_expand(\\n\\t)

    # library version number files
    exists( "../openig_version.pri" ) {

    include( "../openig_version.pri" )
        isEmpty( VERSION ){ error( "$$TARGET -- bad or undefined VERSION variable inside file openig_version.pri" )
    } else {
        message( "$$TARGET -- Set version info to: $$VERSION" )
    }

    }
    else { error( "$$TARGET -- could not find pri library version file openig_version.pri" ) }

    # end of library version number files
}

win32-g++:QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL
win32-g++:LIBS += -lstdc++.dll

win32 {
    OSGROOT = $$(OSG_ROOT)
    isEmpty(OSGROOT) {
        message($$TARGET -- \"OpenSceneGraph\" not detected...)
    }
    else {
        message($$TARGET -- \"OpenSceneGraph\" detected in \"$$OSGROOT\")
        INCLUDEPATH += $$OSGROOT/include
        LIBS += -L$$OSGROOT/lib
    }
    OSGBUILD = $$(OSG_BUILD)
    isEmpty(OSGBUILD) {
        message($$TARGET -- \"OpenSceneGraph build\" not detected...)
    }
    else {
        message($$TARGET -- \"OpenSceneGraph build\" detected in \"$$OSGBUILD\")
        DEPENDPATH += $$OSGBUILD/lib
        INCLUDEPATH += $$OSGBUILD/include
        LIBS += -L$$OSGBUILD/lib
    }

    OPENIGBUILD = $$(OPENIG_BUILD)
    isEmpty (OPENIGBUILD) {
        OPENIGBUILD = $$IN_PWD/..
    }
    message($$TARGET -- \"openig build\" detected in \"$$OPENIGBUILD\")
    DESTDIR = $$OPENIGBUILD/lib
    DLLDESTDIR = $$OPENIGBUILD/bin

    LIBS += -L$$OPENIGBUILD/lib

    HFILES   = $$files($${PWD}/*.h)
    HDESTDIR = $$OPENIGBUILD/include/$$TARGET
    HFILES  ~= s,/,\\,g
    HDESTDIR  ~= s,/,\\,g

#   Get the filename(only) list for distclean to remove only the files added from this plugin
    for(var,HFILES) {
        distfiles += $$HDESTDIR\\$$basename(var)
    }
    QMAKE_DISTCLEAN += $$distfiles
    QMAKE_DISTCLEAN += -r $$HDESTDIR

    PDIR =  $$PWD
    PDIR  ~= s,/,\\,g

    QMAKE_POST_LINK  = test -d $$quote($$HDESTDIR) || $$QMAKE_MKDIR $$quote($$HDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$PDIR\*.h $$quote($$HDESTDIR) $$escape_expand(\\n\\t)

    SFILES   = $$files($${PWD}/../Resources/shaders/logz*.glsl)
    SDESTDIR = $$OPENIGBUILD/bin/resources/shaders/

    SFILES ~= s,/,\\,g
    SDESTDIR ~= s,/,\\,g

#   Get the filename(only) list for distclean to remove only the files added from this plugin
    for(var,SFILES) {
        distfiles += $$SDESTDIR\\$$basename(var)
    }
    QMAKE_DISTCLEAN += $$distfiles

    PDIR = $${PWD}/../Resources/shaders
    PDIR ~= s,/,\\,g

    QMAKE_POST_LINK += if not exist $$quote($$SDESTDIR) $$QMAKE_MKDIR $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PDIR\logz*.glsl) $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
}
