#-------------------------------------------------
#
# Project created by QtCreator 2015-01-28T11:29:59
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent

TARGET = OpenIG-Plugin-Muse
TEMPLATE = lib

DEFINES += IGPLUGINMUSE_LIBRARY

SOURCES +=  igpluginmuse.cpp\
            movingmodelmanager.cpp\
            miscellaneous.cpp

HEADERS +=  movingmodelmanager.h\
            miscellaneous.h\
            scenemodel.h

LIBS += -losg -losgDB -losgViewer -lOpenThreads \
        -lOpenIG-Engine \
        -lQtUtil -lCstShare

INCLUDEPATH += ../
DEPENDPATH += ../

OTHER_FILES += CMakeLists.txt \
               $${PWD}/DataFiles/*
DISTFILES += CMakeLists.txt

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

    INCLUDEPATH +=  /usr/local/muse/libs/inc/ \
                    /usr/local/muse/core/inc \
                    /usr/local/muse/amx/projects \
                    /usr/local/include/bullet \
                    /usr/local/include/Public_Headers
    DEPENDPATH +=   /usr/local/muse/libs/inc/ \
                    /usr/local/muse/core/inc \
                    /usr/local/muse/amx/projects \
                    /usr/local/include/bullet \
                    /usr/local/include/Public_Headers

    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    FILE      = $${PWD}/DataFiles/libIgPlugin-Muse.so.xml
    DDIR      = $${DESTDIR}/libOpenIG-Plugin-Muse.so.xml
    mac: DDIR = $${DESTDIR}/libOpenIG-Plugin-Muse.dylib.xml

    QMAKE_POST_LINK =  test -d $$quote($$DESTDIR) || $$QMAKE_MKDIR $$quote($$DESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += test -e $$quote($$DDIR) || $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)

    #remove the files we manually installed above when we do a make distclean....
    #!build_pass:message("$$escape_expand(\n)$$basename(_PRO_FILE_) Files to be removed during \"make distclean\": "$$DDIR$$escape_expand(\n))
    QMAKE_DISTCLEAN += $${DDIR}

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

win32 {
     message( "Currently there is no Windows support for $$TARGET" )
}
#In event we add Windows support will put this here...
#win32-g++:QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL
#win32-g++:LIBS += -lstdc++.dll
