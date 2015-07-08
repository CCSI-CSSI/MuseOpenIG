
// Copyright (c) 2008-2012 Sundog Software, LLC. All rights reserved worldwide

#include "CloudsDrawable.h"
#include <SilverLining.h>
#include "AtmosphereReference.h"

#include <osg/Material>
#include <osg/GL2Extensions>

#include <assert.h>

#include <IgCore/attributes.h>
#include <osg/ValueObject>

using namespace SilverLining;
using namespace igplugins;

CloudsDrawable::CloudsDrawable()
        : osg::Drawable()
        , _envMapDirty(false)
        , _pluginContext(0)
{
    initialize();
}

CloudsDrawable::CloudsDrawable(osgViewer::View* view, igcore::ImageGenerator* ig)
        : osg::Drawable()
        , _view(view)
        , _ig(ig)
        , _envMapDirty(false)
        , _pluginContext(0)
{
    initialize();
}

// Set up our attributes and callbacks.
void CloudsDrawable::initialize()
{
	setDataVariance(osg::Object::DYNAMIC);
    setUseVertexBufferObjects(false);
    setUseDisplayList(false);

    if (_view) {
        SilverLiningCloudsUpdateCallback *updateCallback = new SilverLiningCloudsUpdateCallback;
        setUpdateCallback(updateCallback);

        SilverLiningCloudsComputeBoundingBoxCallback *boundsCallback = new SilverLiningCloudsComputeBoundingBoxCallback;
        boundsCallback->camera = _view->getCamera();
        setComputeBoundingBoxCallback(boundsCallback);
    }
}

// Draw the clouds.
void CloudsDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
	AtmosphereReference *ar = dynamic_cast<AtmosphereReference *>(renderInfo.getCurrentCamera()->getUserData());
	SilverLining::Atmosphere *atmosphere = 0;
	if (ar) atmosphere = ar->atmosphere;

	renderInfo.getState()->disableAllVertexArrays();

    if (atmosphere)
    {		
        osg::GL2Extensions* ext = osg::GL2Extensions::Get(renderInfo.getContextID(),true);
		if (ext)
        {
			{
				GLint shader = (GLint)atmosphere->GetBillboardShader();

				ext->glUseProgram(shader);
				GLint loc = ext->glGetUniformLocation(shader, "lightsEnabled");
				if (loc != -1)
				{
                    std::vector<GLint> lightsEnabled;
                    for (size_t i=0; i<8; ++i)
                    {
                        lightsEnabled.push_back(_ig->isLightEnabled(i) ? 1 : 0);
                    }
                    ext->glUniform1iv(loc, lightsEnabled.size(), &lightsEnabled.front());

				}
				GLint cbloc = ext->glGetUniformLocation(shader, "lightsBrightness");
				if (cbloc != -1)
				{					
					std::vector<GLfloat> lightsBrightness;
					for (size_t i = 0; i<8; ++i)
					{
						igcore::LightAttributes attr = _ig->getLightAttributes(i);
						lightsBrightness.push_back(attr._cloudBrightness);

						//std::cout << "Cloud Light Attrib: " << i << ", " << attr._cloudBrightness << std::endl;
					}
					//std::cout << std::endl;
					ext->glUniform1fv(cbloc, lightsBrightness.size(), &lightsBrightness.front());
				}
				GLint lightingBrightnessLoc = ext->glGetUniformLocation(shader, "todLightsBrightness");
				if (lightingBrightnessLoc != -1)
				{					
					float factor = _todHour > 4 && _todHour < 18 ? _lightBrightness_day : _lightBrightness_night;
					//std::cout << "todLightsBrightness = " << factor << std::endl;
					ext->glUniform1f(lightingBrightnessLoc, factor);
				}
				GLint lightingBrightnessEnabledLoc = ext->glGetUniformLocation(shader, "todLightsBrightnessEnabled");
				if (lightingBrightnessEnabledLoc != -1)
				{	
					//std::cout << "todLightsBrightnessEnabled = " << _lightBrightness_enable << std::endl;
					ext->glUniform1i(lightingBrightnessEnabledLoc, _lightBrightness_enable ? 1 : 0);
				}
				ext->glUseProgram(0);
			}
			{
				SL_VECTOR(unsigned int) shaders = atmosphere->GetActivePlanarCloudShaders();
				for (SL_VECTOR(unsigned int)::iterator itr = shaders.begin(); itr != shaders.end(); ++itr)
				{
					GLint shader = (GLint)*itr;

					ext->glUseProgram(shader);
                    GLint loc = ext->glGetUniformLocation(shader, "lightsEnabled[0]");
					if (loc != -1)
					{
                        std::vector<GLint> lightsEnabled;
                        for (size_t i=0; i<8; ++i)
                        {
                            lightsEnabled.push_back(_ig->isLightEnabled(i) ? 1 : 0);
                        }
                        ext->glUniform1iv(loc, lightsEnabled.size(), &lightsEnabled.front());
					}
					ext->glUseProgram(0);
				}
			}
            
			 
			atmosphere->SetCameraMatrix(renderInfo.getCurrentCamera()->getViewMatrix().ptr());
			atmosphere->SetProjectionMatrix(renderInfo.getCurrentCamera()->getProjectionMatrix().ptr());

            atmosphere->DrawObjects(true, true, true);

            if (false)//(_envMapDirty)
            {
                CloudsDrawable *mutableThis = const_cast<CloudsDrawable*>(this);
                void*           envPtr = 0;

                if (atmosphere->GetEnvironmentMap(envPtr,6,false,0,false))
                {

                    mutableThis->_envMapID = (GLint)(long)envPtr;

                    mutableThis->_envMapDirty = false;

                    if (_pluginContext)
                    {
                        igcore::EnvironmentalMapAttributes attr;
                        attr._envMapId = mutableThis->_envMapID;

                        mutableThis->_pluginContext->addAttribute("EnvironmentMap", new igplugincore::PluginContext::Attribute<igcore::EnvironmentalMapAttributes>(attr));
                    }
                }
            }
		}
    }

    renderInfo.getState()->dirtyAllVertexArrays();
}

// We use the Atmosphere::GetCloudBounds() to get the bounds of the clouds in the scene. This is updated
// and persisted after the update pass, so it fits well with OSG's threading model.
osg::BoundingBox SilverLiningCloudsComputeBoundingBoxCallback::computeBound(const osg::Drawable&) const
{
    osg::BoundingBox box;
    
    if (camera) 
    {
        AtmosphereReference *ar = dynamic_cast<AtmosphereReference *>( camera->getUserData() );
        if ( ar && ar->atmosphere)
        {
            SilverLining::Atmosphere * atmosphere = ar->atmosphere;            

            double minX, minY, minZ, maxX, maxY, maxZ;
            atmosphere->GetCloudBounds(minX, minY, minZ, maxX, maxY, maxZ);
            osg::Vec3d min(minX, minY, minZ);
            osg::Vec3d max(maxX, maxY, maxZ);
            box = osg::BoundingBox(min, max);
        }
    }

    return box;
}

// Recompute the bounds of the clouds every frame.
void SilverLiningCloudsUpdateCallback::update(osg::NodeVisitor*, osg::Drawable* drawable)
{
    if (drawable) {
        drawable->dirtyBound();
    }
}
