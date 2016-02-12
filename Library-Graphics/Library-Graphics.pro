#-------------------------------------------------
#
# Project created by QtCreator 2015-03-29T20:22:26
#
#-------------------------------------------------

QT       -= core gui

CONFIG += silent

TARGET = OpenIG-Graphics
TEMPLATE = lib

DEFINES += IGLIBGRAPHICS_LIBRARY

SOURCES += 	ColorValue.cpp\
            DataFormat.cpp\
            Light.cpp\
            LightData.cpp\
            LightManager.cpp\
            OIGMath.cpp\
            TileSpaceLightGrid.cpp\
            UserObjectBindings.cpp


HEADERS +=  AxisAlignedBoundingBox.h \
            AxisAlignedBoundingBox.inl\
            Camera.h\
            Camera.inl\
            CameraFwdDeclare.h\
            ColorValue.h\
            CommonTypes.h\
            CommonUtils.h\
            DataFormat.h\
            Delegate.h\
            ForwardDeclare.h\
            IntSize.h\
            Light.h\
            LightData.h\
            LightManager.h\
            Matrix3.h\
            Matrix4.h\
            MatrixForwardDeclare.h\
            MatrixUtils.h\
            Octree.h\
            Octree.inl\
            OctreeFwdDeclare.h\
            OctreeNode.h\
            OctreeNode.inl\
            OctreeNodeFwdDeclare.h\
            OIGAssert.h\
            OIGMath.h\
            Plane.h\
            Plane.inl\
            ScreenRect.h\
            Signal.h\
            STLUtilities.h\
            TileSpaceLightGrid.h\
            UserObjectBindings.h\
            Vector.h\
            Vector2.h\
            Vector3.h\
            Vector4.h\
            VectorForwardDeclare.h\
            VectorUtils.h\
            export.h


INCLUDEPATH += ../
DEPENDPATH += ../

OTHER_FILES += CMakeLists.txt
DISTFILES += CMakeLists.txt

