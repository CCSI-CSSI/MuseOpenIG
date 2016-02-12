set( LIB_NAME OpenIG-Graphics )
add_definitions( -DIGLIBGRAPHICS_LIBRARY )

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
	Light.cpp
	LightData.cpp
	LightManager.cpp
	OIGMath.cpp
	TileSpaceLightGrid.cpp
	UserObjectBindings.cpp
)

SOURCE_GROUP(Shaders FILES ${SHADER_FILES})

add_library( ${LIB_NAME} SHARED
	${LIB_PUBLIC_HEADERS} 
	${SOURCE_FILES}
)

include_directories(
		${Boost_INCLUDE_DIRS}
)

target_link_libraries( ${LIB_NAME}
	${Boost_LIBRARIES}
)

set_target_properties( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
set_target_properties( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Library Graphics" )
set_target_properties (${target} PROPERTIES DEFINE_SYMBOL  "IGLIBGRAPHICS_LIBRARY")

include( ModuleInstall REQUIRED )

