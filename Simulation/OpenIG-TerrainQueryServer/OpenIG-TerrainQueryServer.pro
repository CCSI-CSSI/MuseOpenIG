TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

TARGET = openig-client-tqserver

SOURCES += main.cpp

include(DataFiles/deployment.pri)
qtcAddDeployment()

INCLUDEPATH += ../
DEPENDPATH += ../

DEFINES += OPENIG_SDK

OTHER_FILES += CMakeLists.txt

DISTFILES += CMakeLists.txt

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

}
