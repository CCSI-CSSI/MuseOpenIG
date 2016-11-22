#-------------------------------------------------
#
# Project created by QtCreator 2015-03-03T17:38:56
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent warn_off

TARGET = OpenIG-Plugin-GPUVegetation
TEMPLATE = lib

DEFINES += IGPLUGINGPUVEGETATION_LIBRARY

SOURCES += IGPluginGPUVegetation.cpp

HEADERS +=

LIBS += -losg -losgDB -losgViewer -lOpenIG-Engine -lOpenThreads -lOpenIG-PluginBase -lOpenIG-Base

INCLUDEPATH += ../
DEPENDPATH += ../

OTHER_FILES += CMakeLists.txt
DISTFILES += CMakeLists.txt

OTHER_FILES += $${PWD}/DataFiles/*
OTHER_FILES += $${PWD}/shaders/*

unix {
    !mac:contains(QMAKE_HOST.arch, x86_64):{
        DESTDIR = /usr/local/lib64/plugins
        target.path = /usr/local/lib64/plugins
    } else {
        DESTDIR = /usr/local/lib/plugins
        target.path = /usr/local/lib/plugins
    }
    message($$TARGET Lib will be installed into $$DESTDIR)

    INSTALLS += target

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include
    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    INCLUDEPATH += /usr/local/lib
    DEPENDPATH += /usr/local/lib

    FILE = $${PWD}/DataFiles/libIgPlugin-GPUVegetation.so.xml
    DDIR = $${DESTDIR}/libOpenIG-Plugin-GPUVegetation.so.xml
    mac:DDIR = $${DESTDIR}/libOpenIG-Plugin-GPUVegetation.dylib.xml

    QMAKE_POST_LINK =  test -d $$quote($$DESTDIR) || $$QMAKE_MKDIR $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    #!build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$DDIR$$escape_expand(\n))
    QMAKE_DISTCLEAN += $${DDIR}

    SFILES   = $${PWD}/shaders/*.glsl
    SDESTDIR = /usr/local/openig/resources/shaders/

    QMAKE_POST_LINK += test -d $$quote($$SDESTDIR) || $$QMAKE_MKDIR $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$SFILES) $$quote($$SDESTDIR) $$escape_expand(\\n\\t)

#   Get the filename(only) list for distclean to remove only the files added from this plugin
    for(var,SFILES) {
        distfiles += $$SDESTDIR/$$basename(var)
    }
    QMAKE_DISTCLEAN += $$distfiles

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
    DESTDIR = $$OPENIGBUILD/lib
    DLLDESTDIR = $$OPENIGBUILD/bin/plugins

    LIBS += -L$$OPENIGBUILD/lib

    FILE = $${PWD}/DataFiles/libIgPlugin-GPUVegetation.so.xml
    DFILE = $${DLLDESTDIR}/OpenIG-Plugin-GPUVegetation.dll.xml

    FILE ~= s,/,\\,g
    DFILE ~= s,/,\\,g

    QMAKE_DISTCLEAN += $$DFILE
    message(Plugin-GPUVegetation copying $$FILE to $$DFILE)
    QMAKE_POST_LINK =  if not exist $$quote($$DLLDESTDIR) $$QMAKE_MKDIR $$quote($$DLLDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    #!build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$DDIR$$escape_expand(\n))

    SFILES   = $$files($${PWD}/shaders/*.glsl)
    SDESTDIR = $$(OPENIG_BUILD)/bin/resources/shaders/

    SFILES   ~= s,/,\\,g
    SDESTDIR ~= s,/,\\,g
    PDIR = $${PWD}/shaders
    PDIR ~= s,/,\\,g

    #!build_pass:message("SDESTDIR: $$SDESTDIR")
    QMAKE_POST_LINK += test -d $$quote($$SDESTDIR) || $$QMAKE_MKDIR $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$PDIR\*.glsl $$quote($$SDESTDIR) $$escape_expand(\\n\\t)

#   Get the filename(only) list for distclean to remove only the files added from this plugin
    for(var,SFILES) {
        distfiles += $$SDESTDIR\\$$basename(var)
    }
    #!build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$distfiles$$escape_expand(\n))
    QMAKE_DISTCLEAN += $$distfiles
}
