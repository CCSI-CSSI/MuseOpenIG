TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

TARGET = openig-client-ig

SOURCES += main.cpp

include(DataFiles/deployment.pri)
qtcAddDeployment()

INCLUDEPATH += ../
DEPENDPATH += ../

DEFINES += OPENIG_SDK

OTHER_FILES +=  CMakeLists.txt \
                DataFiles/default.txt \
                DataFiles/openig.xml \
                DataFiles/deployment.pri \
                DataFiles/OpenIG-Splash.jpg \
                DataFiles/model/*.*

DISTFILES += CMakeLists.txt \
            DataFiles/default.txt \
            DataFiles/openig.xml \
            DataFiles/deployment.pri \
            DataFiles/OpenIG-Splash.jpg \
            DataFiles/model/*.*

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgGA -losgText -losgUtil -losgSim\
        -lOpenIG-Engine -lOpenIG-Base -lOpenIG-PluginBase -lOpenIG-Networking -lOpenIG-Protocol -lOpenIG-Graphics

unix {
    LIBS += -L/usr/local/lib64

    DEFINES += LINUX
    DESTDIR = /usr/local/bin/openig-client

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    INCLUDEPATH += /usr/lib64
    DEPENDPATH += /usr/lib64

    !mac:LIBS += -lX11
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

#    !build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$distfiles$$escape_expand(\n))
    QMAKE_DISTCLEAN += $$distfiles
#    QMAKE_DISTCLEAN += -r $$DESTDIR/demo $$DESTDIR/igdata

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
    INCLUDEPATH += $$OPENIGBUILD/include

    LIBS += -L$$OPENIGBUILD/lib

    DESTDIR = $$OPENIGBUILD/bin/openig-client

    BOOSTROOT = $$(BOOST_ROOT)
    isEmpty(BOOSTROOT) {
        !build_pass:message($$basename(_PRO_FILE_) \"boost\" not detected...)
    }
    else {
        INCLUDEPATH += $$BOOSTROOT
        win32-g++ {
        !build_pass:message($$basename(_PRO_FILE_) win32-g++ --\"boost\" detected in \"$$BOOSTROOT\")
        LIBS += -L$$BOOSTROOT\stage\lib \
                -lboost_system -lboost_filesystem -lboost_date_time \
                -lboost_regex  -lboost_thread     -lboost_chrono
        } else {
            !build_pass:message($$basename(_PRO_FILE_) -- win32 -- \"boost\" detected in \"$$BOOSTROOT\")
            LIBS += -L$$BOOSTROOT\stage\lib
            CONFIG( debug,debug|release ){
                !build_pass:message($$basename(_PRO_FILE_) -- Boost using debug version of libraries )
                LIBS += -llibboost_filesystem-vc120-mt-gd-1_58 -llibboost_system-vc120-mt-gd-1_58
            }else{
                !build_pass:message($$basename(_PRO_FILE_) -- Boost using release version of libraries )
                LIBS += -llibboost_filesystem-vc120-mt-1_58 -llibboost_system-vc120-mt-1_58
            }
        }
    }

    FILE = $${PWD}/DataFiles/default.txt
    DDIR = $$DESTDIR

    FILE ~= s,/,\\,g
    DDIR ~= s,/,\\,g
    distfiles = $$DDIR\\$$basename(FILE)

    !build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$distfiles$$escape_expand(\n))
    QMAKE_PRE_LINK   = $$QMAKE_CHK_DIR_EXISTS $$quote($$DDIR) $$QMAKE_MKDIR $$quote($$DDIR) $$escape_expand(\\n\\t)
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
