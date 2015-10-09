CONFIG  += ordered release silent
TEMPLATE = subdirs
SUBDIRS +=  IgCore \
            IgPluginCore \
            OpenIG \
            IG \
            IgPlugin-SkyDome \
            veggen \
            vegviewer \
            IgPlugin-SimpleLighting \
    IgPlugin-GPUVegetation \
    IgPlugin-Lighting \
    IgPlugin-VDBOffset \
    IgPlugin-ModelComposition \
    IgPlugin-Animation \
    IgPlugin-LightPoints \
    IgPlugin-TimeOfDayBasedMaterial \
    IgPlugin-RunwayLightsControl \
    IgPlugin-RunwayLights \
    IgPlugin-FBXAnimation \
    IgPlugin-OSGParticleEffects \
    IgLib-Networking \
    IgPlugin-Networking \
    Demo

OTHER_FILES += releasenotes.txt \
               README.md \
               INSTALL \
               openig_version.pri \
               CMakeLists.txt \
               OpenIG_Environmental_Vars.reg \
               LICENSE
unix {
    exists( /usr/local/lib/libSilverLining* ){
        message( "Configuring for build with Sundog-Software's SilverLining..." )
        SUBDIRS += IgPlugin-SilverLining
    }else{
        message( "Sundog-Software's SilverLining not found on the system. The SilverLining plugin is not included in the build..." )
    }

    exists( /usr/local/lib/libTriton* ){
        message( "Configuring for build with Sundog-Software's Triton..." )
        SUBDIRS += IgPlugin-Triton
    }else{
        message( "Sundog-Software's Triton not found on the system. The Triton plugin is not included in the build..." )
    }

    exists( /usr/local/lib/libCstShare* ){
        message( "Configuring for build with CstShare..." )
        SUBDIRS += IgPlugin-SLScene
    }else{
        message( "CstShare not found on the system. The SLScene plugin is not included in the build..." )
    }

    !mac:exists( /usr/local/lib/libBullet* ):exists( /usr/local/lib64/libosgb* ) {
        message( "Configuring for build with Bullets ..." )
        SUBDIRS += IgLib-Bullet
        SUBDIRS += IgPlugin-Bullet
        SUBDIRS += IG-Bullet
    }
    else{
        !mac:message( "Bullets not found on the system. The Bullets application is not included in the build..." )
    }

    mac:exists( /usr/local/lib/libBullet* ):exists( /usr/local/lib/libosgb* ) {
        message( "Configuring for build with Bullets ..." )
        SUBDIRS += IgLib-Bullet
        SUBDIRS += IgPlugin-Bullet
        SUBDIRS += IG-Bullet
    }
    else{
        mac:message( "Bullets not found on the system. The Bullets application is not included in the build..." )
    }

    !mac:exists( /usr/local/lib64/libosgEarth* ){
        message( "Configuring for build with osgEarth ..." )
        SUBDIRS += IgPlugin-OSGEarthSimpleLighting
        SUBDIRS += IG-Earth
    }else{
        !mac:message( "osgEarth not found on the system. The osgEarth application is not included in the build..." )
    }

    mac:exists( /usr/local/lib/libosgEarth* ){
        message( "Configuring for build with osgEarth ..." )
        SUBDIRS += IgPlugin-OSGEarthSimpleLighting
        SUBDIRS += IG-Earth
    }else{
        mac:message( "osgEarth not found on the system. The osgEarth application is not included in the build..." )
    }
}

win32 {
    HAS_BULLET64=FALSE
    SLROOT = $$(SILVERLINING)
    isEmpty(SLROOT) {
        message( "SilverLining not found on the system. The SilverLining plugin is not included in the build..." )
    }
    else {
        message( "Configuring for build with SilverLining..." )
        SUBDIRS += IgPlugin-SilverLining
    }

    exists( C:/Program Files/BULLET_PHYSICS ): exists( C:/Program Files/osgBullet ) {
        message( "Configuring for 64bit build with Bullets ..." )
        SUBDIRS += IgLib-Bullet
        SUBDIRS += IgPlugin-Bullet
        SUBDIRS += IG-Bullet
        HAS_BULLET64=TRUE
    }
    else{
        message( "Required Bullet libs are not found on the 64bit system. The bullet applications are not included in the build..." )
    }

    contains(HAS_BULLET64, FALSE):exists( C:/Program Files (x86)/BULLET_PHYSICS ): exists( C:/Program Files (x86)/osgBullet ){
        message( "Configuring for 32bit build with Bullets ..." )
        SUBDIRS += IgLib-Bullet
        SUBDIRS += IgPlugin-Bullet
        SUBDIRS += IG-Bullet
    }
    else{
        contains(HAS_BULLET64, FALSE){
            message( "Required Bullet libs are not found on the 32bit system. The bullet applications are not included in the build..." )
        }
    }

    exists( C:/Program Files/OSGEARTH ) {
        OSGEARTH_ROOT = C:\Program Files\OSGEARTH
    }
    OSGEARTHROOT = $$(OSGEARTH_ROOT)
    isEmpty(OSGEARTHROOT) {
        message( "osgEarth not found on the system. The application is not included in the build..." )
    }
    else {
        message( "Configuring for build with osgEarth ..." )
        SUBDIRS += IgPlugin-OSGEarthSimpleLighting
        SUBDIRS += IG-Earth
    }

    CSTSHAREROOT = $$(CSTSHARE)
    isEmpty(CSTSHAREROOT) {
        message( "CstShare not found on the system. The plugin is not included in the build..." )
    }
    else {
        message( "Configuring for build with CstShare..." )
        SUBDIRS += IgPlugin-SLScene
    }
}
