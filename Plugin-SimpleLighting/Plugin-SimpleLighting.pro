#-------------------------------------------------
#
# Project created by QtCreator 2015-02-17T22:54:05
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent warn_off

TARGET = OpenIG-Plugin-SimpleLighting
TEMPLATE = lib

DEFINES += IGPLUGINSIMPLELIGHTING_LIBRARY

SOURCES += IGPluginSimpleLighting.cpp

HEADERS +=

INCLUDEPATH += ../
DEPENDPATH += ../

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgShadow -losgGA -losgText -losgUtil \
        -lOpenIG-Engine -lOpenIG-Base -lOpenIG-PluginBase

OTHER_FILES += libIgPlugin-SimpleLighting.so.xml

OTHER_FILES += CMakeLists.txt
DISTFILES += CMakeLists.txt

OTHER_FILES += \
    $${PWD}/DataFiles/* \
    $${PWD}/CmakeLists.txt

unix {
    !mac:contains(QMAKE_HOST.arch, x86_64):{
        DESTDIR = /usr/local/lib64/plugins
        target.path = /usr/local/lib64/plugins
    } else {
        DESTDIR = /usr/local/lib/plugins
        target.path = /usr/local/lib/plugins
    }
    message(Libs will be installed into $$DESTDIR)

    INSTALLS += target

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include
    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    FILE      = $${PWD}/DataFiles/libIgPlugin-SimpleLighting.so.xml
    DDIR      = $${DESTDIR}/libOpenIG-Plugin-SimpleLighting.so.xml
    mac: DDIR = $${DESTDIR}/libOpenIG-Plugin-SimpleLighting.dylib.xml

    QMAKE_POST_LINK =  test -d $$quote($$DESTDIR) || $$QMAKE_MKDIR $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DDIR) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    #!build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$DDIR$$escape_expand(\n))
    QMAKE_DISTCLEAN += $${DDIR}

    SFILES   = $$files($${PWD}/../Resources/shaders/simplelighting_*.glsl)
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
    isEmpty( VERSION ){ error( "bad or undefined VERSION variable inside file openig_version.pri" )
    } else {
    message( "Set version info to: $$VERSION" )
    }

    }
    else { error( "could not find pri library version file openig_version.pri" ) }

    # end of library version number files
}

win32-g++:QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL

win32 {
    OSGROOT = $$(OSG_ROOT)
    isEmpty(OSGROOT) {
        message(\"OpenSceneGraph\" not detected...)
    }
    else {
        message(\"OpenSceneGraph\" detected in \"$$OSGROOT\")
        INCLUDEPATH += $$OSGROOT/include
        LIBS += -L$$OSGROOT/lib
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
    DLLDESTDIR = $$OPENIGBUILD/bin/plugins

    LIBS += -L$$OPENIGBUILD/lib # -lstdc++.dll

    FILE = $${PWD}/DataFiles/libIgPlugin-SimpleLighting.so.xml
    DFILE = $${DLLDESTDIR}/OpenIG-Plugin-SimpleLighting.dll.xml

    FILE ~= s,/,\\,g
    DFILE ~= s,/,\\,g

    QMAKE_POST_LINK =  if not exist $$quote($$DLLDESTDIR) $$QMAKE_MKDIR $$quote($$DLLDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += if not exist $$quote($$DFILE) copy /y $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    SFILES   = $$files($${PWD}/../Resources/shaders/simplelighting_*.glsl)
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
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PDIR\simplelighting_*.glsl) $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
}

