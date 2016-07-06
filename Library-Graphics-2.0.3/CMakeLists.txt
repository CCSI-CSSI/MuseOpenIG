
set( LIB_NAME OpenIG-Graphics )
add_definitions( -DIGLIBGRAPHICS_LIBRARY )

if (CUDA_FOUND)
add_definitions( -DCUDA_FOUND=1 )
else()
add_definitions( -DCUDA_FOUND=0 )
endif()

if (TBB_FOUND)
add_definitions( -DTBB_FOUND=1 )
else()
add_definitions( -DTBB_FOUND=0 )
endif()

set (LIB_PUBLIC_HEADERS

	AxisAlignedBoundingBox.h
	AxisAlignedBoundingBox.inl
	Camera.h
	Camera.inl
	CameraFwdDeclare.h
	ColorValue.h
	CommonTypes.h
	CommonUtils.h
	DataFormat.h
	Delegate.h
	ForwardDeclare.h
	GPGPUDevice.h
	IntSize.h
	Light.h
	LightData.h
	LightManager.h
	Matrix3.h
	Matrix4.h
	MatrixForwardDeclare.h
	MatrixUtils.h
	Octree.h
	Octree.inl
	OctreeFwdDeclare.h
	OctreeNode.h
	OctreeNode.inl
	OctreeNodeFwdDeclare.h
	OIGAssert.h
	OIGMath.h
	Plane.h
	Plane.inl
	ScreenRect.h
	Signal.h
	STLUtilities.h
	TileSpaceLightGrid.h
	TileSpaceLightGridImpl.h
	TileSpaceLightGridCPUImpl.h
	UserObjectBindings.h
	Vector.h
	Vector2.h
	Vector3.h
	Vector4.h
	VectorForwardDeclare.h
	VectorUtils.h
	export.h
)

set (SOURCE_FILES 
	ColorValue.cpp
	DataFormat.cpp
	GPGPUDevice.cpp 
	Light.cpp
	LightData.cpp
	LightManager.cpp
	OIGMath.cpp
	TileSpaceLightGrid.cpp
	TileSpaceLightGridImpl.cpp
	TileSpaceLightGridCPUImpl.cpp
	UserObjectBindings.cpp
)

IF( CUDA_FOUND )
 set (LIB_PUBLIC_HEADERS ${LIB_PUBLIC_HEADERS}

 TileSpaceLightGridGPUImpl.h 
 cuda_tile_space_light_grid.cuh
 )

 set( SOURCE_FILES ${SOURCE_FILES}
  TileSpaceLightGridGPUImpl.cpp 
  cuda_tile_space_light_grid.cu
)
ENDIF( CUDA_FOUND )

SOURCE_GROUP(Shaders FILES ${SHADER_FILES})

IF( CUDA_FOUND )
cuda_add_library( ${LIB_NAME} SHARED
	${LIB_PUBLIC_HEADERS} 
	${SOURCE_FILES}
)
ELSE(  )
add_library( ${LIB_NAME} SHARED
	${LIB_PUBLIC_HEADERS} 
	${SOURCE_FILES}
)
ENDIF(  )

include_directories(
		${Boost_INCLUDE_DIRS}
)

IF( TBB_FOUND )
include_directories(
		${TBB_INCLUDE_DIRS}
)
ENDIF()

target_link_libraries( ${LIB_NAME} ${Boost_LIBRARIES} )

IF( TBB_FOUND )
target_link_libraries( ${LIB_NAME} ${TBB_LIBRARIES} )
ENDIF()

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Library Graphics" )
set_target_properties (${target} PROPERTIES DEFINE_SYMBOL  "IGLIBGRAPHICS_LIBRARY")

include( ModuleInstall REQUIRED )

