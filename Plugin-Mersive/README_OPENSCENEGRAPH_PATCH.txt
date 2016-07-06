Diff provided below was approximately against OSG developer release 3.5.3

This patch is needed for the Mersive plugin to properly  handle OSG shadows
due to how the Mersive plugin handles its projection matrix's.  OSG's
current code will reject them and not create shadows properly.  This code
will get the Projection Matrix from a UserValue stored with the current Camera
within the Mersive Plugin's updateCallback method.

In most cases I think it should be able to be modified to work with
most versions of OpenSceneGraph.


diff --git a/src/osgShadow/MinimalShadowMap.cpp b/src/osgShadow/MinimalShadowMap.cpp
index 46fdd4b..41a5ef6 100644
--- a/src/osgShadow/MinimalShadowMap.cpp
+++ b/src/osgShadow/MinimalShadowMap.cpp
@@ -278,7 +278,16 @@ void MinimalShadowMap::ViewData::cullShadowReceivingScene( )
 {
     BaseClass::ViewData::cullShadowReceivingScene( );
 
-    _clampedProjection = *_cv->getProjectionMatrix();
+    //If we put our matrix in the ProjectionMatrix Userspace get it, else use the
+    //default one from the camera...
+    if(!_cv->getCurrentCamera()->getUserValue("ProjectionMatrix", _clampedProjection))
+    {
+        //OSG_NOTICE<<"NOTICE::cullShadowReceivingScene: Using default camera's (\"ProjectionMatrix\""<<std::endl;
+        _clampedProjection = *_cv->getProjectionMatrix();
+    }
+    //else
+    //    OSG_NOTICE<<"NOTICE::cullShadowReceivingScene: Using  getUserValue(\"ProjectionMatrix\" for ProjectionMatrix"<<std::endl;
+
 
     if( _cv->getComputeNearFarMode() ) {

