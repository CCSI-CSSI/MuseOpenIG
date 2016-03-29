#-------------------------------------------------
#
# Project created by QtCreator 2015-01-12T12:01:03
#
#-------------------------------------------------

QT       -= core gui

#CONFIG += silent

TARGET = OpenIG-Plugin-Triton
TEMPLATE = lib

DEFINES += IGPLUGINTRITON_LIBRARY

CONFIG -= warn_on

SOURCES +=	igplugintriton.cpp \
		TritonDrawable.cpp \
		PlanarReflection.cpp

HEADERS +=	TritonDrawable.h \
		PlanarReflection.h

INCLUDEPATH += ../
DEPENDPATH += ../

LIBS += -losg -losgDB -losgViewer -losgUtil -lOpenThreads\
        -lOpenIG-Engine -lOpenIG-Base -lOpenIG-PluginBase

OTHER_FILES += CMakeLists.txt
DISTFILES += CMakeLists.txt

OTHER_FILES += \
       $${PWD}/DataFiles/*

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

    INCLUDEPATH += /usr/local/include/Triton
    DEPENDPATH += /usr/local/include/Triton

    LIBS += -lTriton -lfftss

   #
   # Allow an alternate boost library path to be set via ENV variable
   #
   BOOSTROOT = $$(BOOST_ROOT)
   isEmpty(BOOSTROOT) {
      !build_pass:message($$basename(_PRO_FILE_) -- \"BOOST_ROOT env var\" not set...using system default paths to look for boost )
       LIBS +=  -lboost_thread -lboost_system -lboost_filesystem
   }
   else {
       !build_pass:message($$basename(_PRO_FILE_) -- \"BOOST_ROOT env var\" detected - set to: \"$$BOOSTROOT\")
       LIBS += -L$$BOOSTROOT/stage/lib \
               -lboost_thread -lboost_system -lboost_filesystem
       INCLUDEPATH += $$BOOSTROOT
       DEPENDPATH  += $$BOOSTROOT
   }

    FILE = $${PWD}/DataFiles/libIgPlugin-Triton.so.xml
    DDIR = $${DESTDIR}/libOpenIG-Plugin-Triton.so.xml

    mac: DDIR = $${DESTDIR}/libOpenIG-Plugin-Triton.dylib.xml

    QMAKE_POST_LINK =  test -d $$quote($$DESTDIR) || $$QMAKE_MKDIR $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DDIR) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    #!build_pass:message("$$escape_expand(\n)Files to be removed during \"make distclean\": "$$DDIR$$escape_expand(\n\n))
    QMAKE_DISTCLEAN += $${DDIR}

    !mac: LIBS += -lGL -lGLU
    mac: LIBS += -framework openGL

    TPATH = $$(TRITON_PATH)
    isEmpty(TPATH){
        message(TRITON_PATH not set in your environment -- cannot install the OIG Triton shaders properly!!!!)
    } else {
        SFILES   = $$files($${PWD}/../Resources/shaders/Triton/user*.glsl)
        for(file,SFILES) {
            exists( $$file ) {
                SDESTDIR = $$(TRITON_PATH)/Resources/

                #Get the filename(only) list for distclean to remove only the files added from this plugin
                #for(var,SFILES) {
                #    distfiles += $$SDESTDIR/$$basename(var)
                #}
                #Do we want to delete these files or not??  Probably better to find a way to restore original files instead
                #so will need to modify this area to save original files before copying ours and overwriting the original ones.
                #message(Will remove -- $$distfiles -- during distclean)
                #QMAKE_DISTCLEAN += $$distfiles

                QMAKE_POST_LINK += test -d $$quote($$SDESTDIR) || $$QMAKE_MKDIR $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
                exists($$SDESTDIR) {
                    QMAKE_POST_LINK += cp $$file $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
                    !build_pass:message(Installing the OIG Triton shader -- $$file -- into $$SDESTDIR)
                } else {
                    message(Unable to install files into missing directory: $$SDESTDIR!!!!!!!!!!)
                }
            }
        }

        SFILES   = $${PWD}/../Resources/shaders/Triton/forward_plus_triton_ps.glsl
        exists( $$SFILES ){
            SDESTDIR = /usr/local/openig/resources/shaders/
            distfiles += $$SDESTDIR

            !build_pass:message(Installing the OIG F+ Triton shader -- $$SFILES -- into $$SDESTDIR)
            QMAKE_POST_LINK += test -d $$quote($$dirname(SDESTDIR)) || $$QMAKE_MKDIR $$quote($$dirname(SDESTDIR)) $$escape_expand(\\n\\t)
            QMAKE_POST_LINK += $$QMAKE_COPY $$SFILES $$quote($$SDESTDIR) $$escape_expand(\\n\\t)

            QMAKE_DISTCLEAN += $$distfiles
        } else {
            message(Unable to install the OIG F+ Triton shader!!!!!!!!!!!!!!!!)
        }
    }

    # library version number files
    exists( "../openig_version.pri" ) {

	include( "../openig_version.pri" )
        isEmpty( VERSION ){ !build_pass:error($$basename(_PRO_FILE_) -- bad or undefined VERSION variable inside file openig_version.pri)
	} else {
        !build_pass:message($$basename(_PRO_FILE_) -- Set version info to: $$VERSION)
	}

    }
    else { !build_pass:error($$basename(_PRO_FILE_) -- could not find pri library version file openig_version.pri) }

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
    !build_pass:message($$basename(_PRO_FILE_) -- \"openig build\" detected in \"$$OPENIGBUILD\")
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

    TROOT = $$(TRITON)
    isEmpty(TROOT) {
        !build_pass:message($$basename(_PRO_FILE_) -- \"Triton\" not detected...)
    }
    else {
        !build_pass:message($$basename(_PRO_FILE_) -- \"Triton\" detected in \"$$TROOT\")
        INCLUDEPATH += $$TROOT\Triton
        !build_pass:message($$basename(_PRO_FILE_) $$INCLUDEPATH)
    }
    TBUILD = $$(TRITON_BUILD)
    isEmpty(TBUILD) {
        !build_pass:message($$basename(_PRO_FILE_) -- \"Triton build\" not detected...)
    }
    else {
        !build_pass:message($$basename(_PRO_FILE_) -- \"Triton build\" detected in \"$$TBUILD\")
        DEPENDPATH += $$TBUILD
        INCLUDEPATH += $$TROOT\\"Public Headers"
        !build_pass:message($$basename(_PRO_FILE_) -- \"Triton includes\" detected at $$quote($$INCLUDEPATH))
        LIBS += -L$$TBUILD\lib\vc12\x64 -lTriton-MT-DLL
    }

    FILE = $${PWD}/DataFiles/libIgPlugin-Triton.so.windows.xml
    DFILE = $${DLLDESTDIR}/OpenIG-Plugin-Triton.dll.xml

    FILE ~= s,/,\\,g
    DFILE ~= s,/,\\,g

    QMAKE_POST_LINK =  if not exist $$quote($$DLLDESTDIR) $$QMAKE_MKDIR $$quote($$DLLDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    #!build_pass:message("$$escape_expand(\n)Files to be removed during \"make distclean\": "$$DFILE$$escape_expand(\n\n))
    QMAKE_DISTCLEAN += $${DFILE}

    TPATH = $$(TRITON_PATH)
    isEmpty(TPATH){
        message(TRITON_PATH not set in your environment, cannot install the OIG Triton shaders properly!!!!)
    } else {
        SFILES   = $$files($${PWD}/DataFiles/user*.glsl)
        SDESTDIR = $$(TRITON_PATH)/Resources/
        SFILES  ~= s,/,\\,g
        SDESTDIR  ~= s,/,\\,g

        #Do we want to delete these files or not??  Probably better to find a way to restore original files instead
        #so will need to modify this area to save original files before copying ours and overwriting the original ones.
        #message(Will remove -- $$distfiles -- during distclean)

        #Get the filename(only) list for distclean to remove only the files added from this plugin
#        for(var,SFILES) {
#            distfiles += $$SDESTDIR\\$$basename(var)
#        }

        PDIR =  $$PWD/DataFiles/
        PDIR  ~= s,/,\\,g

        !build_pass:message(Installing the OIG Triton shaders -- $$files($$PDIR\user*.glsl) -- into $$SDESTDIR)
        QMAKE_POST_LINK += test -d $$quote($$SDESTDIR) || $$QMAKE_MKDIR $$quote($$SDESTDIR) $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += $$QMAKE_COPY $$PDIR\user*.glsl $$quote($$SDESTDIR) $$escape_expand(\\n\\t)

        SFILES   = $${PWD}/DataFiles/forward_plus_triton_ps.glsl
        SDESTDIR = $$OPENIGBUILD/bin/resources/shaders/forward_plus_triton_ps.glsl

        SFILES    ~= s,/,\\,g
        SDESTDIR  ~= s,/,\\,g
        distfiles += $$SDESTDIR

        !build_pass:message(Installing the OIG F+ SilverLining shader -- $$SFILES -- into $$quote($$dirname(SDESTDIR)))
        QMAKE_POST_LINK += test -d $$quote($$dirname(SDESTDIR)) || $$QMAKE_MKDIR $$quote($$dirname(SDESTDIR)) $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += $$QMAKE_COPY $$SFILES $$quote($$dirname(SDESTDIR)) $$escape_expand(\\n\\t)

        QMAKE_DISTCLEAN += $$distfiles

    }
}
