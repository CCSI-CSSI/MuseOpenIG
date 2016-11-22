#-------------------------------------------------
#
# Project created by QtCreator 2015-02-04T13:03:21
#
#-------------------------------------------------

QT     -= core gui
CONFIG -= warn_on
CONFIG +=  silent warn_off

TARGET   = OpenIG-Plugin-SkyDome
TEMPLATE = lib

DEFINES += IGPLUGINSKYDOME_LIBRARY

SOURCES += IGPluginSkyDome.cpp

HEADERS +=

INCLUDEPATH += ../
DEPENDPATH += ../

LIBS += -losg -losgDB -losgViewer -losgParticle -lOpenThreads\
      -lOpenIG-Engine -lOpenIG-PluginBase -lOpenIG-Base

OTHER_FILES += CMakeLists.txt
DISTFILES += CMakeLists.txt

OTHER_FILES += \
    $${PWD}/DataFiles/libIgPlugin-SkyDome.so.xml \
    $${PWD}/DataFiles/skydome.osgb.tar.gz

unix {
    !mac:contains(QMAKE_HOST.arch, x86_64):{
        DESTDIR = /usr/local/lib64/plugins
        target.path = /usr/local/lib64/plugins
    } else {
        DESTDIR = /usr/local/lib/plugins
        target.path = /usr/local/lib/plugins
    }
    !build_pass:message($$basename(_PRO_FILE_) Lib will be installed into $$DESTDIR)

    INSTALLS += target

    FILE = $${PWD}/DataFiles/libIgPlugin-SkyDome.so.xml
    DDIR = $${DESTDIR}/libOpenIG-Plugin-SkyDome.so.xml
    mac:DDIR = $${DESTDIR}/libOpenIG-Plugin-SkyDome.dylib.xml

    QMAKE_POST_LINK =  test -d $$quote($$DESTDIR) || $$QMAKE_MKDIR $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DDIR) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    #!build_pass:message("$$escape_expand(\n)Files to be removed during \"make distclean\": "$$DDIR$$escape_expand(\n))
    QMAKE_DISTCLEAN += $${DDIR}

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    # library version number files
    exists( "../openig_version.pri" ) {

    include( "../openig_version.pri" )
        isEmpty( VERSION ){ !build_pass:error($$basename(_PRO_FILE_)  -- bad or undefined VERSION variable inside file openig_version.pri)
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

    OPENIGBUILD = $$(OPENIG_BUILD)
    isEmpty (OPENIGBUILD) {
        OPENIGBUILD = $$IN_PWD/..
    }
    !build_pass:message($$basename(_PRO_FILE_) -- \"openig build\" detected in \"$$OPENIGBUILD\")
    DESTDIR = $$OPENIGBUILD/lib
    DLLDESTDIR = $$OPENIGBUILD/bin/plugins

    DDIR = $${DLLDESTDIR}
    DDIR ~= s,/,\\,g
    QMAKE_PRE_LINK =  if not exist $$quote($$DDIR) $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)

    LIBS += -L$$OPENIGBUILD/lib

    FILE = $${PWD}/DataFiles/libIgPlugin-SkyDome.so.xml
    DFILE = $${DLLDESTDIR}/OpenIG-Plugin-SkyDome.dll.xml

    FILE ~= s,/,\\,g
    DFILE ~= s,/,\\,g

    QMAKE_POST_LINK = if not exist $$quote($$DFILE) copy /y $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    !build_pass:message("$$escape_expand(\n)Files to be removed during \"make distclean\": "$$DFILE$$escape_expand(\n))
    QMAKE_DISTCLEAN += $${DFILE}

}
