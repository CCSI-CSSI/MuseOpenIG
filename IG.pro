CONFIG  += ordered debug silent
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
    Demo

unix {
    exists( /usr/local/lib/libSilverLining* ){
    message( "Configuring for build with SilverLining..." )
    SUBDIRS += IgPlugin-SilverLining
    }else{
    message( "SilverLining not found on the system. The plugin is not included in the buiild..." )
    }

    exists( /usr/local/lib/libCstShare* ){
    message( "Configuring for build with CstShare..." )
    SUBDIRS += IgPlugin-SLScene
    }else{
    message( "CstShare not found on the system. The plugin is not included in the buiild..." )
    }
}

win32 {
    SLROOT = $$(SILVERLINING)
    message( "Configuring for build with SilverLining..." )
    isEmpty(SLROOT) {
        message( "SilverLining not found on the system. The plugin is not included in the buiild..." )
    }
    else {
        SUBDIRS += IgPlugin-SilverLining
    }

    CSTSHAREROOT = $$(CSTSHARE)
    message( "Configuring for build with CstShare..." )
    isEmpty(CSTSHAREROOT) {
        message( "CstShare not found on the system. The plugin is not included in the buiild..." )
    }
    else {
        SUBDIRS += IgPlugin-SLScene
    }
}
