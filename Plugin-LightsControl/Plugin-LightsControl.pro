#-------------------------------------------------
#
# Project created by QtCreator 2015-04-28T19:29:56
#
#-------------------------------------------------

QT       -= core gui
CONFIG +=  silent warn_off

TARGET = OpenIG-Plugin-LightsControl
TEMPLATE = lib

DEFINES += IGPLUGINLIGHTSCONTROL_LIBRARY

SOURCES += IGPluginLightsControl.cpp

HEADERS +=

INCLUDEPATH += ../
DEPENDPATH += ../

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgGA -losgText -losgShadow -losgSim -losgParticle\
        -lOpenIG-Engine -lOpenIG-Base -lOpenIG-PluginBase -lOpenIG-Graphics -lOpenIG-Utils

#LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgShadow -losgUtil -losgSim -losgText -losgGA -losgTexture\
#        -lOpenIG-Engine -lOpenIG-Base -lOpenIG-Graphics -lOpenIG-PluginBase -lOpenIG-Utils

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
    !build_pass:message($$TARGET Lib will be installed into $$DESTDIR)

    #
    # Allow alternate boost library path to be set via ENV variable
    #
    BOOSTROOT = $$(BOOST_ROOT)
    isEmpty(BOOSTROOT) {
       !build_pass:message($$basename(_PRO_FILE_) -- \"BOOST_ROOT env var\" not set...using system default paths to look for boost )
        LIBS +=  -lboost_date_time -lboost_thread -lboost_filesystem -lboost_system
    }
    else {
        !build_pass:message($$basename(_PRO_FILE_) -- \"BOOST_ROOT env var\" detected - set to: \"$$BOOSTROOT\")
        LIBS += -L$$BOOSTROOT/stage/lib \
                -lboost_system -lboost_filesystem -lboost_thread
        INCLUDEPATH += $$BOOSTROOT
        DEPENDPATH  += $$BOOSTROOT
    }

    INSTALLS += target

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    SFILES  = $$files($${PWD}/../Resources/shaders/lightpoint_*.glsl)
    SFILES += $$files($${PWD}/../Resources/shaders/sprite_bb_*.glsl)
    TFILES += $$files($${PWD}/../Resources/textures/*)

    SDESTDIR = /usr/local/openig/resources/shaders/
    TDESTDIR = /usr/local/openig/resources/textures/

#   Get the filename(only) list for distclean to remove only the files added from this plugin
    for(var,SFILES) {
        distfiles += $$SDESTDIR$$basename(var)
    }
    for(var,TFILES) {
        distfiles += $$TDESTDIR$$basename(var)
    }
    QMAKE_DISTCLEAN += $$distfiles

    QMAKE_POST_LINK  = test -d $$quote($$SDESTDIR) || $$QMAKE_MKDIR $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -d $$quote($$TDESTDIR) || $$QMAKE_MKDIR $$quote($$TDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$SFILES) $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$TFILES) $$quote($$TDESTDIR) $$escape_expand(\\n\\t)

    # library version number files
    exists( "../openig_version.pri" ) {

    include( "../openig_version.pri" )
        isEmpty( VERSION ){ !build_pass:error( "$$TARGET -- bad or undefined VERSION variable inside file openig_version.pri" )
    } else {
        !build_pass:message( "$$TARGET -- Set version info to: $$VERSION" )
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

    SFILES   = $$files($${PWD}/../Resources/shaders/lightpoint_*.glsl)
    SFILES  += $$files($${PWD}/../Resources/shaders/sprite_bb_*.glsl)
    SDESTDIR = $$OPENIGBUILD/bin/resources/shaders/

    SFILES   ~= s,/,\\,g
    SDESTDIR ~= s,/,\\,g

    TFILES   = $$files($${PWD}/../Resources/textures/*.*)
    TDESTDIR = $$OPENIGBUILD/bin/resources/textures/

    TFILES   ~= s,/,\\,g
    TDESTDIR ~= s,/,\\,g

#   Get the filename(only) list for distclean to remove only the files added from this plugin
    for(var,SFILES) {
        distfiles += $$SDESTDIR$$basename(var)#shaders and/or textures if any
    }
    for(var,TFILES) {
        distfiles += $$TDESTDIR$$basename(var)#shaders and/or textures if any
    }

    PDIR =  $$PWD
    PDIR  ~= s,/,\\,g

#    !build_pass:message("$$escape_expand(\n)Files to be installed to: "$$SDESTDIR" =: "$$files($$PDIR\..\Resources\shaders\*.glsl) $$escape_expand(\n\n))
#    !build_pass:message("$$escape_expand(\n)Files to be installed to: "$$TDESTDIR" =: "$$files($$PDIR\..\Resources\textures\*.*) $$escape_expand(\n\n))
#    !build_pass:message("$$escape_expand(\n)Files to be removed during \"make distclean\": "$$distfiles$$escape_expand(\n\n))
    QMAKE_DISTCLEAN += $$distfiles

    QMAKE_POST_LINK  = test -d $$quote($$SDESTDIR) || $$QMAKE_MKDIR $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PDIR\..\Resources\shaders\lightpoint_*.glsl) $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PDIR\..\Resources\shaders\sprite_bb_*.glsl) $$quote($$SDESTDIR) $$escape_expand(\\n\\t)

    QMAKE_POST_LINK += test -d $$quote($$TDESTDIR) || $$QMAKE_MKDIR $$quote($$TDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PDIR\..\Resources\textures\*.*) $$quote($$TDESTDIR) $$escape_expand(\\n\\t)
}
