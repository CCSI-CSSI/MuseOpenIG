TEMPLATE = app
CONFIG += console silent
CONFIG -= app_bundle
CONFIG -= qt

TARGET = openig-demo-earth

SOURCES += main.cpp

include(DataFiles/deployment.pri)
qtcAddDeployment()

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

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgGA -lOpenIG-Engine

unix {
    DESTDIR = /usr/local/bin/openig-earth

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
        message(IG-Earth -- \"BOOST_ROOT env var\" not set...using system default paths to look for boost )
        LIBS +=  -lboost_system -lboost_filesystem
    }
    else {
        message(IG-Earth -- \"BOOST_ROOT env var\" detected - set to: \"$$BOOSTROOT\")
        LIBS += -L$$BOOSTROOT/stage/lib \
                -lboost_system -lboost_filesystem
        INCLUDEPATH += $$BOOSTROOT
        DEPENDPATH  += $$BOOSTROOT
    }
    !mac:LIBS += -lX11

    LIBS += -losgEarth -losgEarthUtil

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

    FILE = $${PWD}/DataFiles/OpenIG-Earth-Splash.jpg
    DDIR = $${DESTDIR}/igdata
    DFILE = $${DDIR}/OpenIG-Earth-Splash.jpg
    distfiles += $$DFILE

    QMAKE_POST_LINK += test -d $$quote($$DDIR) || $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DFILE) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    QMAKE_DISTCLEAN += $$distfiles
#    QMAKE_DISTCLEAN += -r $$DESTDIR

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
win32-g++:LIBS += -lstdc++.dll

win32 {
    LIBS += -lUser32

    OSGROOT = $$(OSG_ROOT)
    isEmpty(OSGROOT) {
        !build_pass:message(\"$$basename(_PRO_FILE_) \"OpenSceneGraph\" not detected...)
    }
    else {
        !build_pass:message(\"$$basename(_PRO_FILE_) \"OpenSceneGraph\" detected in \"$$OSGROOT\")
        INCLUDEPATH += $$OSGROOT/include
        LIBS += -L$$OSGROOT/lib
    }
    OSGBUILD = $$(OSG_BUILD)
    isEmpty(OSGBUILD) {
        !build_pass:message(\"$$basename(_PRO_FILE_) \"OpenSceneGraph build\" not detected...)
    }
    else {
        !build_pass:message(\"$$basename(_PRO_FILE_) \"OpenSceneGraph build\" detected in \"$$OSGBUILD\")
        DEPENDPATH += $$OSGBUILD/lib
        INCLUDEPATH += $$OSGBUILD/include
        LIBS += -L$$OSGBUILD/lib
    }

    exists( C:/Program Files/OSGEARTH ) {
        OSGEARTH_ROOT = C:\Program Files\OSGEARTH
    }
    OSGEARTHROOT = $$(OSGEARTH_ROOT)
    isEmpty(OSGEARTHROOT) {
        !build_pass:message(\"$$basename(_PRO_FILE_) \"osgEarth\" not detected...)
    }
    else {
        !build_pass:message(\"$$basename(_PRO_FILE_) \"osgEarth\" detected in \"$$OSGEARTHROOT\")
        INCLUDEPATH += $$OSGEARTHROOT\include
        LIBS += -L$$OSGEARTHROOT
        LIBS += -L$$OSGEARTHROOT\lib
        LIBS += -losgEarth -losgEarthUtil
    }

# did not find osgEarth yet, check for build env var...
    isEmpty(OSGEARTHROOT) {
        OSGEARTHBUILD = $$(OSGEARTH_BUILD)
        isEmpty(OSGEARTHBUILD) {
            !build_pass:message(\"$$basename(_PRO_FILE_) \"osgEarth build\" not detected...)
        }
        else {
            !build_pass:message(\"$$basename(_PRO_FILE_) \"osgEarth build\" detected in \"$$OSGEARTHBUILD\")
            DEPENDPATH += $$OSGEARTHBUILD
            INCLUDEPATH += $$OSGBUILD\include
            LIBS += -L$$OSGEARTHBUILD\BUILD\lib\release
            LIBS += -losgEarth -losgEarthUtil
        }
    }

    OPENIGBUILD = $$(OPENIG_BUILD)
    isEmpty (OPENIGBUILD) {
        OPENIGBUILD = $$IN_PWD/..
    }
    message(\"openig build\" detected in \"$$OPENIGBUILD\")
    DESTDIR = $$OPENIGBUILD/bin/openig-earth
    LIBS += -L$$OPENIGBUILD/lib

    FILE = $${PWD}/DataFiles/default.txt
    DDIR = $$DESTDIR
    DFILE = $${DDIR}/$$basename(FILE)

    FILE ~= s,/,\\,g
    DDIR ~= s,/,\\,g
    DFILE ~= s,/,\\,g
    distfiles = $$DFILE

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

    FILE = $${PWD}/DataFiles/OpenIG-Earth-Splash.jpg
    DDIR = $${DESTDIR}/igdata
    DFILE = $${DDIR}/OpenIG-Earth-Splash.jpg

    FILE  ~= s,/,\\,g
    DDIR  ~= s,/,\\,g
    DFILE ~= s,/,\\,g
    distfiles += $$DFILE
    QMAKE_POST_LINK += $$QMAKE_CHK_DIR_EXISTS $$quote($$DDIR) $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    !build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$distfiles$$escape_expand(\n))
    QMAKE_DISTCLEAN += $$distfiles
#    QMAKE_DISTCLEAN += /s /q $$DESTDIR/igdata
}
