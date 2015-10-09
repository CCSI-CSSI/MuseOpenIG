#-------------------------------------------------
#
# Project created by QtCreator 2015-01-12T12:01:03
#
#-------------------------------------------------

QT -= core gui

CONFIG += silent

TARGET = IgPlugin-SilverLining
TEMPLATE = lib

DEFINES += IGPLUGINSILVERLINING_LIBRARY

CONFIG -= warn_on

SOURCES += igpluginsilverlining.cpp \
    SkyDrawable.cpp \
    CloudsDrawable.cpp

HEADERS += \
    CloudsDrawable.h \
    SkyDrawable.h \
    AtmosphereReference.h

INCLUDEPATH += ../
DEPENDPATH += ../

LIBS += -losg -losgDB -losgViewer -lOpenThreads -lOpenIG -lIgPluginCore -lIgCore

OTHER_FILES += \
    $${PWD}/ConfigFiles/* \
    $${PWD}/CmakeLists.txt

unix {
    !mac:contains(QMAKE_HOST.arch, x86_64):{
        DESTDIR = /usr/local/lib64/igplugins
        target.path = /usr/local/lib64/igplugins
    } else {
        DESTDIR = /usr/local/lib/igplugins
        target.path = /usr/local/lib/igplugins
    }
    message($$TARGET Lib will be installed into $$DESTDIR)

    INSTALLS += target

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    INCLUDEPATH += /usr/local/include/Public_Headers
    DEPENDPATH += /usr/local/include/Public_Headers

    LIBS += -lSilverLiningOpenGL

    #
    # Allow alternate boost library path to be set via ENV variable
    #
    BOOSTROOT = $$(BOOST_ROOT)
    isEmpty(BOOSTROOT) {
        message($$TARGET -- \"BOOST_ROOT env var\" not set...using system default paths to look for boost )
        LIBS +=  -lboost_date_time
    }
    else {
        message($$TARGET -- \"BOOST_ROOT env var\" detected - set to: \"$$BOOSTROOT\")
        LIBS += -L$$BOOSTROOT/stage/lib \
                -lboost_date_time
        INCLUDEPATH += $$BOOSTROOT
        DEPENDPATH  += $$BOOSTROOT
    }

    FILE = $${PWD}/ConfigFiles/libIgPlugin-SilverLining.so.xml
    DDIR = $${DESTDIR}/libIgPlugin-SilverLining.so.xml

    mac: DDIR = $${DESTDIR}/libIgPlugin-SilverLining.dylib.xml

    QMAKE_POST_LINK =  test -d $$quote($$DESTDIR) || $$QMAKE_MKDIR $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DDIR) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    QMAKE_DISTCLEAN += $${DESTDIR}/libIgPlugin-SilverLining.*.xml

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
        message($$TARGET -- \"OpenSceneGraph\" not detected...)
    }
    else {
        message($$TARGET -- \"OpenSceneGraph\" detected in \"$$OSGROOT\")
        INCLUDEPATH += $$OSGROOT/include
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
    DLLDESTDIR = $$OPENIGBUILD/bin/igplugins

    LIBS += -L$$OPENIGBUILD/lib

    SLROOT = $$(SILVERLINING)
    isEmpty(SLROOT) {
        message($$TARGET -- \"SilverLining\" not detected...)
    }
    else {
        message($$TARGET -- \"SilverLining\" detected in \"$$SLROOT\")
        INCLUDEPATH += $$quote(\"$$SLROOT\Public Headers\")
        message($$TARGET -- $$INCLUDEPATH)
    }
    SLBUILD = $$(SILVERLINING_BUILD)
    isEmpty(SLBUILD) {
        message($$TARGET -- \"SilverLining build\" not detected...)
    }
    else {
        message($$TARGET -- \"SilverLining build\" detected in \"$$SLBUILD\")
        DEPENDPATH += $$SLBUILD
        LIBS += -L$$SLBUILD\lib\vc12\x64 -lSilverLining-MT-DLL
    }

    BOOSTROOT = $$(BOOST_ROOT)
    isEmpty(BOOSTROOT) {
        message($$TARGET -- \"boost\" not detected...)
    }
    else {
        INCLUDEPATH += $$BOOSTROOT
        win32-g++ {
            message($$TARGET -- win32-g++ --\"boost\" detected in \"$$BOOSTROOT\")
            LIBS += -L$$BOOSTROOT\stage\lib -lboost_date_time
        } else {
            message($$TARGET -- win32 -- \"boost\" detected in \"$$BOOSTROOT\")
            LIBS += -L$$BOOSTROOT\stage\lib
            CONFIG( debug,debug|release ){
                message($$TARGET -- Boost using debug version of libraries )
                LIBS += -llibboost_date_time-vc120-mt-gd-1_58
            }else{
                message($$TARGET -- Boost using release version of libraries )
                LIBS += -llibboost_date_time-vc120-mt-1_58
            }
        }
    }

    FILE = $${PWD}/ConfigFiles/libIgPlugin-SilverLining.so.windows.xml
    DFILE = $${DLLDESTDIR}/IgPlugin-SilverLining.dll.xml

    FILE ~= s,/,\\,g
    DFILE ~= s,/,\\,g

    QMAKE_POST_LINK =  if not exist $$quote($$DLLDESTDIR) $$QMAKE_MKDIR $$quote($$DLLDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += if not exist $$quote($$DFILE) copy /y $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    QMAKE_DISTCLEAN += $${DLLDESTDIR}/libIgPlugin-SilverLining.*.xml

}
