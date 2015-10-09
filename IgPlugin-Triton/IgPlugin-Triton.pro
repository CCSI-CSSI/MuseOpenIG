#-------------------------------------------------
#
# Project created by QtCreator 2015-01-12T12:01:03
#
#-------------------------------------------------

QT       -= core gui

#CONFIG += silent

TARGET = IgPlugin-Triton
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

LIBS += -losg -losgDB -losgViewer -losgUtil -lOpenThreads -lOpenIG -lIgPluginCore -lIgCore

OTHER_FILES += \
       $${PWD}/ConfigFiles/*


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

    INCLUDEPATH += /usr/local/include/Triton
    DEPENDPATH += /usr/local/include/Triton

    LIBS += -lTriton -lfftss

    FILE = $${PWD}/ConfigFiles/libIgPlugin-Triton.so.xml
    DDIR = $${DESTDIR}/libIgPlugin-Triton.so.xml

    mac: DDIR = $${DESTDIR}/libIgPlugin-Triton.dylib.xml

    QMAKE_POST_LINK =  test -d $$quote($$DESTDIR) || $$QMAKE_MKDIR $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DDIR) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    QMAKE_DISTCLEAN += $${DESTDIR}/libIgPlugin-Triton.*.xml

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
    message($$TARGET -- \"openig build\" detected in \"$$OPENIGBUILD\")
    DESTDIR = $$OPENIGBUILD/lib
    DLLDESTDIR = $$OPENIGBUILD/bin/igplugins

    LIBS += -L$$OPENIGBUILD/lib

    TROOT = $$(TRITON)
    isEmpty(TROOT) {
        message($$TARGET -- \"Triton\" not detected...)
    }
    else {
        message($$TARGET -- \"Triton\" detected in \"$$TROOT\")
        INCLUDEPATH += $$TROOT\Triton
        message($$INCLUDEPATH)
    }
    TBUILD = $$(TRITON_BUILD)
    isEmpty(TBUILD) {
        message($$TARGET -- \"Triton build\" not detected...)
    }
    else {
        message($$TARGET -- \"Triton build\" detected in \"$$TBUILD\")
        DEPENDPATH += $$TBUILD
        INCLUDEPATH += $$TROOT\\"Public Headers"
        message($$TARGET -- \"Triton includes\" detected at $$quote($$INCLUDEPATH))
        LIBS += -L$$TBUILD\lib\vc12\x64 -lTriton-MT-DLL
    }

    FILE = $${PWD}/ConfigFiles/libIgPlugin-Triton.so.windows.xml
    DFILE = $${DLLDESTDIR}/IgPlugin-Triton.dll.xml

    FILE ~= s,/,\\,g
    DFILE ~= s,/,\\,g

    QMAKE_POST_LINK =  if not exist $$quote($$DLLDESTDIR) $$QMAKE_MKDIR $$quote($$DLLDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += if not exist $$quote($$DFILE) copy /y $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    QMAKE_DISTCLEAN += $${DLLDESTDIR}/libIgPlugin-Triton.*.xml

}
