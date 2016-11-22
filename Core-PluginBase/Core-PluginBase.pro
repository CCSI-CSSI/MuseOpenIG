#-------------------------------------------------
#
# Project created by QtCreator 2015-01-11T11:38:11
#
#-------------------------------------------------

QT     -= core gui

CONFIG += silent warn_off

TARGET = OpenIG-PluginBase
TEMPLATE =  lib

DEFINES +=  IGPLUGINCORE_LIBRARY

SOURCES +=  PluginHost.cpp

HEADERS +=  Config.h\
            Export.h\
            IGPluginCore.h\
            Plugin.h\
            PluginContext.h\
            PluginHost.h\
            PluginOperation.h

INCLUDEPATH += ../
DEPENDPATH += ../

LIBS += -losg -losgDB -losgViewer -lOpenThreads -lOpenIG-Base

OTHER_FILES += CMakeLists.txt
DISTFILES += CMakeLists.txt

unix {
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

    #
    # Allow alternate boost library path to be set via ENV variable
    #
    BOOSTROOT = $$(BOOST_ROOT)
    isEmpty(BOOSTROOT) {
        message($$TARGET -- \"BOOST_ROOT env var\" not set...using system default paths to look for boost )
        LIBS +=  -lboost_system -lboost_filesystem
    }
    else {
        message($$TARGET -- \"BOOST_ROOT env var\" detected - set to: \"$$BOOSTROOT\")
        LIBS += -L$$BOOSTROOT/stage/lib -lboost_system -lboost_filesystem
        INCLUDEPATH += $$BOOSTROOT
        DEPENDPATH  += $$BOOSTROOT
    }

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

win32-g++:QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL -march=i686

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
                LIBS += -llibboost_filesystem-vc120-mt-gd-1_58 -llibboost_system-vc120-mt-gd-1_58
            }else{
                message($$TARGET -- Boost using release version of libraries )
                LIBS += -llibboost_filesystem-vc120-mt-1_58 -llibboost_system-vc120-mt-1_58
            }
        }
    }

    OPENIGBUILD = $$(OPENIG_BUILD)
    isEmpty (OPENIGBUILD) {
    OPENIGBUILD = $$IN_PWD/..
    }
    message($$TARGET -- \"openig build\" detected in \"$$OPENIGBUILD\")
    LIBS += -L$$OPENIGBUILD/lib

    DESTDIR = $$OPENIGBUILD/lib
    DLLDESTDIR = $$OPENIGBUILD/bin

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

}
