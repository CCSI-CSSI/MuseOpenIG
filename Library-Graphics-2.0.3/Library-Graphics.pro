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

SOURCES +=  ColorValue.cpp\
            DataFormat.cpp\
            GPGPUDevice.cpp\
            Light.cpp\
            LightData.cpp\
            LightManager.cpp\
            OIGMath.cpp\
            TileSpaceLightGrid.cpp\
            TileSpaceLightGridImpl.cpp\
            TileSpaceLightGridCPUImpl.cpp\
            UserObjectBindings.cpp

HEADERS +=  AxisAlignedBoundingBox.h\
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
            GPGPUDevice.h\
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
            TileSpaceLightGridImpl.h\
            TileSpaceLightGridCPUImpl.h\
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

    DEPENDPATH += /usr/local/lib64

    exists(/usr/local/cuda){
        USE_CUDA = true;
        CUDA_DEFINES += CUDA_FOUND=1
        DEFINES += $$CUDA_DEFINES

        message("Library Graphics found and using CUDA!!!")

        SOURCES += TileSpaceLightGridGPUImpl.cpp
                   #cuda_tile_space_light_grid.cu

        HEADERS += TileSpaceLightGridGPUImpl.h
                   #cuda_tile_space_light_grid.cuh

        CUDA_SOURCES += cuda_tile_space_light_grid.cu
        # auto-detect CUDA path
        CUDA_DIR = $$system(which nvcc | sed 's,/bin/nvcc$,,')

        ## correctly formats CUDA_DEFINES for nvcc
        for(_defines, CUDA_DEFINES):{
            formatted_defines += -D$$_defines
        }
        CUDA_DEFINES = $$formatted_defines
        message(CUDA_DEFINES: $$CUDA_DEFINES)

        INCLUDEPATH += $$CUDA_DIR/include
        DEPENDPATH += $$CUDA_DIR/include
        !contains(QMAKE_HOST.arch, x86_64) {
            message("$$basename(_PRO_FILE_) x86 build")
            QMAKE_LIBDIR += $$CUDA_DIR/lib
        } else {
            message("$$basename(_PRO_FILE_) x86_64 build")
            QMAKE_LIBDIR += $$CUDA_DIR/lib64
        }
        LIBS += -lcuda -lcudart

        CONFIG(debug, debug|release) {
            #Debug settings
            message("Using x64 Debug arch config LINUX for build")
            #read as: --compiler-options options,... + ISO-standard C++ exception handling
            # + speed over size, + create debug symbols, + code generation multi-threaded debug
            NVCC_OPTIONS += -Xcompiler -fPIC -g
        }
        else {
            #Release settings
            message("Using x64 Release arch config LINUX for build")
            #read as: --compiler-options options,... + ISO-standard C++ exception handling
            # + speed over size, + code generation multi-threaded
            NVCC_OPTIONS += -Xcompiler -fPIC
        }

        cuda.input  = CUDA_SOURCES
        cuda.output = ${OBJECTS_DIR}${QMAKE_FILE_BASE}_cuda.obj
        cuda.commands = nvcc -c -Xcompiler $$join(QMAKE_CXXFLAGS,",") $$NVCC_OPTIONS $$CUDA_DEFINES $$join(INCLUDEPATH,'" -I"',' -I"','"') ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT}
        cuda.dependcy_type = TYPE_C
        QMAKE_EXTRA_COMPILERS += cuda
    } else {
    DEFINES -= CUDA_FOUND
}

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
    HFILES  += $$files($${PWD}/*.inl)
    HFILES  += $$files($${PWD}/*.cuh)
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

#win32-g++:QMAKE_CXXFLAGS += -fpermissive -shared-libgcc -D_GLIBCXX_DLL

## SYSTEM_TYPE - compiling for 32 or 64 bit architecture
SYSTEM_TYPE = 64

win32 {
    LIBS += -lopengl32 -lglu32 -lUser32

    DEFINES += GRAPHICS_LIBRARY_HAS_NAMESPACE

#   message(LIBS: $$LIBS)
#  #Debug and Release flags necessary for compilation and linking
#  QMAKE_CFLAGS_DEBUG += /MTd
#  QMAKE_CXXFLAGS_DEBUG += /MTd
#  QMAKE_CFLAGS_RELEASE += /MT
#  QMAKE_CXXFLAGS_RELEASE += /MT
#  # The following library conflicts with something in Cuda
#  QMAKE_LFLAGS_RELEASE = /NODEFAULTLIB:msvcrt.lib
#  QMAKE_LFLAGS_DEBUG = /NODEFAULTLIB:msvcrtd.lib

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
    DESTDIR = $$OPENIGBUILD\lib
    DLLDESTDIR = $$OPENIGBUILD\bin

    LIBS += -L$$OPENIGBUILD\lib

    CUDAPATH = $$(CUDA_PATH)
        isEmpty(CUDAPATH) {
       !build_pass:message($$basename(_PRO_FILE_) -- \"CUDA\" not detected...)
    }
    else {
        ## CUDA_DIR - the directory of cuda such that CUDA\<version-number\ contains the bin, lib, src and include folders
        CUDA_DIR=$$(CUDA_PATH)

        !contains(QMAKE_TARGET.arch, x86_64) {
            message("$$basename(_PRO_FILE_) x86 build")
            QMAKE_LIBDIR += $$CUDA_DIR\lib\Win32
        } else {
            message("$$basename(_PRO_FILE_) x86_64 build")
            QMAKE_LIBDIR += $$CUDA_DIR\lib\x64
        }

        LIBS += -L$$QMAKE_LIBDIR -lcudart
        ## CUDA_COMPUTE_ARCH - This will enable nvcc to compiler appropriate architecture specific code for different compute versions.
        ## Multiple architectures can be requested by using a space to seperate. example:
        ## CUDA_COMPUTE_ARCH = 10 20 30 35
        CUDA_COMPUTE_ARCH = 20

        ## CUDA_DEFINES - The seperate defines needed for the cuda device and host methods
        CUDA_DEFINES += CUDA_FOUND=1
	   DEFINES += $$CUDA_DEFINES

        ## CUDA_SOURCES - the source (generally .cu) files for nvcc. No spaces in path names
        CUDA_SOURCES += cuda_tile_space_light_grid.cu

        ## CUDA_LIBS - the libraries to link
        CUDA_LIBS= -lcuda -lcudart

        ## CUDA_INC - all incldues needed by the cuda files (such as CUDA\<version-number\include)
        CUDA_INC += -I$$CUDA_DIR\include -I../ -I./

        INCLUDEPATH += $$CUDA_DIR\include

        ## NVCC_OPTIONS - any further options for the compiler
        NVCC_OPTIONS += --use_fast_math --ptxas-options=-v
        CONFIG(debug, debug|release) {
            #Debug settings
            message("Using x64 Debug arch config MSVC2013 for build")
            #read as: --compiler-options options,... + ISO-standard C++ exception handling
            # + speed over size, + create debug symbols, + code generation multi-threaded debug
            NVCC_OPTIONS += -Xcompiler /EHsc,/O2,/Zi,/MDd -g
        }
        else {
            #Release settings
            message("Using x64 Release arch config MSVC2013 for build")
            #read as: --compiler-options options,... + ISO-standard C++ exception handling
            # + speed over size, + code generation multi-threaded
            NVCC_OPTIONS += -Xcompiler /EHsc,/O2,/MD
        }

        SOURCES += TileSpaceLightGridGPUImpl.cpp
        HEADERS += TileSpaceLightGridGPUImpl.h

        ## correctly formats CUDA_COMPUTE_ARCH to CUDA_ARCH with code gen flags
        ## resulting format example: -gencode arch=compute_20,code=sm_20
        for(_a, CUDA_COMPUTE_ARCH):{
            formatted_arch =$$join(_a,'',' -gencode arch=compute_',',code=sm_$$_a')
            CUDA_ARCH += $$formatted_arch
        }

        ## correctly formats CUDA_DEFINES for nvcc
        for(_defines, CUDA_DEFINES):{
            formatted_defines += -D$$_defines
        }
        CUDA_DEFINES = $$formatted_defines

        #nvcc config
        CONFIG(debug, debug|release) {
                #Debug settings
                CUDA_OBJECTS_DIR = cudaobj/$$SYSTEM_NAME/Debug
                cuda_d.input = CUDA_SOURCES
                cuda_d.output = $$CUDA_OBJECTS_DIR/${QMAKE_FILE_BASE}_cuda.o
                cuda_d.commands = $$CUDA_DIR/bin/nvcc -D_DEBUG $$CUDA_DEFINES $$NVCC_OPTIONS $$CUDA_INC $$CUDA_LIBS --machine $$SYSTEM_TYPE $$CUDA_ARCH -c ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT}
                cuda_d.dependency_type = TYPE_C
                QMAKE_EXTRA_COMPILERS += cuda_d
                message(cuda_d DEBUG: $$cuda_d.commands)
        }
        else {
                # Release settings
                CUDA_OBJECTS_DIR = cudaobj/$$SYSTEM_NAME/Release
                cuda.input = CUDA_SOURCES
                cuda.output = $$CUDA_OBJECTS_DIR/${QMAKE_FILE_BASE}_cuda.o
                cuda.commands = $$CUDA_DIR/bin/nvcc $$CUDA_DEFINES $$NVCC_OPTIONS $$CUDA_INC $$CUDA_LIBS --machine $$SYSTEM_TYPE $$CUDA_ARCH -c  ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT}
                cuda.dependency_type = TYPE_C
                QMAKE_EXTRA_COMPILERS += cuda
                message(cuda RELEASE: $$cuda.commands)
        }
    }

    HFILES   = $$files($${PWD}/*.h)
    HFILES  += $$files($${PWD}/*.inl)
    HFILES  += $$files($${PWD}/*.cuh)
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
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PDIR\*.h)   $$quote($$HDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PDIR\*.inl) $$quote($$HDESTDIR) $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$PDIR\*.cuh) $$quote($$HDESTDIR) $$escape_expand(\\n\\t)
}
