#-------------------------------------------------
#
# Project created by QtCreator 2015-03-29T20:22:26
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent
CONFIG += c++11

TARGET = IgPlugin-Networking
TEMPLATE = lib

DEFINES += IGPLUGINNETWORKING_LIBRARY

SOURCES += igpluginnetworking.cpp

HEADERS +=

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgShadow -losgText -lOpenIG -lIgCore -lIgPluginCore -lIgLib-Networking

INCLUDEPATH += ../
DEPENDPATH += ../

OTHER_FILES += \
    $${PWD}/DataFiles/libIgPlugin-Networking.so.xml \
    $${PWD}/CMakeLists.txt

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

    #
    # Allow alternate boost library path to be set via ENV variable
    #
    BOOSTROOT = $$(BOOST_ROOT)
    isEmpty(BOOSTROOT) {
        message($$TARGET --  \"BOOST_ROOT env var\" not set...using system default paths to look for boost )
        !mac:LIBS +=  -lboost_system -lboost_filesystem -lboost_date_time \
                      -lboost_regex  -lboost_thread     -lboost_chrono
         mac:LIBS += -lboost_system-mt -lboost_filesystem-mt -lboost_date_time-mt \
                      -lboost_regex-mt  -lboost_thread-mt     -lboost_chrono-mt
    }
    else {
        message($$TARGET --  \"BOOST_ROOT env var\" detected - set to: \"$$BOOSTROOT\")
        !mac:LIBS += -L$$BOOSTROOT/stage/lib \
                -lboost_system -lboost_filesystem -lboost_date_time \
                -lboost_regex  -lboost_thread     -lboost_chrono
         mac:LIBS += -L$$BOOSTROOT/lib \
                      -lboost_system-mt -lboost_filesystem-mt -lboost_date_time-mt \
                      -lboost_regex-mt  -lboost_thread-mt     -lboost_chrono-mt
        INCLUDEPATH += $$BOOSTROOT
        DEPENDPATH  += $$BOOSTROOT
    }

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    FILE = $${PWD}/DataFiles/libIgPlugin-Networking.so.xml
    DDIR = $${DESTDIR}/libIgPlugin-Networking.so.xml

    mac: DDIR = $${DESTDIR}/libIgPlugin-Networking.dylib.xml

    QMAKE_POST_LINK =  test -d $$quote($$DESTDIR) || $$QMAKE_MKDIR $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DDIR) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    QMAKE_DISTCLEAN += $${DESTDIR}/libIgPlugin-Networking.*.xml

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
    DLLDESTDIR = $$OPENIGBUILD/bin/igplugins

    LIBS += -L$$OPENIGBUILD/lib

    BOOSTROOT = $$(BOOST_ROOT)
    isEmpty(BOOSTROOT) {
        message($$TARGET -- \"boost\" not detected...)
    }
    else {
        INCLUDEPATH += $$BOOSTROOT
        win32-g++ {
        message($$TARGET -- win32-g++ --\"boost\" detected in \"$$BOOSTROOT\")
        LIBS += -L$$BOOSTROOT\stage\lib \
                -lboost_system -lboost_filesystem -lboost_date_time \
                -lboost_regex  -lboost_thread     -lboost_chrono
        } else {
            message($$TARGET -- win32 -- \"boost\" detected in \"$$BOOSTROOT\")
            LIBS += -L$$BOOSTROOT\stage\lib
            CONFIG( debug,debug|release ){
                message($$TARGET -- Boost using debug version of libraries )
                LIBS += -llibboost_filesystem-vc120-mt-gd-1_58 -llibboost_system-vc120-mt-gd-1_58 \
                        -llibboost_date_time-vc120-mt-gd-1_58  -llibboost_regex-vc120-mt-gd-1_58 \
                        -llibboost_thread-vc120-mt-gd-1_58     -llibboost_chrono-vc120-mt-gd-1_58
            }else{
                message($$TARGET -- Boost using release version of libraries )
                LIBS += -llibboost_filesystem-vc120-mt-1_58 -llibboost_system-vc120-mt-1_58 \
                        -llibboost_date_time-vc120-mt-1_58  -llibboost_regex-vc120-mt-1_58 \
                        -llibboost_thread-vc120-mt-1_58     -llibboost_chrono-vc120-mt-1_58
            }
        }
    }

    FILE = $${PWD}/DataFiles/libIgPlugin-Networking.so.xml
    DFILE = $${DLLDESTDIR}/IgPlugin-Networking.dll.xml

    FILE ~= s,/,\\,g
    DFILE ~= s,/,\\,g

    QMAKE_POST_LINK =  if not exist $$quote($$DLLDESTDIR) $$QMAKE_MKDIR $$quote($$DLLDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += if not exist $$quote($$DFILE) copy /y $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    QMAKE_DISTCLEAN += $${DLLDESTDIR}/libIgPlugin-Networking.*.xml
}
