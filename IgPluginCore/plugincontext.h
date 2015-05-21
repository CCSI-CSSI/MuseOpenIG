//#******************************************************************************
//#*
//#*      Copyright (C) 2015  Compro Computer Services
//#*      http://openig.compro.net
//#*
//#*      Source available at: https://github.com/CCSI-CSSI/MuseOpenIG
//#*
//#*      This software is released under the LGPL.
//#*
//#*   This software is free software; you can redistribute it and/or modify
//#*   it under the terms of the GNU Lesser General Public License as published
//#*   by the Free Software Foundation; either version 2.1 of the License, or
//#*   (at your option) any later version.
//#*
//#*   This software is distributed in the hope that it will be useful,
//#*   but WITHOUT ANY WARRANTY; without even the implied warranty of
//#*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
//#*   the GNU Lesser General Public License for more details.
//#*
//#*   You should have received a copy of the GNU Lesser General Public License
//#*   along with this library; if not, write to the Free Software
//#*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//#*
//#*****************************************************************************
#ifndef PLUGINCONTEXT_H
#define PLUGINCONTEXT_H

#include <IgPluginCore/export.h>
#include <IgCore/imagegenerator.h>

#include <IgCore/igcore.h>

#include <map>

namespace igplugincore
{

/*! This class is represnting a context that is passed from the ImageGenerator to the plugin
 * It has reference of the igcore::ImageGenerator and \ref igplugincore::PluginContext::Attribute
 * \brief The PluginContext class
 * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
 * \copyright (c)Compro Computer Services, Inc.
 * \date      Fri Jan 16 2015
 */
class IGPLUGINCORE_EXPORT PluginContext
{
public:
    /*!
     * \brief Constructor
     * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
     * \copyright (c)Compro Computer Services, Inc.
     * \date      Fri Jan 16 2015
     */
    PluginContext()
        : _ig(0) {}

    /*!
     * \brief Constructor with reference to \ref igcore::ImageGenerator
     * \param the reference to \ref igcore::ImageGenerator
     * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
     * \copyright (c)Compro Computer Services, Inc.
     * \date      Fri Jan 16 2015
     */
    PluginContext(igcore::ImageGenerator* ig)
        : _ig(ig) {}\

    /*!
     * \brief Sets the \ref igcore::ImageGenerator reference
     * \param ig    The \ref igcore::ImageGenerator reference
     * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
     * \copyright (c)Compro Computer Services, Inc.
     * \date      Fri Jan 16 2015
     */
    void setImageGenerator(igcore::ImageGenerator* ig)
    {
        _ig = ig;
    }

    /*!
     * \brief Returns the \ref igcore::ImageGenerator reference
     * \return The \ref igcore::ImageGenerator reference
     * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
     * \copyright (c)Compro Computer Services, Inc.
     * \date      Fri Jan 16 2015
     */
    igcore::ImageGenerator* getImageGenerator() const
    {
        return _ig;
    }


    /*! This struct gives the user chance to add custom structs in the
     * \ref igplugincore::PluginContext to be passed from the \ref igcore::ImageGenerator
     * to all the plugins. The \ref openig::OpenIG Image Generator implementation is
     * keeping a reference of a \ref igplugincore::PluginContext and it cleans it after
     * each frame
     * \brief Attribute struct
     * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
     * \copyright (c)Compro Computer Services, Inc.
     * \date      Fri Jan 16 2015
     */

    template<typename T>
    struct Attribute : public osg::Referenced
    {
        /*!
         * \brief Attribute
         * \param A custom value, struct
         * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
         * \copyright (c)Compro Computer Services, Inc.
         * \date      Fri Jan 16 2015
         */
        Attribute(T value)
            : _value(value) {}

        /*!
         * \brief Set some value
         * \param The value
         * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
         * \copyright (c)Compro Computer Services, Inc.
         * \date      Fri Jan 16 2015
         */
        void setValue(const T& value)
        {
            _value = value;
        }\

        /*!
         * \brief Gets the value
         * \return The value
         * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
         * \copyright (c)Compro Computer Services, Inc.
         * \date      Fri Jan 16 2015
         */
        T& getValue()
        {
            return _value;
        }

        T   _value;
    };

    typedef std::multimap< std::string, osg::ref_ptr<osg::Referenced> >                    AttributeMap;
    typedef std::multimap< std::string, osg::ref_ptr<osg::Referenced> >::iterator          AttributeMapIterator;
    typedef std::multimap< std::string, osg::ref_ptr<osg::Referenced> >::const_iterator    AttributeMapConstIterator;

    /*!
     * \brief Gets a handle of all the \ref igplugincore::PluginContext::Attribute present
     * in the context
     * \return
     * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
     * \copyright (c)Compro Computer Services, Inc.
     * \date      Fri Jan 16 2015
     */
    AttributeMap&   getAttributes()
    {
        return _attributes;
    }

    /*!
     * \brief Adds an \ref igplugincore::PluginContext::Attribute with a given name
     * \param The name of the \ref igplugincore::PluginContext::Attribute
     * \param The custom \ref igplugincore::PluginContext::Attribute
     * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
     * \copyright (c)Compro Computer Services, Inc.
     * \date      Fri Jan 16 2015
     */
    void addAttribute( const std::string& name, osg::Referenced* attr )
    {
        if (attr == 0) return;

        _attributes.insert(AttributeMap::value_type(name,attr));
    }

    /*!
     * \brief Gets an \ref igplugincore::PluginContext::Attribute by its name
     * \param The name of the \ref igplugincore::PluginContext::Attribute
     * \return 0 on failure, the \ref igplugincore::PluginContext::Attribute if found
     * \author    Trajce Nikolov Nick trajce.nikolov.nick@gmail.com
     * \copyright (c)Compro Computer Services, Inc.
     * \date      Fri Jan 16 2015
     */
    osg::Referenced* getAttribute(const std::string& name)
    {
        AttributeMapIterator itr = _attributes.find(name);
        if (itr != _attributes.end())
            return itr->second.get();
        else
            return 0;
    }

protected:
    igcore::ImageGenerator* _ig;            /*! \brief a reference to ImageGenerator */
    AttributeMap            _attributes;    /*! \brief attrinute name based map */
};

} // namespace

#endif // PLUGINCONTEXT_H
