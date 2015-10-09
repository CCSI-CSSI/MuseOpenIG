#-------------------------------------------------
#
# Project created by QtCreator 2015-03-29T20:22:26
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent

TARGET = IgLib-Networking
TEMPLATE = lib

DEFINES += IGLIBNETWORKING_LIBRARY

SOURCES +=  buffer.cpp \
            network.cpp \
            packet.cpp \
            udpnetwork.cpp \
			tcpserver.cpp \
			tcpclient.cpp

HEADERS +=  buffer.h \
            export.h \
            network.h \
            packet.h \
            parser.h \
            udpnetwork.h \
			tcpserver.h \
			tcpclient.h \
			error.h 

INCLUDEPATH += ../
DEPENDPATH += ../

OTHER_FILES += CMakeLists.txt

unix {
    !mac:contains(QMAKE_HOST.arch, x86_64):{
        DESTDIR = /usr/local/lib64
        LIBS+=-L/usr/local/lib64
        target.path = /usr/local/lib64
    } else {
        DESTDIR = /usr/local/lib
        LIBS+=-L/usr/local/lib
        target.path = /usr/local/lib
    }
    message($$TARGET Lib will be installed into $$DESTDIR)

    INSTALLS += target

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include    

    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    #
    # Allow alternate boost library path to be set via ENV variable
    #
    BOOSTROOT = $$(BOOST_ROOT)
    isEmpty(BOOSTROOT) {
        message($$TARGET --  -- \"BOOST_ROOT env var\" not set...using system default paths to look for boost )
        !mac:LIBS +=  -lboost_system -lboost_thread
         mac:LIBS += -lboost_system-mt -lboost_thread-mt
    }
    else {
        message($$TARGET --  -- \"BOOST_ROOT env var\" detected - set to: \"$$BOOSTROOT\")
        !mac:LIBS += -L$$BOOSTROOT/stage/lib \
                -lboost_system -l boost_thread
         mac:LIBS += -L$$BOOSTROOT/lib \
                     -lboost_system-mt -lboost_thread-mt
        INCLUDEPATH += $$BOOSTROOT
        DEPENDPATH  += $$BOOSTROOT
    }

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
    BOOSTROOT = $$(BOOST_ROOT)
    isEmpty(BOOSTROOT) {
        message($$TARGET -- \"boost\" not detected...)
    }
    else {
        INCLUDEPATH += $$BOOSTROOT
        win32-g++ {
            message($$TARGET -- win32-g++ --\"boost\" detected in \"$$BOOSTROOT\")
            LIBS += -L$$BOOSTROOT\stage\lib -llibboost_system
        } else {
            message($$TARGET -- win32 -- \"boost\" detected in \"$$BOOSTROOT\")
            LIBS += -L$$BOOSTROOT\stage\lib
            CONFIG( debug,debug|release ){
                message($$TARGET -- Boost using debug version of libraries )
                LIBS += -llibboost_system-vc120-mt-gd-1_58
            }else{
                message($$TARGET -- Boost using release version of libraries )
                LIBS += -llibboost_system-vc120-mt-1_58
            }
        }
    }

    OPENIGBUILD = $$(OPENIG_BUILD)
    isEmpty (OPENIGBUILD) {
        OPENIGBUILD = $$IN_PWD/..
    }
    DESTDIR = $$OPENIGBUILD/lib
    DLLDESTDIR = $$OPENIGBUILD/bin

    LIBS += -L$$OPENIGBUILD/lib
}

