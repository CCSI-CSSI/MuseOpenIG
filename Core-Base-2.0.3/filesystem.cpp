#include "filesystem.h"
#include "stringutils.h"

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/thread.hpp>

#include <osg/Notify>

#include <osgDB/FileNameUtils>

#include <sstream>
#include <fstream>
#include <limits.h>
#if ( __WORDSIZE == 64 )
#define BUILD_64   1
#endif

using namespace OpenIG::Base;

bool FileSystem::fileExists(const std::string& strFileName)
{
    boost::filesystem::path path(strFileName);
    return boost::filesystem::exists(path);
}
std::string FileSystem::fileFullPath(const std::string& strFileName)
{
    if (fileExists(strFileName)==false)
        return "";

    boost::filesystem::path path(strFileName);
    return boost::filesystem::complete(path).string();
}

std::string FileSystem::readFileIntoString(const std::string& strFileName)
{
    if (fileExists(strFileName)==false)
        return "";

    std::ifstream iff;
    iff.open(fileFullPath(strFileName).c_str(), std::ifstream::in);

    if (iff.good()==false)
        return "";

    std::string source;
    std::string temp;
    while(std::getline(iff, temp)) 
    {
        source += temp;
        source += "\n";
    }

    iff.close();
    return source;
}

std::string FileSystem::path(FileSystem::PathType type, const std::string& path)
{
	switch (type)
	{
	case Plugins:
	{
		// CR ----------------------------------------------------------------------------------
#if defined (__linux) || defined (__APPLE__)
		char * oig_root = NULL;
		std::string igplugin_path;

		oig_root = getenv("OPENIG_LIBRARY_PATH");

		if (!oig_root)
		{
			osg::notify(osg::NOTICE) << "ImageGenerator Core: Env variable OPENIG_LIBRARY_PATH doesn't exist!!!" << std::endl;

#if !defined(BUILD_64) || defined (__APPLE__)
			igplugin_path = "/usr/local/lib/plugins";
#else
			igplugin_path = "/usr/local/lib64/plugins";
#endif
		}
		else
		{
			igplugin_path = oig_root;
			igplugin_path += "/plugins";
		}

		return igplugin_path+"/"+path;

#elif   defined (_WIN32)
		char* c_oig_root;
		std::string oig_root;
		size_t requiredSize;
		bool var_present = false;

		//Use win32 safe version of getenv()
		getenv_s(&requiredSize, NULL, 0, "OPENIG_LIBRARY_PATH");
		if (requiredSize == 0)
		{
			osg::notify(osg::NOTICE) << "ImageGenerator Core: Env variable OPENIG_LIBRARY_PATH doesn't exist!!!, setting requiredsize to 14 for default path!!!" << std::endl;
			requiredSize = 14;
		}
		else
			var_present = true;

		c_oig_root = (char*)malloc(requiredSize * sizeof(char));
		if (!c_oig_root)
		{
			osg::notify(osg::NOTICE) << "ImageGenerator Core: Failed to allocate memory for OPENIG_LIBRARY_PATH variable!!, exiting!!!" << std::endl;
			exit(1);
		}

		if (var_present)
		{
			// Get the value of the LIB environment variable.
			getenv_s(&requiredSize, c_oig_root, requiredSize, "OPENIG_LIBRARY_PATH");
			oig_root = c_oig_root;
			oig_root += "\\plugins";
		}
		else //Fallback path to try in event of no env var setting.
			oig_root = "..\\plugins";

		free(c_oig_root);

		return oig_root + "/" + path;
#endif
		// CR ----------------------------------------------------------------------------------
	}
	break;
	case Resources:
	{
#if defined (__linux) || defined (__APPLE__)
		char * oig_root = NULL;
		std::string igres_path;

		oig_root = getenv("OPENIG_RESOURCE_PATH");

		if (!oig_root)
		{
			osg::notify(osg::NOTICE) << "ImageGenerator Core: Env variable OPENIG_RESOURCE_PATH doesn't exist!!!" << std::endl;

			igres_path = "/usr/local/openig/resources";
		}
		else
		{
			igres_path = oig_root;
		}

		return igres_path.empty() ? path : igres_path;

#elif   defined (_WIN32)
		char* c_oig_root;
		std::string oig_root;
		size_t requiredSize;
		bool var_present = false;

		//Use win32 safe version of getenv()
		getenv_s(&requiredSize, NULL, 0, "OPENIG_RESOURCE_PATH");
		if (requiredSize == 0)
		{
			osg::notify(osg::NOTICE) << "ImageGenerator Core: Env variable OPENIG_RESOURCE_PATH doesn't exist!!!, setting requiredsize to 14 for default path!!!" << std::endl;
			requiredSize = 14;
		}
		else
			var_present = true;

		c_oig_root = (char*)malloc(requiredSize * sizeof(char));
		if (!c_oig_root)
		{
			osg::notify(osg::NOTICE) << "ImageGenerator Core: Failed to allocate memory for OPENIG_RESOURCE_PATH variable!!, exiting!!!" << std::endl;
			exit(1);
		}

		if (var_present)
		{
			// Get the value of the LIB environment variable.
			getenv_s(&requiredSize, c_oig_root, requiredSize, "OPENIG_RESOURCE_PATH");
			oig_root = c_oig_root;
		}
		else //Fallback path to try in event of no env var setting.
			oig_root = path;

		free(c_oig_root);

		return oig_root;
#endif
	}
	break;
	case Data:
	{
#if defined (__linux) || defined (__APPLE__)
		char * oig_root = NULL;
		std::string igdata_path;

		oig_root = getenv("OPENIG_DATA_PATH");

		if (!oig_root)
		{
			osg::notify(osg::NOTICE) << "ImageGenerator Core: Env variable OPENIG_DATA_PATH doesn't exist!!!" << std::endl;

			igdata_path = "/usr/local/openig/database";
		}
		else
		{
			igdata_path = oig_root;
		}

		return igdata_path.empty() ? path : igdata_path;

#elif   defined (_WIN32)
		char* c_oig_root;
		std::string oig_root;
		size_t requiredSize;
		bool var_present = false;

		//Use win32 safe version of getenv()
		getenv_s(&requiredSize, NULL, 0, "OPENIG_DATA_PATH");
		if (requiredSize == 0)
		{
			osg::notify(osg::NOTICE) << "ImageGenerator Core: Env variable OPENIG_DATA_PATH doesn't exist!!!, setting requiredsize to 14 for default path!!!" << std::endl;
			requiredSize = 14;
		}
		else
			var_present = true;

		c_oig_root = (char*)malloc(requiredSize * sizeof(char));
		if (!c_oig_root)
		{
			osg::notify(osg::NOTICE) << "ImageGenerator Core: Failed to allocate memory for OPENIG_DATA_PATH variable!!, exiting!!!" << std::endl;
			exit(1);
		}

		if (var_present)
		{
			// Get the value of the LIB environment variable.
			getenv_s(&requiredSize, c_oig_root, requiredSize, "OPENIG_DATA_PATH");
			oig_root = c_oig_root;
		}
		else //Fallback path to try in event of no env var setting.
			oig_root = path;

		free(c_oig_root);

		return oig_root;
#endif
	}
	break;
	case PathList:
	{
		if (path.size() > 2)
		{
			StringUtils::Tokens tokens = StringUtils::instance()->tokenize(path, "{}");
			if (tokens.size())
			{
				std::string paths = tokens.at(0);
				tokens.erase(tokens.begin());

				if (tokens.size())
				{
					osg::notify(osg::NOTICE) << "ImageGenerator Core: FileSystem: PathList: " << paths << std::endl;

					std::string file = tokens.at(0);
					tokens = StringUtils::instance()->tokenize(paths, ";");

					StringUtils::TokensIterator itr = tokens.begin();
					for (; itr != tokens.end(); ++itr)
					{
						std::string onePath = *itr;

						osg::notify(osg::NOTICE) << "\tChecking: " << onePath << std::endl;

						OpenIG::Base::StringUtils::Tokens singlePathTokens = StringUtils::instance()->tokenize(onePath, "/");
						OpenIG::Base::StringUtils::TokensIterator sp_itr = singlePathTokens.begin();

						std::ostringstream completePath;
						if (onePath.at(0) == '/' || onePath.at(0) == '\\')
							completePath << "/";

						for (; sp_itr != singlePathTokens.end(); ++sp_itr)
						{
							std::string& token = *sp_itr;
							if (token.size() && token.at(0) == '$')
							{
								token = StringUtils::instance()->env(token);
							}
							completePath << token << "/";
						}
						onePath = completePath.str();

						osg::notify(osg::NOTICE) << "\t\tChecking: " << onePath << std::endl;

						if (!fileExists(onePath + "/" + file))
						{
							StringUtils::Tokens ftokens = StringUtils::instance()->tokenize(file, ".");
							StringUtils::TokensIterator fitr = ftokens.begin();

							std::string partialFileName = *fitr;
							do
							{
								if (fileExists(onePath + "/" + partialFileName))
								{
									osg::notify(osg::NOTICE) << "\tFound valid file: " << (onePath + "/" + partialFileName) << std::endl;
									osg::notify(osg::NOTICE) << "\tFound valid path: " << (onePath + "/" + file) << std::endl;
									return onePath + "/" + file;
								}
								++fitr;
								if (fitr != ftokens.end())
									partialFileName += "." + *fitr;
							} while (fitr != ftokens.end());
						}
						else
						{
							osg::notify(osg::NOTICE) << "\tFound valid path: " << (onePath + "/" + file) << std::endl;
							return onePath + "/" + file;
						}
					}
				}
			}
		}
	}
	break;
	}

	return path;
}

