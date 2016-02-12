TEMPLATE = app
CONFIG += console silent
CONFIG -= app_bundle
CONFIG -= qt

TARGET = openig-demo-ig


SOURCES += main.cpp

include(DataFiles/deployment.pri)
qtcAddDeployment()

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgGA -lOpenIG-Engine -lOpenIG-Base

INCLUDEPATH += ../
DEPENDPATH += ../

OTHER_FILES += DataFiles/default.txt \
               DataFiles/openig.xml \
               DataFiles/OpenIG-Splash.jpg

DISTFILES += DataFiles/default.txt \
             DataFiles/openig.xml \
             DataFiles/OpenIG-Splash.jpg

OTHER_FILES += CMakeLists.txt
DISTFILES += CMakeLists.txt

unix {
    DEFINES += LINUX
    DESTDIR = /usr/local/bin/openig-ig

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    INCLUDEPATH += /usr/lib64
    DEPENDPATH += /usr/lib64

    #
    # Allow alternate boost library path to be set via ENV variable
    #
    BOOSTROOT = $$(BOOST_ROOT)
    isEmpty(BOOSTROOT) {
        !build_pass:message($$basename(_PRO_FILE_) -- \"BOOST_ROOT env var\" not set...using system default paths to look for boost )
        LIBS +=  -lboost_system -lboost_filesystem
    }
    else {
        !build_pass:message($$basename(_PRO_FILE_) -- \"BOOST_ROOT env var\" detected - set to: \"$$BOOSTROOT\")
        LIBS += -L$$BOOSTROOT/stage/lib \
                -lboost_system -lboost_filesystem
        INCLUDEPATH += $$BOOSTROOT
        DEPENDPATH  += $$BOOSTROOT
    }
    !mac:LIBS += -lX11

    FILE = $${PWD}/DataFiles/default.txt
    DDIR = $$DESTDIR
    DFILE = $$DESTDIR/default.txt
    distfiles = $$DFILE
    QMAKE_POST_LINK =  test -d $$quote($$DESTDIR)  || $$QMAKE_MKDIR $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DFILE) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    FILE = $${PWD}/DataFiles/openig.xml
    DDIR = $${DESTDIR}/igdata
    DFILE = $${DDIR}/openig.xml
    distfiles += $$DFILE
    QMAKE_POST_LINK += test -d $$quote($$DDIR) || $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DFILE) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    FILE = $${PWD}/DataFiles/OpenIG-Splash.jpg
    DDIR = $${DESTDIR}/igdata
    DFILE = $${DDIR}/OpenIG-Splash.jpg
    distfiles += $$DFILE
    QMAKE_POST_LINK += test -d $$quote($$DDIR) || $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DFILE) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

#    !build_pass:message("$$escape_expand(\n)Files to be removed during \"make distclean\": "$$distfiles$$escape_expand(\n))
    QMAKE_DISTCLEAN += $$distfiles
#    QMAKE_DISTCLEAN += -r $$DESTDIR

    # library version number files
    exists( "../openig_version.pri" ) {

        include( "../openig_version.pri" )
        isEmpty( VERSION ){ !build_pass:error( "$$basename(_PRO_FILE_) bad or undefined VERSION variable inside file openig_version.pri" )
        } else {
        !build_pass:message( "$$basename(_PRO_FILE_) Set version info to: $$VERSION" )
        }

    }
    else { !build_pass:error( "$$basename(_PRO_FILE_) could not find pri library version file openig_version.pri" ) }

    # end of library version number files
}


win32-g++:QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL
win32-g++:LIBS += -lstdc++.dll

win32 {
    LIBS += -lUser32

    OSGROOT = $$(OSG_ROOT)
    isEmpty(OSGROOT) {
        !build_pass:message(\"$$basename(_PRO_FILE_) OpenSceneGraph\" not detected...)
    }
    else {
        !build_pass:message(\"$$basename(_PRO_FILE_) OpenSceneGraph\" detected in \"$$OSGROOT\")
        INCLUDEPATH += $$OSGROOT/include
        LIBS += -L$$OSGROOT/lib
    }
    OSGBUILD = $$(OSG_BUILD)
    isEmpty(OSGBUILD) {
        !build_pass:message(\"$$basename(_PRO_FILE_) OpenSceneGraph build\" not detected...)
    }
    else {
        !build_pass:message(\"$$basename(_PRO_FILE_) OpenSceneGraph build\" detected in \"$$OSGBUILD\")
        DEPENDPATH += $$OSGBUILD/lib
        INCLUDEPATH += $$OSGBUILD/include
        LIBS += -L$$OSGBUILD/lib
    }

    OPENIGBUILD = $$(OPENIG_BUILD)
    isEmpty (OPENIGBUILD) {
        OPENIGBUILD = $$IN_PWD/..
    }
    DESTDIR = $$OPENIGBUILD/bin/openig-ig
    LIBS += -L$$OPENIGBUILD/lib

    FILE = $${PWD}/DataFiles/default.txt
    DDIR = $$DESTDIR

    FILE ~= s,/,\\,g
    DDIR ~= s,/,\\,g
    distfiles = $$DDIR\\$$basename(FILE)
    QMAKE_PRE_LINK   = $$QMAKE_CHK_DIR_EXISTS $$quote($$DDIR) $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    FILE = $${PWD}/DataFiles/openig.xml
    DDIR = $${DESTDIR}/igdata
    DFILE = $${DDIR}/$$basename(FILE)

    FILE  ~= s,/,\\,g
    DDIR  ~= s,/,\\,g
    DFILE ~= s,/,\\,g
    distfiles += $$DFILE
    QMAKE_POST_LINK += $$QMAKE_CHK_DIR_EXISTS $$quote($$DDIR) $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    FILE = $${PWD}/DataFiles/OpenIG-Splash.jpg
    DDIR = $${DESTDIR}/igdata
    DFILE = $${DDIR}/$$basename(FILE)

    FILE  ~= s,/,\\,g
    DDIR  ~= s,/,\\,g
    DFILE ~= s,/,\\,g
    distfiles += $$DFILE
    QMAKE_POST_LINK += $$QMAKE_CHK_DIR_EXISTS $$quote($$DDIR) $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    !build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$distfiles$$escape_expand(\n))
    QMAKE_DISTCLEAN += $$distfiles
# Need to figure out how to pass the /s without it being converted to\s by qt. before we can just remove the dir and all internal files -- CGR
#    QMAKE_DISTCLEAN += /s /q $$DESTDIR\\igdata
}
