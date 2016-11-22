SET( LIB_NAME OpenIG-Graphics )
ADD_DEFINITIONS( -DIGLIBGRAPHICS_LIBRARY )

SET (LIB_PUBLIC_HEADERS

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
    Export.h
)

SET (SOURCE_FILES
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

ADD_LIBRARY( ${LIB_NAME} SHARED
    ${LIB_PUBLIC_HEADERS}
    ${SOURCE_FILES}
)

INCLUDE_DIRECTORIES(
        ${Boost_INCLUDE_DIRS}
)

TARGET_LINK_LIBRARIES( ${LIB_NAME}
    ${Boost_LIBRARIES}
)

SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES VERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES SOVERSION ${OPENIG_VERSION} )
SET_TARGET_PROPERTIES( ${LIB_NAME} PROPERTIES PROJECT_LABEL "Library Graphics" )
SET_TARGET_PROPERTIES (${target} PROPERTIES DEFINE_SYMBOL  "IGLIBGRAPHICS_LIBRARY")

INCLUDE( ModuleInstall REQUIRED )

