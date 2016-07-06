#-------------------------------------------------
#
# Project created by QtCreator 2015-03-03T17:38:56
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent

TARGET = OpenIG-Plugin-Client
TEMPLATE = lib

DEFINES += IGPLUGINOPENGIG_LIBRARY OPENIG_SDK

SOURCES += main.cpp

HEADERS +=

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgGA -losgUtil -losgText \
        -lOpenIG-PluginBase -lOpenIG-Base -lOpenIG-Engine -lOpenIG-Networking -lOpenIG-Protocol

INCLUDEPATH += ../
DEPENDPATH += ../

OTHER_FILES += CMakeLists.txt
DISTFILES += CMakeLists.txt

OTHER_FILES += $${PWD}/DataFiles/*

unix {
    !mac:contains(QMAKE_HOST.arch, x86_64):{
        DESTDIR = /usr/local/lib64/plugins
        target.path = /usr/local/lib64/plugins
    } else {
        DESTDIR = /usr/local/lib/plugins
        target.path = /usr/local/lib/plugins
    }
    message($$TARGET Lib will be installed into $$DESTDIR)

    INSTALLS += target

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include
    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    INCLUDEPATH += /usr/local/lib
    DEPENDPATH += /usr/local/lib

    FILE = $${PWD}/DataFiles/libIgPlugin-Client.so.xml
    DDIR = $${DESTDIR}/libOpenIG-Plugin-Client.so.xml
    mac:DDIR = $${DESTDIR}/libOpenIG-Plugin-Client.dylib.xml

    QMAKE_POST_LINK =  test -d $$quote($$DESTDIR) || $$QMAKE_MKDIR $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    !build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$DDIR$$escape_expand(\n))
    QMAKE_DISTCLEAN += $${DDIR}

    # library version number files
    exists( "../../openig_version.pri" ) {

    include( "../../openig_version.pri" )
        isEmpty( VERSION ){!build_pass:error($$basename(_PRO_FILE_) -- bad or undefined VERSION variable inside file openig_version.pri)
    } else {
       !build_pass:message($$basename(_PRO_FILE_) -- Set version info to: $$VERSION)
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
    INCLUDEPATH += $$OPENIGBUILD/include

    DESTDIR = $$OPENIGBUILD/lib
    DLLDESTDIR = $$OPENIGBUILD/bin/plugins

    LIBS += -L$$OPENIGBUILD/lib

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
                LIBS += -llibboost_filesystem-vc120-mt-gd-1_58 -llibboost_date_time-vc120-mt-gd-1_58 -llibboost_system-vc120-mt-gd-1_58
                LIBS += -llibboost_regex-vc120-mt-gd-1_58 -llibboost_thread-vc120-mt-gd-1_58 -llibboost_chrono-vc120-mt-gd-1_58
            }else{
                !build_pass:message($$basename(_PRO_FILE_) -- Boost using release version of libraries )
                LIBS += -llibboost_filesystem-vc120-mt-1_58 -llibboost_date_time-vc120-mt-1_58 -llibboost_system-vc120-mt-1_58
                LIBS += -llibboost_regex-vc120-mt-1_58 -llibboost_thread-vc120-mt-1_58 -llibboost_chrono-vc120-mt-1_58
            }
        }
    }

    FILE = $${PWD}/DataFiles/libIgPlugin-Client.so.xml
    DFILE = $${DLLDESTDIR}/OpenIG-Plugin-Client.dll.xml

    FILE ~= s,/,\\,g
    DFILE ~= s,/,\\,g

    QMAKE_DISTCLEAN += $$DFILE
    message(Plugin-GPUVegetation copying $$FILE to $$DFILE)
    QMAKE_POST_LINK =  if not exist $$quote($$DLLDESTDIR) $$QMAKE_MKDIR $$quote($$DLLDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    #!build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$DDIR$$escape_expand(\n))
}
