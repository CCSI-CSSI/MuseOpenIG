#-------------------------------------------------
#
# Project created by QtCreator 2015-03-05T11:20:12
#
#-------------------------------------------------

QT     -= core gui
CONFIG += silent

TARGET = OpenIG-Plugin-ForwardPlusLighting
TEMPLATE = lib

DEFINES += IGPLUGINFORWARDPLUSLIGHTING_LIBRARY

SOURCES +=  dummylight.cpp\
            forwardpluscullvisitor.cpp\
            forwardplusengine.cpp\
            forwardpluslightimplementationcallback.cpp\
            igpluginforwardpluslighting.cpp\
            lightmanagerstateattribute.cpp

HEADERS +=  dummylight.h\
            forwardpluscullvisitor.h\
            forwardplusengine.h\
            forwardpluslightimplementationcallback.h\
            osgtofputils.h\
            lightmanagerstateattribute.h


INCLUDEPATH += ../
DEPENDPATH += ../

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgShadow -losgUtil -losgSim -losgText -losgGA\
        -lOpenIG-Engine -lOpenIG-Base -lOpenIG-Graphics -lOpenIG-PluginBase -lOpenIG-Utils

OTHER_FILES += DataFiles/libIgPlugin-ForwardPlusLighting.so.xml
DISTFILES += DataFiles/libIgPlugin-ForwardPlusLighting.so.xml

OTHER_FILES += CMakeLists.txt
DISTFILES += CMakeLists.txt

unix {
    !mac:contains(QMAKE_HOST.arch, x86_64):{
        DESTDIR = /usr/local/lib64/plugins
        target.path = /usr/local/lib64/plugins
    } else {
        DESTDIR = /usr/local/lib/plugins
        target.path = /usr/local/lib/plugins
    }
    message(Libs will be installed into $$DESTDIR)

    #
    # Allow alternate boost library path to be set via ENV variable
    #
    BOOSTROOT = $$(BOOST_ROOT)
    isEmpty(BOOSTROOT) {
       !build_pass:message($$basename(_PRO_FILE_) -- \"BOOST_ROOT env var\" not set...using system default paths to look for boost )
        LIBS +=  -lboost_date_time -lboost_thread -lboost_system
    }
    else {
        !build_pass:message($$basename(_PRO_FILE_) -- \"BOOST_ROOT env var\" detected - set to: \"$$BOOSTROOT\")
        LIBS += -L$$BOOSTROOT/stage/lib \
                -lboost_date_time -lboost_thread -lboost_system
        INCLUDEPATH += $$BOOSTROOT
        DEPENDPATH  += $$BOOSTROOT
    }

    INSTALLS += target

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    FILE = $${PWD}/DataFiles/libIgPlugin-ForwardPlusLighting.so.xml
    DDIR = $${DESTDIR}/libOpenIG-Plugin-ForwardPlusLighting.so.xml

    mac:DDIR = $${DESTDIR}/libOpenIG-Plugin-ForwardPlusLighting.dylib.xml

    QMAKE_DISTCLEAN += $$DDIR

    QMAKE_POST_LINK =  test -d $$quote($$DESTDIR) || $$QMAKE_MKDIR $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DDIR) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    SFILES   = $$files($${PWD}/../Resources/shaders/forwardplus*.glsl)
    SFILES  += $$files($${PWD}/../Resources/shaders/shadow_*.glsl)
    SDESTDIR = /usr/local/openig/resources/shaders/

#   Get the filename(only) list for distclean to remove only the files added from this plugin
    for(var,SFILES) {
        distfiles += $$SDESTDIR/$$basename(var)
    }
    QMAKE_DISTCLEAN += $$distfiles

    QMAKE_POST_LINK += test -d $$quote($$SDESTDIR) || $$QMAKE_MKDIR $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$SFILES) $$quote($$SDESTDIR) $$escape_expand(\\n\\t)

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

win32 {
    LIBS += -lopengl32 -lglu32 -lUser32

    OSGROOT = $$(OSG_ROOT)
    isEmpty(OSGROOT) {
        message(\"OpenSceneGraph\" not detected...)
    }
    else {
        message(\"OpenSceneGraph\" detected in \"$$OSGROOT\")
        INCLUDEPATH += $$OSGROOT/include
        LIBS += -L$$OSGROOT/lib
    }
    OSGBUILD = $$(OSG_BUILD)
    isEmpty(OSGBUILD) {
        message(\"OpenSceneGraph build\" not detected...)
    }
    else {
        message(\"OpenSceneGraph build\" detected in \"$$OSGBUILD\")
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
                LIBS += -llibboost_filesystem-vc120-mt-gd-1_58  -llibboost_date_time-vc120-mt-gd-1_58 \
                        -llibboost_system-vc120-mt-gd-1_58 -llibboost_thread-vc120-mt-gd-1_58
            }else{
                message($$TARGET -- Boost using release version of libraries )
                LIBS += -llibboost_filesystem-vc120-mt-1_58  -llibboost_date_time-vc120-mt-1_58 \
                        -llibboost_system-vc120-mt-1_58 -llibboost_thread-vc120-mt-1_58
            }
        }
    }

    OPENIGBUILD = $$(OPENIG_BUILD)
    isEmpty (OPENIGBUILD) {
        OPENIGBUILD = $$IN_PWD/..
    }
    DESTDIR = $$OPENIGBUILD/lib
    DLLDESTDIR = $$OPENIGBUILD/bin/plugins

    LIBS += -L$$OPENIGBUILD/lib

    FILE = $${PWD}/DataFiles/libIgPlugin-ForwardPlusLighting.so.xml
    DFILE = $${DLLDESTDIR}/OpenIG-Plugin-ForwardPlusLighting.dll.xml

    FILE ~= s,/,\\,g
    DFILE ~= s,/,\\,g

    QMAKE_DISTCLEAN += $$DFILE

    message(Plugin-ForwardPlusLighting copying $$FILE to $$DFILE)
    QMAKE_POST_LINK =  if not exist $$quote($$DLLDESTDIR) $$QMAKE_MKDIR $$quote($$DLLDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    SFILES   = $$files($${PWD}/shaders/*.glsl)
    SDESTDIR = $$OPENIGBUILD/bin/resources/shaders/

    SFILES ~= s,/,\\,g
    SDESTDIR ~= s,/,\\,g

#   Get the filename(only) list for distclean to remove only the files added from this plugin
    for(var,SFILES) {
        distfiles += $$SDESTDIR\\$$basename(var)
    }
    QMAKE_DISTCLEAN += $$distfiles

    PDIR = $${PWD}/shaders
    PDIR ~= s,/,\\,g

    QMAKE_POST_LINK += test -d $$quote($$SDESTDIR) || $$QMAKE_MKDIR $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PDIR\*.glsl) $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
}
