#include "texturecache.h"
#include "filesystem.h"

#include <osgDB/ReadFile>

#include <boost/unordered_map.hpp>

namespace osg
{
   // These are search paths for the images
   void TextureCache::addPath(const std::string& path)
   {
      bool not_found = true;

      // Dont add it multiple times
      Paths::iterator itr = _paths.begin();
      for (; itr != _paths.end(); ++itr)
      {
         if (path == *itr)
         {
            not_found = false;
            break;
         }
      }
      if (not_found) _paths.push_back(path);
   }

   // Gets a texture from the cache
   Texture2DPointer TextureCache::get(const std::string& name)
   {
      // If existing simply return
      MapNamesToTexture2DPointers::iterator itr = _cache.find(name);
      if (itr != _cache.end()) return itr->second;

      // read the image from the given path
      osg::ref_ptr<osg::Image> image = osgDB::readImageFile(name);
      if (!image.valid())
      {
         // If failed, look into the search paths
         Paths::iterator itr = _paths.begin();
         for (; itr != _paths.end(); ++itr)
         {
            const std::string path = *itr + "/" + name;
            image = osgDB::readImageFile(path);
            if (image.valid()) break;
         }
      }

      if (!image.valid())
      {
         osg::notify(osg::NOTICE) << "LightsControl: Texture cache: failed to load texture: " << name << std::endl;
         return Texture2DPointer();
      }

      osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
      texture->setImage(image);
      texture->setUnRefImageDataAfterApply(true);

      _cache[name] = texture;
      return texture;
   }

   // Gets unique ID from the
   // 6 names of the cubemap
   static unsigned getTextureID(const OpenIG::Base::StringUtils::StringList& files)
   {
	   if (files.size() != 6) return 0;

	   std::size_t seed = 0;
	   boost::hash_combine(seed, files.at(0));
	   boost::hash_combine(seed, files.at(1));
	   boost::hash_combine(seed, files.at(2));
	   boost::hash_combine(seed, files.at(3));
	   boost::hash_combine(seed, files.at(4));
	   boost::hash_combine(seed, files.at(5));
	   return seed;
   }


   void TextureCubeMapCache::addPath(const std::string& path)
   {
	   bool not_found = true;

	   // Dont add it multiple times
	   Paths::iterator itr = _paths.begin();
	   for (; itr != _paths.end(); ++itr)
	   {
		   if (path == *itr)
		   {
			   not_found = false;
			   break;
		   }
	   }
	   if (not_found) _paths.push_back(path);
   }

   TextureCubeMapPointer TextureCubeMapCache::get(const FileNames& fileNames)
   {
	   // Get an id for this file list
	   unsigned ID = getTextureID(fileNames);
	   if (ID == 0) return TextureCubeMapPointer();

	   // If existing simply return
	   MapNamesToTextureCubeMapPointers::iterator itr = _cache.find(ID);
	   if (itr != _cache.end()) return itr->second;

	   // We try to load all 6 images in here
	   typedef std::map<osg::TextureCubeMap::Face, osg::ref_ptr<osg::Image> >		FaceImages;
	   FaceImages	images;

	   // load the images, all 6 of them, if failed, return NULL
	   OpenIG::Base::StringUtils::StringList::const_iterator sitr = fileNames.begin();
	   // counter for the image
	   osg::TextureCubeMap::Face face = (osg::TextureCubeMap::Face)0;

	   // load the images here
	   for (; sitr != fileNames.end(); ++sitr)
	   {
		   // the file name
		   const std::string& name = *sitr;

		   // read the image from the given path
		   osg::ref_ptr<osg::Image> image = osgDB::readImageFile(name);
		   if (!image.valid())
		   {
			   // If failed, look into the search paths
			   Paths::iterator itr = _paths.begin();
			   for (; itr != _paths.end(); ++itr)
			   {
				   const std::string path = *itr + "/" + name;
				   image = osgDB::readImageFile(path);
				   if (image.valid()) break;
			   }
		   }
		   if (!image.valid())
		   {
			   osg::notify(osg::NOTICE) << "LightsControl: Texture cache: failed to load texture: " << name << std::endl;
			   return TextureCubeMapPointer();
		   }

		   // valid Image, save it
		   images[face] = image;

		   face = (osg::TextureCubeMap::Face)((int)face + 1);
	   }

	   if (images.size() != 6) return TextureCubeMapPointer();

	   osg::ref_ptr<osg::TextureCubeMap> texture = new osg::TextureCubeMap;
	   for (size_t i = 0; i < images.size(); ++i)
	   {
		   osg::ref_ptr<osg::Image>& image = images[(osg::TextureCubeMap::Face)i];
		   texture->setImage((osg::TextureCubeMap::Face)i, image);
	   }
	   texture->setUnRefImageDataAfterApply(true);

	   _cache[ID] = texture;
	   return texture;
   }
}