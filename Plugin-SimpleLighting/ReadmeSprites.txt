
There is currently a bug in OpenIG that does not allow the spirite textures
to properly show when the Plugin-SimpleLighting plugin is used instead of
the ForwardPlusLighting plugin.

As a temporary work around you can make a one line change to one of
our shader files: Resources/shaders/sprite_bb_gs.glsl

Simple comment out line 117...

    if (actualSize.y<minPixelSize||actualSize.y<minPixelSize)
    {
-->     //adjustSize = true;
    }

This will allow the sprite texture to be visible again for use with the SimpleLighting Plugin.
Should you decide to go back to the ForwardPlusLighting plugin, please ensure you uncomment
this line again for proper use with the F+ plugin.