unix {
    !mac:contains(QMAKE_HOST.arch, x86_64):{
        DESTDIR = /usr/local/lib64
        LIBS+=-L/usr/local/lib64
        target.path = /usr/local/lib64
    } else {
        DESTDIR = /usr/local/lib
        LIBS+=-L/usr/local/lib
        target.path = /usr/local/lib
    }
   !build_pass:message($$basename(_PRO_FILE_) Lib will be installed into $$DESTDIR)

    INSTALLS += target

    INCLUDEPATH += /usr/local/include
    DEPENDPATH += /usr/local/include

    INCLUDEPATH += /usr/local/lib64
    DEPENDPATH += /usr/local/lib64

    #
    # Allow alternate boost library path to be set via ENV variable
    #
    BOOSTROOT = $$(BOOST_ROOT)
    isEmpty(BOOSTROOT) {
        !build_pass:message($$basename(_PRO_FILE_) -- \"BOOST_ROOT env var\" not set...using system default paths to look for boost )
        LIBS +=  -lboost_system -lboost_filesystem -lboost_date_time -lboost_regex -lboost_thread -lboost_chrono
    }
    else {
        !build_pass:message($$basename(_PRO_FILE_) -- \"BOOST_ROOT env var\" detected - set to: \"$$BOOSTROOT\")
        LIBS += -L$$BOOSTROOT/stage/lib \
                -lboost_system -lboost_filesystem -lboost_date_time -lboost_regex -lboost_thread -lboost_chrono
        INCLUDEPATH += $$BOOSTROOT
        DEPENDPATH  += $$BOOSTROOT
    }

    HFILES   = $$files($${PWD}/*.h)
    HDESTDIR = /usr/local/include/$$TARGET

#   Get the filename(only) list for distclean to remove only the files added from this plugin
#    for(var,HFILES) {
#        distfiles += $$HDESTDIR/$$basename(var)
#    }
#    QMAKE_DISTCLEAN += $$distfiles
    QMAKE_DISTCLEAN += -r $$HDESTDIR

    QMAKE_POST_LINK  = test -d $$quote($$HDESTDIR) || $$QMAKE_MKDIR $$quote($$HDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$HFILES) $$quote($$HDESTDIR) $$escape_expand(\\n\\t)

    # library version number files
    exists( "../openig_version.pri" ) {

	include( "../openig_version.pri" )
        isEmpty( VERSION ){ !build_pass:error( "$$basename(_PRO_FILE_) -- bad or undefined VERSION variable inside file openig_version.pri" )
	} else {
        !build_pass:message("$$basename(_PRO_FILE_) -- Set version info to: $$VERSION" )
	}

    }
    else { !build_pass:error( "$$basename(_PRO_FILE_) -- could not find pri library version file openig_version.pri" ) }

    # end of library version number files
}


win32-g++:QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL

win32 {
    LIBS += -lopengl32 -lglu32 -lUser32

    BOOSTROOT = $$(BOOST_ROOT)
    isEmpty(BOOSTROOT) {
       !build_pass:message($$basename(_PRO_FILE_) -- \"boost\" not detected...)
    }
    else {
        INCLUDEPATH += $$BOOSTROOT
        win32-g++ {
            !build_pass:message($$basename(_PRO_FILE_) -- win32-g++ --\"boost\" detected in \"$$BOOSTROOT\")
            LIBS += -L$$BOOSTROOT\stage\lib -llibboost_filesystem -lboost_date_time -lboost_regex -lboost_thread -lboost_chrono
        } else {
            !build_pass:message($$basename(_PRO_FILE_) -- win32 -- \"boost\" detected in \"$$BOOSTROOT\")
            LIBS += -L$$BOOSTROOT\stage\lib
            CONFIG( debug,debug|release ){
                !build_pass:message($$basename(_PRO_FILE_) -- Boost using debug version of libraries )
                LIBS += -llibboost_system-vc120-mt-gd-1_58 -llibboost_filesystem-vc120-mt-gd-1_58 -llibboost_date_time-vc120-mt-gd-1_58 -llibboost_regex-vc120-mt-gd-1_58 -llibboost_thread-vc120-mt-gd-1_58 -llibboost_chrono-vc120-mt-gd-1_58
            }else{
                !build_pass:message($$basename(_PRO_FILE_) -- Boost using release version of libraries )
                LIBS += -llibboost_system-vc120-mt-1_58 -llibboost_filesystem-vc120-mt-1_58 -llibboost_date_time-vc120-mt-1_58 -llibboost_regex-vc120-mt-1_58 -llibboost_thread-vc120-mt-1_58 -llibboost_chrono-vc120-mt-1_58
            }
        }
    }

    OPENIGBUILD = $$(OPENIG_BUILD)
    isEmpty (OPENIGBUILD) {
        OPENIGBUILD = $$IN_PWD/..
    }
    DESTDIR = $$OPENIGBUILD/lib
    DLLDESTDIR = $$OPENIGBUILD/bin

    LIBS += -L$$OPENIGBUILD/lib

    HFILES   = $$files($${PWD}/*.h)
    HDESTDIR = $$OPENIGBUILD/include/$$TARGET
    HFILES  ~= s,/,\\,g
    HDESTDIR  ~= s,/,\\,g

#   Get the filename(only) list for distclean to remove only the files added from this plugin
    for(var,HFILES) {
        distfiles += $$HDESTDIR\\$$basename(var)
    }
    QMAKE_DISTCLEAN += $$distfiles
    QMAKE_DISTCLEAN += -r $$HDESTDIR

    PDIR =  $$PWD
    PDIR  ~= s,/,\\,g

    QMAKE_POST_LINK  = test -d $$quote($$HDESTDIR) || $$QMAKE_MKDIR $$quote($$HDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$PDIR\*.h $$quote($$HDESTDIR) $$escape_expand(\\n\\t)
}
