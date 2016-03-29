#include <cstdlib>
#include <string.h>
#include <IniFile.h>


#define	RC_FILE_NAME_DEFAULT "/etc/mersiveListener.rc"
#define RC_FILE_ENV_VARIABLE "MERSIVE_RC_FILE"


IniFile::IniFile(void)
{
    mRcFileName = NULL;

    this->fileLoaded=false;
    this->iniMapStringRcKey();

    char *lpRcFile = getenv( RC_FILE_ENV_VARIABLE );
    mRcFileName = new char[1024];

    if( lpRcFile != NULL &&  mRcFileName != NULL) {
        strncpy(this->mRcFileName, lpRcFile, 1024);
    }
    else if(mRcFileName != NULL)
        strncpy(this->mRcFileName, RC_FILE_NAME_DEFAULT, 1024);
    else
    {
        std::cerr << "IniFile::IniFile Unable to allocate char[1024], exiting" << std::endl;
        exit(-1);
    }

}

IniFile::~IniFile(void)
{
    if(mRcFileName != NULL)
        delete mRcFileName;
}

void	IniFile::loadFile(){
    this->readValuesFromRcFile();
    this->fileLoaded = true;
}

//Member Functions -----------------------------------------
void IniFile::readValuesFromRcFile(){
    string line;
    ifstream rcFile(this->mRcFileName);
    if(rcFile.is_open()){
        while(rcFile.good()){
            getline(rcFile, line);
            if(line.length() > 1 && line.substr(0,1) != "#")
            {
                this->parseRcLine(&line);
            }
        }
    }else{
        cerr << "Unable to open rc file, quitting." << endl;
        exit(0);
    }
}


char* IniFile::getRcFileName(){
    return this->mRcFileName;
}

void  IniFile::setRcFileName(char* aName){
    this->mRcFileName = aName;
}

void IniFile::parseRcLine(string* aLine){
    string key, value;
    int indEquals, indEnd;

    indEquals = aLine->find("=", 0);
    indEnd = aLine->find(";",0);
    if(indEnd > 0 && indEnd < indEquals)
        return;

    if(indEnd == -1)
        indEnd = aLine->length();

    key = aLine->substr(0, indEquals);
    value = aLine->substr(indEquals+1, indEnd-indEquals);
    char* str = (char*)value.c_str();

    //assign the rcKey to the map (only strings assigned, any conversions must take place later)
        //make sure the key exists
        map<string, mRcKey>::iterator iter = mapStringRcKey.find(key);
        if (iter != mapStringRcKey.end()) {
          this->mapRcKeyValues[mapStringRcKey[key]] = str ;
        }

}


void 	IniFile::iniMapStringRcKey(){
    mapStringRcKey["CALIBRATION_CLIENT_BIN"] = CALIBRATION_CLIENT_BIN;
    mapStringRcKey["CALIBRATION_SERVER_IP"] = CALIBRATION_SERVER_IP;
    mapStringRcKey["COLOR_TUNER_CLIENT_BIN"] = COLOR_TUNER_CLIENT_BIN;
    mapStringRcKey["COLOR_TUNER_CLIENT_BIN_V2"] = COLOR_TUNER_CLIENT_BIN_V2;
    mapStringRcKey["COLOR_TUNER_CLIENT_BIN_V3"] = COLOR_TUNER_CLIENT_BIN_V3;
    mapStringRcKey["GAMMA_SERVER_IP"] = GAMMA_SERVER_IP;
    mapStringRcKey["COLOR_TUNER_SERVER_BIN"] = COLOR_TUNER_SERVER_BIN;
    mapStringRcKey["COLOR_TUNER_SERVER_BIN_V2"] = COLOR_TUNER_SERVER_BIN_V2;
    mapStringRcKey["COLOR_TUNER_SERVER_BIN_V3"] = COLOR_TUNER_SERVER_BIN_V3;
    mapStringRcKey["CUSTOM_APP_BIN"] = CUSTOM_APP_BIN;
    mapStringRcKey["WARPER_SERVER_PORT"] = WARPER_SERVER_PORT;
    mapStringRcKey["GAMMA_SERVER_PORT"] = GAMMA_SERVER_PORT;
        mapStringRcKey["CALIBRATION_SERVER_PORT"] = CALIBRATION_SERVER_PORT;
        mapStringRcKey["MASK_SERVER_PORT"] = MASK_SERVER_PORT;
        mapStringRcKey["MASK_SERVER_IP"] = MASK_SERVER_IP;
        mapStringRcKey["MASK_SERVER_BIN"] = MASK_SERVER_BIN;
        mapStringRcKey["MASK_CLIENT_BIN"] = MASK_CLIENT_BIN;

        mapStringRcKey["POST_EDIT_SERVER_BIN"] = POST_EDIT_SERVER_BIN;
        mapStringRcKey["POST_EDIT_CLIENT_BIN"] = POST_EDIT_CLIENT_BIN;

        mapStringRcKey["WORLD_ALIGNMENT_SERVER_BIN"] = WORLD_ALIGNMENT_SERVER_BIN;
        mapStringRcKey["WORLD_ALIGNMENT_CLIENT_BIN"] = WORLD_ALIGNMENT_CLIENT_BIN;

        mapStringRcKey["ICC_CONFIG_WIDGET_BIN"] = ICC_CONFIG_WIDGET_BIN;
        mapStringRcKey["IMAGE_VIEWER_SERVER_BIN"] = IMAGE_VIEWER_SERVER_BIN;
        mapStringRcKey["IMAGE_VIEWER_CLIENT_BIN"] = IMAGE_VIEWER_CLIENT_BIN;

        mapStringRcKey["LOG_OUTPUT"] = LOG_OUTPUT;
        mapStringRcKey["LOG_DIRECTORY"] = LOG_DIRECTORY;
        mapStringRcKey["COLOR_CAPTURE_SERVER_BIN"] = COLOR_CAPTURE_SERVER_BIN;
        mapStringRcKey["COLOR_CAPTURE_SERVER_BIN_3D"] = COLOR_CAPTURE_SERVER_BIN_3D;
        mapStringRcKey["COLOR_MAP_BROWSER_BIN"] = COLOR_MAP_BROWSER_BIN;
        mapStringRcKey["COLOR_MAP_BROWSER_BIN_3D"] = COLOR_MAP_BROWSER_BIN_3D;
        mapStringRcKey["FRAME_TOOL_BIN"] = FRAME_TOOL_BIN;
        mapStringRcKey["UPGRADE_SERVER_BIN"] = UPGRADE_SERVER_BIN;
    mapStringRcKey["GAMMA_SELECTOR_UTILITY"] = GAMMA_SELECTOR_UTILITY;
    mapStringRcKey["USE_VERTEX_SHADER"] = USE_VERTEX_SHADER;
    mapStringRcKey["USE_DEFAULT_FULLSCREEN"] = USE_DEFAULT_FULLSCREEN;

}

char*	IniFile::getKeyValue(mRcKey KEY)
{
    if(this->fileLoaded)
        return (char*)mapRcKeyValues[KEY].c_str();
    else{
        cerr << "ERROR: Attempt to get key value before loading ini file\n";
        return NULL;
    }
}



