#-------------------------------------------------
#
# Project created by QtCreator 2015-01-12T12:01:03
#
#-------------------------------------------------

QT -= core gui

CONFIG += silent

TARGET = OpenIG-Plugin-SilverLining
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

LIBS += -losg -losgDB -losgViewer -lOpenThreads\
        -lOpenIG-Engine -lOpenIG-Base -lOpenIG-PluginBase

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

    LIBS += -lSilverLiningOpenGL

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

    FILE = $${PWD}/DataFiles/libIgPlugin-SilverLining.so.xml
    DDIR = $${DESTDIR}/libOpenIG-Plugin-SilverLining.so.xml

    mac: DDIR = $${DESTDIR}/libOpenIG-Plugin-SilverLining.dylib.xml

    QMAKE_POST_LINK =  test -d $$quote($$DESTDIR) || $$QMAKE_MKDIR $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DDIR) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    #!build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$DDIR$$escape_expand(\n))
    QMAKE_DISTCLEAN += $${DDIR}

    !mac: LIBS += -lGL -lGLU
    mac: LIBS += -framework openGL

    SPATH = $$(SILVERLINING_PATH)
    isEmpty(SPATH){
        message(SILVERLINING_PATH not set in your environment -- cannot install the OIG SilverLining shaders properly!!!!)
    } else {
        SFILES   = $$files($${PWD}/../Resources/shaders/SilverLining/WithForwardPlusAndLogZ/UserFunctions*.glsl)
        for(file,SFILES) {
            exists( $$file ) {
                SDESTDIR = $$SPATH/Resources/Shaders/
                #Do we want to delete these files or not??  Probably better to find a way to restore original files instead
                #so will need to modify this area to save original files before copying ours and overwriting the original ones.
                #message(Will remove -- $$distfiles -- during distclean)

                #Get the filename(only) list for distclean to remove only the files added from this plugin
                #for(var,SFILES) {
                #    distfiles += $$SDESTDIR/$$basename(var)
                #}
                #QMAKE_DISTCLEAN += $$distfiles
                PDIR =  $$PWD/DataFiles/

                QMAKE_POST_LINK += test -d $$quote($$SDESTDIR) || $$QMAKE_MKDIR $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
                exists($$SDESTDIR) {
                    QMAKE_POST_LINK += cp $$file $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
                    !build_pass:message(Installing the OIG SilverLining shaders -- $$file -- into $$SDESTDIR)
                } else {
                    message(Unable to install files into missing directory: $$SDESTDIR!!!!!!!!!!)
                }
            } else {
                message(Unable to install the OIG SilverLining UserFunctions* Shaders!!!!!!!!!!)
            }
        }

        SFILES   = $${PWD}/../Resources/shaders/SilverLining/WithForwardPlusAndLogZ/forward_plus_sl_ps.glsl
        exists( $$SFILES ){
            SDESTDIR = /usr/local/openig/resources/shaders/
            distfiles += $$SDESTDIR

            !build_pass:message(Installing the OIG F+ SilverLining shader -- $$files($$SFILES) -- into $$quote($$SDESTDIR))
            QMAKE_POST_LINK += test -d $$quote($$dirname(SDESTDIR)) || $$QMAKE_MKDIR $$quote($$dirname(SDESTDIR)) $$escape_expand(\\n\\t)
            QMAKE_POST_LINK += $$QMAKE_COPY $$SFILES $$quote($$SDESTDIR) $$escape_expand(\\n\\t)

            QMAKE_DISTCLEAN += $$distfiles
        } else {
            message(Unable to install the OIG F+ SilverLining shader!!!!!!!!!!!!!!!!)
        }
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

    SLROOT = $$(SILVERLINING)
    isEmpty(SLROOT) {
        !build_pass:message($$basename(_PRO_FILE_) -- \"SilverLining\" not detected...)
    }
    else {
        !build_pass:message($$basename(_PRO_FILE_) -- \"SilverLining\" detected in \"$$SLROOT\")
        INCLUDEPATH += $$quote(\"$$SLROOT\Public Headers\")
        !build_pass:message($$basename(_PRO_FILE_) -- $$INCLUDEPATH)
    }
    SLBUILD = $$(SILVERLINING_BUILD)
    isEmpty(SLBUILD) {
        !build_pass:message($$basename(_PRO_FILE_) -- \"SilverLining build\" not detected...)
    }
    else {
       !build_pass:message($$basename(_PRO_FILE_) -- \"SilverLining build\" detected in \"$$SLBUILD\")
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

    FILE = $${PWD}/DataFiles/libIgPlugin-SilverLining.so.windows.xml
    DFILE = $${DLLDESTDIR}/OpenIG-Plugin-SilverLining.dll.xml

    FILE ~= s,/,\\,g
    DFILE ~= s,/,\\,g

    QMAKE_POST_LINK =  if not exist $$quote($$DLLDESTDIR) $$QMAKE_MKDIR $$quote($$DLLDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    #!build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$DFILE$$escape_expand(\n))
    QMAKE_DISTCLEAN += $${DFILE}

    SPATH = $$(SILVERLINING_PATH)
    isEmpty(SPATH){
        message(SILVERLINING_PATH not set in your environment -- cannot install the OIG SilverLining shaders properly!!!!)
    } else {
        SFILES   = $$files(..\Resources\shaders\SilverLining\WithForwardPlusAndLogZ\UserFunctions*.*)
        SDESTDIR = $$SPATH/Resources/Shaders/
        SFILES   ~= s,/,\\,g
        SDESTDIR ~= s,/,\\,g

        #Get the filename(only) list for distclean to remove only the files added from this plugin
#        for(var,SFILES) {
#            distfiles += $$SDESTDIR\\$$basename(var)
#        }
        #Do we want to delete these files or not??  Probably better to find a way to restore original files instead
        #so will need to modify this area to save original files before copying ours and overwriting the original ones.
        #message(Will remove -- $$distfiles -- during distclean)
        #QMAKE_DISTCLEAN += $$distfiles

        PDIR =  $$PWD
        PDIR  ~= s,/,\\,g

        #!build_pass:message(Installing the OIG SilverLining shaders -- $$SFILES -- into $$quote($$dirname(SDESTDIR)))
        QMAKE_POST_LINK += test -d $$quote($$SDESTDIR) || $$QMAKE_MKDIR $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += xcopy /y $${PDIR}\..\Resources\shaders\SilverLining\WithForwardPlusAndLogZ\UserFunction*.glsl $$quote($$dirname(SDESTDIR)) $$escape_expand(\\n\\t)

        SFILES   = $${PDIR}\..\Resources\shaders\SilverLining\WithForwardPlusAndLogZ\forward_plus_sl_ps.glsl
        SDESTDIR = $$OPENIGBUILD/bin/resources/shaders/forward_plus_sl_ps.glsl

        SFILES   ~= s,/,\\,g
        SDESTDIR ~= s,/,\\,g
        distfiles += $$SDESTDIR

        #!build_pass:message(Installing the OIG F+ SilverLining shader -- $$PDIR\forward*.glsl -- into $$quote($$dirname(SDESTDIR)))
        QMAKE_POST_LINK += test -d $$quote($$dirname(SDESTDIR)) || $$QMAKE_MKDIR $$quote($$dirname(SDESTDIR)) $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += $$QMAKE_COPY $${PDIR}\..\Resources\shaders\SilverLining\WithForwardPlusAndLogZ\forward_plus_sl_ps.glsl $$quote($$dirname(SDESTDIR)) $$escape_expand(\\n\\t)

        QMAKE_DISTCLEAN += $$distfiles
    }

}
