CONFIG  += ordered release silent
TEMPLATE = subdirs
SUBDIRS +=  OpenIG-Host \
            OpenIG-TerrainQueryServer \
            OpenIG-ImageGenerator \
            OpenIG-Plugin

OTHER_FILES +=  CMakeModules/*.* \
                CMakeLists.txt
	       