bool FileSystem::match(const OpenIG::Base::StringUtils::StringList& patterns, const std::string& simpleFileName)
{
	OpenIG::Base::StringUtils::StringList::const_iterator itr = patterns.begin();
	for (; itr != patterns.end(); ++itr)
	{
		std::string targetPath = osgDB::getFilePath(*itr);
		std::string pattern = osgDB::getSimpleFileName(*itr);

		if (targetPath.empty()) targetPath = "./";

		boost::filesystem::directory_iterator end_itr; 
		for (boost::filesystem::directory_iterator i(targetPath); i != end_itr; ++i)
		{
			if (!boost::filesystem::is_regular_file(i->status())) continue;

			const std::string filename = i->path().filename().string();

			boost::smatch what;
			boost::regex expression;
			try
			{
				expression = boost::regex(pattern);
				if (!boost::regex_match(filename, what, expression)) continue;
			}
			catch (boost::regex_error& e)
			{
				osg::notify(osg::NOTICE) << "ImageGenerator Core: " << pattern << "is not a valid regular expression: \""<< e.what() << "\"" << std::endl;
				continue;
			}

			if (filename == simpleFileName) return true;
		}
	}

	return false;
}

static boost::mutex lastWriteMutex;
time_t FileSystem::lastWriteTime(const std::string& fileName)
{
	time_t result;
	lastWriteMutex.lock();
	result = boost::filesystem::last_write_time(fileName.c_str());
	lastWriteMutex.unlock();

	return result;
}