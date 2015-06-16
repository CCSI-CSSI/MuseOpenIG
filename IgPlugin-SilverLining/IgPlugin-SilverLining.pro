#-------------------------------------------------
#
# Project created by QtCreator 2015-01-12T12:01:03
#
#-------------------------------------------------

QT       -= core gui

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

LIBS += -losg -losgDB -losgViewer -lOpenThreads -lIgPluginCore -lIgCore

unix {
    !mac:contains(QMAKE_HOST.arch, x86_64):{
        DESTDIR = /usr/local/lib64/igplugins
        target.path = /usr/local/lib64/igplugins
    } else {
        DESTDIR = /usr/local/lib/igplugins
        target.path = /usr/local/lib/igplugins
    }
    message(Libs will be installed into $$DESTDIR)

    INSTALLS += target

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    INCLUDEPATH += /usr/local/include/Public_Headers
    DEPENDPATH += /usr/local/include/Public_Headers

    LIBS += -lSilverLiningOpenGL

    FILE = $${PWD}/libIgPlugin-SilverLining.so.xml
    DDIR = $${DESTDIR}/libIgPlugin-SilverLining.so.xml

    mac: DDIR = $${DESTDIR}/libIgPlugin-SilverLining.dylib.xml

    QMAKE_POST_LINK =  test -d $$quote($$DESTDIR) || $$QMAKE_MKDIR $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DDIR) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    !mac: LIBS += -lGL -lGLU
    mac: LIBS += -framework openGL

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

OTHER_FILES += libIgPlugin-SilverLining.so.xml

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

    OPENIGBUILD = $$(OPENIG_BUILD)
    isEmpty (OPENIGBUILD) {
        OPENIGBUILD = $$IN_PWD/..
    }
    DESTDIR = $$OPENIGBUILD/lib
    DLLDESTDIR = $$OPENIGBUILD/bin/igplugins

    LIBS += -L$$OPENIGBUILD/lib

    SLROOT = "E:\SilverLining SDK(Full source)"
    isEmpty(SLROOT) {
        message(\"SilverLining\" not detected...)
    }
    else {
        message(\"SilverLining\" detected in \"$$SLROOT\")
        INCLUDEPATH += $$quote(\"$$SLROOT/Public Headers\")
        message($$INCLUDEPATH)
    }
    SLBUILD = "E:\SilverLining SDK(Full source)\lib\vc12\x64"
    isEmpty(SLBUILD) {
        message(\"SilverLining build\" not detected...)
    }
    else {
        message(\"SilverLining build\" detected in \"$$SLBUILD\")
        DEPENDPATH += $$SLBUILD
	LIBS += -L$$SLBUILD -lSilverLining-MT-DLL
    }

    FILE = $${PWD}/libIgPlugin-SilverLining.so.windows.xml
    DFILE = $${DLLDESTDIR}/IgPlugin-SilverLining.dll.xml

    FILE ~= s,/,\\,g
    DFILE ~= s,/,\\,g

    QMAKE_POST_LINK =  if not exist $$quote($$DLLDESTDIR) $$QMAKE_MKDIR $$quote($$DLLDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += if not exist $$quote($$DFILE) copy /y $$quote($$FILE) $$quote($$DFILE) $$escape_expand(\\n\\t)
}
