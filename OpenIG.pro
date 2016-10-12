CONFIG  += ordered release silent
TEMPLATE = subdirs
SUBDIRS +=  Library-Graphics \
            Core-Base \
            Core-PluginBase \
            Core-OpenIG \
            Core-Utils \
            Plugin-SkyDome \
            Utility-veggen \
            Utility-vegviewer \
            Utility-oigconv \
            Plugin-GPUVegetation \
            Plugin-LightsControl \
            Plugin-SimpleLighting \
            Plugin-ForwardPlusLighting \
            Plugin-VDBOffset \
            Plugin-ModelComposition \
            Plugin-Animation \
            Plugin-FBXAnimation \
            Plugin-OSGParticleEffects \
            Library-Networking \
            Library-Protocol \
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
               LICENSE \
               Resources/*.* \
               Resources/shaders/*.* \
               Resources/shaders/SilverLining/Original/*.* \
               Resources/shaders/SilverLining/WithForwardPlusAndLogZ/*.* \
               Resources/shaders/SilverLining/WithSimpleLighting/*.* \
               Resources/shaders/Triton/*.* \
               Resources/textures/*.*

unix {
#SILVERLINING
    exists( /usr/local/lib/libSilverLining* ){
        message( "Configuring system to build with Sundog-Software's SilverLining..." )
        SUBDIRS += Plugin-SilverLining
    }else{
        message( "Sundog-Software's SilverLining not found on the system. The SilverLining plugin is not included in the build..." )
    }

#TRITON
    exists( /usr/local/lib/libTriton* ){
        message( "Configuring system to build with Sundog-Software's Triton..." )
        SUBDIRS += Plugin-Triton
    }else{
        message( "Sundog-Software's Triton not found on the system. The Triton plugin is not included in the build..." )
    }

#CSTSHARE
    exists( /usr/local/lib/libCstShare* ){
        message( "Configuring system to build with Legacy Muse products, legacy Muse libraries were found!!!......" )
        SUBDIRS += Plugin-Muse
    }else{
        message( "We are NOT Configuring for building with Legacy Muse products. required Muse libraries were not found!!!..." )
    }

#BULLET
    !mac:exists( /usr/local/lib/libBullet* ):exists( /usr/local/lib64/libosgb* ) {
        message( "Configuring system to build with Bullets ..." )
        SUBDIRS += Library-Bullet
        SUBDIRS += Plugin-Bullet
        SUBDIRS += Application-Bullet
    }
    else{
        !mac:message( "Bullets not found on the system. The Bullets application is not included in the build..." )
    }
    mac:exists( /usr/local/lib/libBullet* ):exists( /usr/local/lib/libosgb* ) {
        message( "Configuring system to build with Bullets ..." )
        SUBDIRS += Library-Bullet
        SUBDIRS += Plugin-Bullet
        SUBDIRS += Application-Bullet
    }
    else{
        mac:message( "Bullets not found on the system. The Bullets application is not included in the build..." )
    }

#OSGEarth
    !mac:exists( /usr/local/lib64/libosgEarth* ){
        message( "Configuring system to build with osgEarth ..." )
        SUBDIRS += Plugin-OSGEarthSimpleLighting
        SUBDIRS += Application-Earth
    }else{
        !mac:message( "osgEarth not found on the system. The osgEarth application is not included in the build..." )
    }
    mac:exists( /usr/local/lib/libosgEarth* ){
        message( "Configuring system to build with osgEarth ..." )
        SUBDIRS += Plugin-OSGEarthSimpleLighting
        SUBDIRS += Application-Earth
    }else{
        mac:message( "osgEarth not found on the system. The osgEarth application is not included in the build..." )
    }

#MYGUI
    !mac:exists( /usr/local/lib64/libMyGUIEngine* ):exists( /usr/local/lib64/libMyGUI.OpenGLPlatform.a ){
        message( "Configuring system to build with MyGUI ..." )
        SUBDIRS += Plugin-UI
    }else{
        message( "MyGUI not found on the Linux system. The Plug-UI is not included in the build..." )
    }
    mac:exists( /usr/local/lib/libMyGUIEngine* ):exists( /usr/local/lib/libMyGUI.OpenGLPlatform.* ){
        message( "Configuring system to build with MyGUI ..." )
        SUBDIRS += Plugin-UI
    }else{
        mac:message( "MyGUI not found on the Mac system. The Plug-UI is not included in the build..." )
    }

##CIGI
    exists( /usr/local/lib64/libcigicl* ){
        message( "Configuring system to build with CIGI Class Libraries..." )
        SUBDIRS += Plugin-CIGI
    }else{
        message( "The CIGI class library was not found on the system. The CIGI plugin is not included in the build..." )
    }
    mac:exists( /usr/local/lib/libcigicl* ){
        mac:message( "Configuring system to build with CIGI Class Libraries..." )
        SUBDIRS += Plugin-CIGI
    }else{
        mac:message( "The CIGI class library was not found on the system. The CIGI plugin is not included in the build..." )
    }

##mersive
    !mac:exists( /mersive/lib/libMersive* ) {
         message( "Configuring system to build with Mersive ..." )
         SUBDIRS += Plugin-Mersive
    }else{
         mac:message( "Mersive not found on the Mac system.  The Mersive Plugin is not included in the build..." )
    }

##JRM Sensors
#    JRMPATH = $$(JRM_OSV_PATH)
#    isEmpty(JRMPATH) {
#         message( "JRM Sensors not found on system, The JRM Sensor plugin is not included in the build ..." )
#    }else{
#         message( "Configuring system to build with JRM Sensors..." )
#         SUBDIRS += Plugin-JRMSensors
#    }
}

win32 {
    HAS_BULLET64=FALSE
#SILVERLINING
    SLROOT = $$(SILVERLINING)
    isEmpty(SLROOT) {
        message( "SilverLining not found on the system. The SilverLining plugin is not included in the build..." )
    }
    else {
        message( "Configuring Windows system to build with SilverLining..." )
        SUBDIRS += Plugin-SilverLining
    }

#TRITON
    TROOT = $$(TRITON)
    isEmpty(TROOT) {
        message( "Triton not found on the system. The Triton plugin is not included in the build..." )
    }
    else {
        message( "Configuring Windows system to build with Triton..." )
        SUBDIRS += Plugin-Triton
    }

#BULLET
    exists( C:/Program Files/BULLET_PHYSICS ): exists( C:/Program Files/osgBullet ) {
        message( "Configuring Windows system 64bit to build with Bullets ..." )
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

#OSGEARTH
    exists( C:/Program Files/OSGEARTH ) {
        OSGEARTH_ROOT = C:\Program Files\OSGEARTH
    }
    OSGEARTHROOT = $$(OSGEARTH_ROOT)
    isEmpty(OSGEARTHROOT) {
        message( "osgEarth not found on the system. The application is not included in the build..." )
    }
    else {
        message( "Configuring Windows system to build with osgEarth ..." )
        SUBDIRS += Plugin-OSGEarthSimpleLighting
        SUBDIRS += Application-Earth
    }

#CSTSHARED
    CSTSHAREROOT = $$(CSTSHARE)
    isEmpty(CSTSHAREROOT) {
        message( "We are NOT Configuring for building with Legacy Muse products. required Muse libraries were not found!!!..." )
    }
    else {
        message( "Configuring Windows system to build with Legacy Muse products..." )
        SUBDIRS += Plugin-Muse
    }

#MYGUI
    MYGUIROOT = $$(MYGUI_ROOT)
    isEmpty(MYGUIROOT) {
        message( "MyGUI not found on the Windows system. The Plug-UI is not included in the build..." )
    }
    else {
        message( "Configuring Windows system to build with MyGUI ..." )
        SUBDIRS += Plugin-UI
    }

#CIGI
    CIGIROOT = $$(CIGI_ROOT)
    isEmpty(CIGIROOT) {
        message( "The CIGI Class libraries were not found on the Windows system. The CIGI plugin is not included in the build..." )
    }
    else {
        message( "The CIGI Class libraries were founnd, configuring Windows system to build with the CIGI class libraries ..." )
        SUBDIRS += Plugin-CIGI
    }
}

#SUBDIRS += Simulation

# library version number files
exists( "openig_version.pri" ){

include( "openig_version.pri" )
isEmpty( VERSION ){ error( "bad or undefined VERSION variable inside file openig_version.pri" ) }

}
else { error( "could not find pri library version file openig_version.pri" ) }

# end of library version number files
