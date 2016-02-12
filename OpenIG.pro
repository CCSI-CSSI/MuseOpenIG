CONFIG  += ordered release silent
TEMPLATE = subdirs
SUBDIRS +=  Library-Graphics \
            Core-Base \
            Core-PluginBase \
            Core-OpenIG \
            Application-IG \
            Plugin-SkyDome \
            Utility-veggen \
            Utility-vegviewer \
            Utility-oigconv \
            Plugin-GPUVegetation \
            Plugin-LightsControl \
            Plugin-ForwardPlusLighting \
            Plugin-VDBOffset \
            Plugin-ModelComposition \
            Plugin-Animation \
            Plugin-FBXAnimation \
            Plugin-OSGParticleEffects \
            Library-Networking \
            Plugin-Networking \
            Application-A320

OTHER_FILES += CMakeModules/*.* \
               InstallerSpecificFiles/*.* \
               releasenotes.txt \
               README.md \
               INSTALL \
               openig_version.pri \
               CMakeLists.txt \
               OpenIG_Environmental_Vars.reg \
               LICENSE

OTHER_FILES += Resources/*.* \
               Resources/shaders/*.* \
               Resources/textures/*.*

unix {
    exists( /usr/local/lib/libSilverLining* ){
        message( "Configuring for build with Sundog-Software's SilverLining..." )
        SUBDIRS += Plugin-SilverLining
    }else{
        message( "Sundog-Software's SilverLining not found on the system. The SilverLining plugin is not included in the build..." )
    }

    exists( /usr/local/lib/libTriton* ){
        message( "Configuring for build with Sundog-Software's Triton..." )
        SUBDIRS += Plugin-Triton
    }else{
        message( "Sundog-Software's Triton not found on the system. The Triton plugin is not included in the build..." )
    }

    exists( /usr/local/lib/libCstShare* ){
        message( "Configuring for build with Legacy Muse products, legacy Muse libraries were found!!!......" )
        SUBDIRS += Plugin-Muse
    }else{
        message( "We are NOT Configuring for building with Legacy Muse products. required Muse libraries were not found!!!..." )
    }

    !mac:exists( /usr/local/lib/libBullet* ):exists( /usr/local/lib64/libosgb* ) {
        message( "Configuring for build with Bullets ..." )
        SUBDIRS += Library-Bullet
        SUBDIRS += Plugin-Bullet
        SUBDIRS += Application-Bullet
    }
    else{
        !mac:message( "Bullets not found on the system. The Bullets application is not included in the build..." )
    }

    mac:exists( /usr/local/lib/libBullet* ):exists( /usr/local/lib/libosgb* ) {
        message( "Configuring for build with Bullets ..." )
        SUBDIRS += Library-Bullet
        SUBDIRS += Plugin-Bullet
        SUBDIRS += Application-Bullet
    }
    else{
        mac:message( "Bullets not found on the system. The Bullets application is not included in the build..." )
    }

    !mac:exists( /usr/local/lib64/libosgEarth* ){
        message( "Configuring for build with osgEarth ..." )
        SUBDIRS += Plugin-OSGEarthSimpleLighting
        SUBDIRS += Application-Earth
    }else{
        !mac:message( "osgEarth not found on the system. The osgEarth application is not included in the build..." )
    }

    mac:exists( /usr/local/lib/libosgEarth* ){
        message( "Configuring for build with osgEarth ..." )
        SUBDIRS += Plugin-OSGEarthSimpleLighting
        SUBDIRS += Application-Earth
    }else{
        mac:message( "osgEarth not found on the system. The osgEarth application is not included in the build..." )
    }

    !mac:exists( /usr/local/lib64/libMyGUIEngine* ):exists( /usr/local/lib64/libMyGUI.OpenGLPlatform.a ){
        message( "Configuring for build with MyGUI ..." )
        SUBDIRS += Plugin-UI
    }else{
        message( "MyGUI not found on the Linux system. The Plug-UI is not included in the build..." )
    }
    mac:exists( /usr/local/lib/libMyGUIEngine* ):exists( /usr/local/lib/libMyGUI.OpenGLPlatform.* ){
        message( "Configuring for build with MyGUI ..." )
        SUBDIRS += Plugin-UI
    }else{
        mac:message( "MyGUI not found on the Mac system. The Plug-UI is not included in the build..." )
    }
}

win32 {
    HAS_BULLET64=FALSE
    SLROOT = $$(SILVERLINING)
    isEmpty(SLROOT) {
        message( "SilverLining not found on the system. The SilverLining plugin is not included in the build..." )
    }
    else {
        message( "Configuring Windows system build with SilverLining..." )
        SUBDIRS += Plugin-SilverLining
    }

    TROOT = $$(TRITON)
    isEmpty(TROOT) {
        message( "Triton not found on the system. The Triton plugin is not included in the build..." )
    }
    else {
        message( "Configuring Windows system build with Triton..." )
        SUBDIRS += Plugin-Triton
    }

    exists( C:/Program Files/BULLET_PHYSICS ): exists( C:/Program Files/osgBullet ) {
        message( "Configuring Windows system 64bit build with Bullets ..." )
        SUBDIRS += Library-Bullet
        SUBDIRS += Plugin-Bullet
        SUBDIRS += Application-Bullet
        HAS_BULLET64=TRUE
    }
    else{
        message( "Required Bullet libs are not found on the 64bit system. The bullet applications are not included in the build..." )
    }

    contains(HAS_BULLET64, FALSE):exists( C:/Program Files (x86)/BULLET_PHYSICS ): exists( C:/Program Files (x86)/osgBullet ){
        message( "Configuring for 32bit build with Bullets ..." )
        SUBDIRS += Library-Bullet
        SUBDIRS += Plugin-Bullet
        SUBDIRS += Application-Bullet
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
        message( "Configuring Windows system build with osgEarth ..." )
        SUBDIRS += Plugin-OSGEarthSimpleLighting
        SUBDIRS += Application-Earth
    }

    CSTSHAREROOT = $$(CSTSHARE)
    isEmpty(CSTSHAREROOT) {
        message( "We are NOT Configuring for building with Legacy Muse products. required Muse libraries were not found!!!..." )
    }
    else {
        message( "Configuring Windows system build with Legacy Muse products..." )
        SUBDIRS += Plugin-SLScene
    }

    MYGUIROOT = $$(MYGUI_ROOT)
    isEmpty(MYGUIROOT) {
        message( "MyGUI not found on the Windows system. The Plug-UI is not included in the build..." )
    }
    else {
        message( "Configuring Windows system build with MyGUI ..." )
        SUBDIRS += Plugin-UI
    }
}
