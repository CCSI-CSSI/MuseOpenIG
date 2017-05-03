TEMPLATE = app
CONFIG += console silent
CONFIG -= app_bundle
CONFIG -= qt

TARGET = openig-demo-a320
CONFIG += warn_off

SOURCES += main.cpp

include(DataFiles/deployment.pri)
qtcAddDeployment()

INCLUDEPATH += ../
DEPENDPATH += ../


OTHER_FILES += CMakeLists.txt \
               DataFiles/default.txt \
               DataFiles/richcolorsdemo.txt \
               DataFiles/openig.xml \
               DataFiles/OpenIG-Splash.jpg \
               DataFiles/flightpath.bin \
               DataFiles/flightpath.path

DISTFILES += CMakeLists.txt \
             DataFiles/default.txt \
             DataFiles/richcolorsdemo.txt \
             DataFiles/openig.xml \
             DataFiles/OpenIG-Splash.jpg \
             DataFiles/flightpath.bin \
             DataFiles/flightpath.path

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgGA -losgText -losgUtil -losgSim\
        -lOpenIG-Engine -lOpenIG-Base -lOpenIG-PluginBase -lOpenIG-Graphics

unix {
    LIBS += -L/usr/local/lib64

    DEFINES += LINUX
    DESTDIR = /usr/local/bin/openig-a320

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
        LIBS +=  -lboost_system -lboost_filesystem -lboost_thread
    }
    else {
        !build_pass:message($$basename(_PRO_FILE_) -- \"BOOST_ROOT env var\" detected - set to: \"$$BOOSTROOT\")
        LIBS += -L$$BOOSTROOT/stage/lib \
                -lboost_system -lboost_filesystem -lboost_thread
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

    FILE = $${PWD}/DataFiles/richcolorsdemo.txt
    DDIR = $$DESTDIR
    DFILE = $$DESTDIR/richcolorsdemo.txt
    distfiles += $$DFILE

    QMAKE_POST_LINK += test -d $$quote($$DESTDIR)  || $$QMAKE_MKDIR $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DFILE) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    FILE = $${PWD}/DataFiles/iitsec.txt
    DDIR = $$DESTDIR
    DFILE = $$DESTDIR/iitsec.txt
    distfiles += $$DFILE

    QMAKE_POST_LINK += test -d $$quote($$DESTDIR)  || $$QMAKE_MKDIR $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DFILE) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    FILE = $${PWD}/DataFiles/boeing.txt
    DDIR = $$DESTDIR
    DFILE = $$DESTDIR/boeing.txt
    distfiles += $$DFILE

    QMAKE_POST_LINK += test -d $$quote($$DESTDIR)  || $$QMAKE_MKDIR $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DFILE) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    FILE = $${PWD}/DataFiles/flightpath.path
    DDIR = $$DESTDIR/demo
    DFILE = $$DESTDIR/demo/flightpath.path
    distfiles += $$DFILE

    QMAKE_POST_LINK += test -d $$quote($$DDIR)  || $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
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

    FILE = $${PWD}/../Resources/OpenIG-Splash-BasedOnOpenIG.png
    DDIR = $${DESTDIR}/igdata
    DFILE = $${DDIR}/OpenIG-Splash-BasedOnOpenIG.png
    distfiles += $$DFILE

    QMAKE_POST_LINK += test -d $$quote($$DDIR) || $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DFILE) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

#    !build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$distfiles$$escape_expand(\n))
    QMAKE_DISTCLEAN += $$distfiles
#    QMAKE_DISTCLEAN += -r $$DESTDIR/demo $$DESTDIR/igdata

    # library version number files
    exists( "../openig_version.pri" ) {

    include( "../openig_version.pri" )
        isEmpty( VERSION ){ !build_pass:error( "$$basename(_PRO_FILE_) bad or undefined VERSION variable inside file openig_version.pri" )
    } else {
        !build_pass:message( "$$basename(_PRO_FILE_) -- Set version info to: $$VERSION" )
    }

    }
    else { error( "$$basename(_PRO_FILE_) could not find pri library version file openig_version.pri" ) }

    # end of library version number files
}

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

    LIBS += -L$$OPENIGBUILD/lib

    DESTDIR = $$OPENIGBUILD/bin/openig-a320

    FILE = $${PWD}/DataFiles/default.txt
    DDIR = $$DESTDIR

    FILE ~= s,/,\\,g
    DDIR ~= s,/,\\,g
    distfiles = $$DDIR\\$$basename(FILE)

    !build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$distfiles$$escape_expand(\n))
    QMAKE_PRE_LINK   = $$QMAKE_CHK_DIR_EXISTS $$quote($$DDIR) $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    FILE = $${PWD}/DataFiles/richcolorsdemo.txt
    DDIR = $$DESTDIR

    FILE ~= s,/,\\,g
    DDIR ~= s,/,\\,g
    distfiles += $$DDIR\\$$basename(FILE)

    !build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$distfiles$$escape_expand(\n))
    QMAKE_PRE_LINK   = $$QMAKE_CHK_DIR_EXISTS $$quote($$DDIR) $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    FILE = $${PWD}/DataFiles/iitsec.txt
    DDIR = $$DESTDIR

    FILE ~= s,/,\\,g
    DDIR ~= s,/,\\,g
    distfiles += $$DDIR\\$$basename(FILE)

    !build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$distfiles$$escape_expand(\n))
    QMAKE_PRE_LINK   = $$QMAKE_CHK_DIR_EXISTS $$quote($$DDIR) $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    FILE = $${PWD}/DataFiles/boeing.txt
    DDIR = $$DESTDIR

    FILE ~= s,/,\\,g
    DDIR ~= s,/,\\,g
    distfiles += $$DDIR\\$$basename(FILE)

    !build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$distfiles$$escape_expand(\n))
    QMAKE_PRE_LINK   = $$QMAKE_CHK_DIR_EXISTS $$quote($$DDIR) $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    FILE = $${PWD}/DataFiles/flightpath.path
    DDIR = $$DESTDIR/demo

    FILE ~= s,/,\\,g
    DDIR ~= s,/,\\,g
    distfiles += $$DDIR\\$$basename(FILE)

    QMAKE_POST_LINK += $$QMAKE_CHK_DIR_EXISTS $$quote($$DDIR) $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    FILE = $${PWD}/DataFiles/openig.xml
    DDIR = $${DESTDIR}/igdata
    DFILE = $${DDIR}/openig.xml

    FILE  ~= s,/,\\,g
    DDIR  ~= s,/,\\,g
    DFILE ~= s,/,\\,g
    distfiles += $$DFILE

    QMAKE_POST_LINK += $$QMAKE_CHK_DIR_EXISTS $$quote($$DDIR) $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    FILE = $${PWD}/DataFiles/OpenIG-Splash.jpg
    DDIR = $${DESTDIR}/igdata
    DFILE = $${DDIR}/OpenIG-Splash.jpg

    FILE  ~= s,/,\\,g
    DDIR  ~= s,/,\\,g
    DFILE ~= s,/,\\,g
    distfiles += $$DFILE

    QMAKE_POST_LINK += $$QMAKE_CHK_DIR_EXISTS $$quote($$DDIR) $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    !build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$distfiles$$escape_expand(\n))
    QMAKE_DISTCLEAN += $$distfiles
#    QMAKE_DISTCLEAN += /s /q $$DESTDIR/demo $$DESTDIR/igdata

}
