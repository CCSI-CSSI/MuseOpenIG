#-------------------------------------------------
#
# Project created by QtCreator 2015-03-29T20:22:26
#
#-------------------------------------------------

QT -= core gui

CONFIG += silent warn_off

TARGET = OpenIG-Plugin-Bullet
TEMPLATE = lib

DEFINES += IGPLUGINBULLET_LIBRARY

SOURCES += IGPluginBullet.cpp

LIBS += -losg -losgDB -losgViewer -lOpenThreads -losgShadow -losgAnimation \
        -lOpenIG-Engine -lOpenIG-Base -lOpenIG-PluginBase -lOpenIG-Bullet

INCLUDEPATH += ../
DEPENDPATH += ../

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
    !build_pass:message($$basename(_PRO_FILE_) Libs will be installed into $$DESTDIR)

    INSTALLS += target

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/local/include/bullet
    DEPENDPATH += /usr/local/include/bullet

    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    LIBS += -lOpenIG-Bullet

    # library version number files
    exists( "../openig_version.pri" ) {

    include( "../openig_version.pri" )
        isEmpty( VERSION ){ !build_pass:error( "$$basename(_PRO_FILE_) bad or undefined VERSION variable inside file openig_version.pri" )
    } else {
        !build_pass:message($$basename(_PRO_FILE_) Set version info to: $$VERSION)
    }

    }
    else { !build_pass:error( "$$basename(_PRO_FILE_) could not find pri library version file openig_version.pri" ) }

    # end of library version number files
}


win32-g++:QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL

win32 {

    OSGROOT = $$(OSG_ROOT)
    isEmpty(OSGROOT) {
        !build_pass:message($$basename(_PRO_FILE_) \"OpenSceneGraph\" not detected...)
    }
    else {
       !build_pass:message($$basename(_PRO_FILE_) \"OpenSceneGraph\" detected in \"$$OSGROOT\")
        INCLUDEPATH += $$OSGROOT/include
        LIBS += -L$$OSGROOT/lib
    }
    OSGBUILD = $$(OSG_BUILD)
    isEmpty(OSGBUILD) {
        !build_pass:message($$basename(_PRO_FILE_) \"OpenSceneGraph build\" not detected...)
    }
    else {
        !build_pass:message($$basename(_PRO_FILE_) \"OpenSceneGraph build\" detected in \"$$OSGBUILD\")
        DEPENDPATH += $$OSGBUILD/lib
        INCLUDEPATH += $$OSGBUILD/include
        LIBS += -L$$OSGBUILD/lib
    }

    exists( C:/Program Files/osgBullet ) {
        OSGBULLETSROOT = "C:\Program Files\osgBullet"
    }
    isEmpty(OSGBULLETSROOT) {
        !build_pass:message($$basename(_PRO_FILE_) \"osgBullets\" not detected...)
    }
    else {
       !build_pass:message($$basename(_PRO_FILE_) \"osgBullets\" detected in \"$$OSGBULLETSROOT\")
        INCLUDEPATH += $$OSGBULLETSROOT\include
#        LIBS += -L$$OSGBULLETSROOT
#        LIBS += -L$$OSGBULLETSROOT\lib
#        LIBS += -losgbCollision -losgbDynamics
    }
    BULLETSBUILD = $$(BULLETS_BUILD)
    isEmpty(BULLETSBUILD) {
        !build_pass:message($$basename(_PRO_FILE_) \"Bullets build\" not detected...)
    }
    else {
        !build_pass:message($$basename(_PRO_FILE_) \"Bullets build\" detected in \"$$BULLETSBUILD\")
        DEPENDPATH += $$BULLETSBUILD
        INCLUDEPATH += $$BULLETSBUILD\src
#        LIBS += -L$$BULLETSBUILD\BUILD\lib\release
#        LIBS += -lLinearMath -lBulletCollision -lBulletDynamics -lIgLib-Bullet
    }

    OPENIGBUILD = $$(OPENIG_BUILD)
    isEmpty (OPENIGBUILD) {
        OPENIGBUILD = $$IN_PWD/..
    }
    DESTDIR = $$OPENIGBUILD/lib
    DLLDESTDIR = $$OPENIGBUILD/bin/plugins

    LIBS += -L$$OPENIGBUILD/lib
    LIBS += -lOpenIG-Bullet

}

