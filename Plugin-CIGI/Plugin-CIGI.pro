#-------------------------------------------------
#
# Project created by QtCreator 2015-01-12T12:01:03
#
#-------------------------------------------------

QT -= core gui

CONFIG += silent warn_off

TARGET = OpenIG-Plugin-CIGI
TEMPLATE = lib

DEFINES += IGPLUGINCIGI_LIBRARY

CONFIG -= warn_on

SOURCES += IGPluginCIGI.cpp

HEADERS +=

INCLUDEPATH += ../
DEPENDPATH += ../

LIBS += -losg -losgDB -losgViewer -lOpenThreads \
        -lOpenIG-Engine -lOpenIG-Base -lOpenIG-PluginBase \
        -lOpenIG-Networking -lOpenIG-Protocol -lcigicl

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
   !build_pass:message($$basename(_PRO_FILE_) Lib will be installed into $$DESTDIR)

    INSTALLS += target

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    INCLUDEPATH += /usr/local/include/Public_Headers
    DEPENDPATH += /usr/local/include/Public_Headers

    #
    # Allow alternate boost library path to be set via ENV variable
    #
    BOOSTROOT = $$(BOOST_ROOT)
    isEmpty(BOOSTROOT) {
       !build_pass:message($$basename(_PRO_FILE_) -- \"BOOST_ROOT env var\" not set...using system default paths to look for boost )
        LIBS +=  -lboost_date_time -lboost_thread
    }
    else {
        !build_pass:message($$basename(_PRO_FILE_) -- \"BOOST_ROOT env var\" detected - set to: \"$$BOOSTROOT\")
        LIBS += -L$$BOOSTROOT/stage/lib \
                -lboost_date_time -lboost_thread
        INCLUDEPATH += $$BOOSTROOT
        DEPENDPATH  += $$BOOSTROOT
    }

    FILE = $${PWD}/DataFiles/libIgPlugin-CIGI.so.xml
    DDIR = $${DESTDIR}/libOpenIG-Plugin-CIGI.so.xml

    mac: DDIR = $${DESTDIR}/libOpenIG-Plugin-CIGI.dylib.xml

    QMAKE_POST_LINK =  test -d $$quote($$DESTDIR) || $$QMAKE_MKDIR $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DDIR) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    #!build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$DDIR$$escape_expand(\n))
    QMAKE_DISTCLEAN += $${DDIR}

    !mac: LIBS += -lGL -lGLU
    mac: LIBS += -framework openGL

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
    LIBS += -lopengl32 -lglu32 -lUser32

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
    DESTDIR = $$OPENIGBUILD/lib
    DLLDESTDIR = $$OPENIGBUILD/bin/plugins

    LIBS += -L$$OPENIGBUILD/lib

    BOOSTROOT = $$(BOOST_ROOT)
    isEmpty(BOOSTROOT) {
        message($$TARGET -- \"boost\" not detected...)
    }
    else {
        INCLUDEPATH += $$BOOSTROOT
        win32-g++ {
            message($$TARGET -- win32-g++ --\"boost\" detected in \"$$BOOSTROOT\")
            LIBS += -L$$BOOSTROOT\stage\lib -llibboost_filesystem -llibboost_system
        } else {
            message($$TARGET -- win32 -- \"boost\" detected in \"$$BOOSTROOT\")
            LIBS += -L$$BOOSTROOT\stage\lib
            CONFIG( debug,debug|release ){
                message($$TARGET -- Boost using debug version of libraries )
                LIBS += -llibboost_filesystem-vc120-mt-gd-1_58 -llibboost_system-vc120-mt-gd-1_58 -llibboost_thread-vc120-mt-gd-1_58
            }else{
                message($$TARGET -- Boost using release version of libraries )
                LIBS += -llibboost_filesystem-vc120-mt-1_58 -llibboost_system-vc120-mt-1_58 -llibboost_thread-vc120-mt-1_58
            }
        }
    }

    CIGIROOT = $$(CIGI_ROOT)
    isEmpty(CIGIROOT) {
        !build_pass:message($$basename(_PRO_FILE_) -- \"CIGI\" not detected...)
    }
    else {
        !build_pass:message($$basename(_PRO_FILE_) -- \"CIGI\" detected in \"$$CIGIROOT\")
        INCLUDEPATH += $$quote(\"$$CIGIROOT\include\")
        LIBS += -L$$CIGIROOT\lib -lcigicl
    }
    CIGIBUILD = $$(CIGI_BUILD)
    isEmpty(CIGIBUILD) {
        !build_pass:message($$basename(_PRO_FILE_) -- \"CIGI build\" not detected...)
    }
    else {
       !build_pass:message($$basename(_PRO_FILE_) -- \"CIGI build\" detected in \"$$CIGIBUILD\")
        DEPENDPATH += $$CIGIBUILD
        LIBS += -L$$CIGIBUILD\lib\Release -lcigicl
    }

    FILE = $${PWD}/DataFiles/libIgPlugin-CIGI.so.xml
    DFILE = $${DLLDESTDIR}/OpenIG-Plugin-CIGI.dll.xml

    FILE ~= s,/,\\,g
    DFILE ~= s,/,\\,g

    INCLUDEPATH ~= s,/,\\,g
    !build_pass:message($$basename(_PRO_FILE_) INCLUDEPATH -- $$INCLUDEPATH)
    !build_pass:message($$basename(_PRO_FILE_) LIBS -- $$LIBS)

    QMAKE_POST_LINK =  if not exist $$quote($$DLLDESTDIR) $$QMAKE_MKDIR $$quote($$DLLDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    #!build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$DFILE$$escape_expand(\n))
    QMAKE_DISTCLEAN += $${DFILE}

}
