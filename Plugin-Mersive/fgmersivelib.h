/**
 * @file fgmersivelib.h
 * @author Ralf Müller
 * @brief Library für Integration Mersive in OpenScene Graph Projects
**/

/* Library for Integration Mersive in Open Scene Graph Projects
 * Author: Ralf Müller
 * Last Change: 19.01.2009
 * (c) Copyright Technische Universität Bergakademie Freiberg - Insitut für Informatik - Virtuelle Realität 2009
 */

// Includes

#include <MersiveRuntimeWarper.h>

#include "IniFile.h"

#include <osg/Camera>
#include <osg/RenderInfo>
#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>
#include <osg/Group>
#include <string>
#include <vector>
#include <list>

//! Definition of the WarperServer Port
#define WARP_SERVER_PORT 8550

//! Cameracallback for Warping the Camera
/**
 * @author Heni Ben Amor, Ralf Müller
 **/


class MersiveOsgWarperCallback : public osg::Camera::DrawCallback
{
    public:
        //! Name of the Displaychannel
        /**  Name of the Display where this Camera works on **/
        std::string m_channelName;
        std::string m_simName;
        //! Width of the Displaychannel
        int m_screenWidth;
        //! Height of the Displaychannel
        int m_screenHeight;
        //RMS vars needed for near/far calcs
        float* otw_terrain_height;
        float* otw_height;
        osg::Vec3d* scene_pos;
        osg::Vec3* scene_att;
        bool* mersivePresent;
        bool* mersiveSetNear;
        osgViewer::Viewer *_viewer;
        osgViewer::View* _view;
        bool low;
        bool high;
        bool _mersive_init;
        bool _mersive_runtime;
        bool _mersive_postDraw;
        bool DEBUG;


        GLdouble *mersiveViewOffset;
        osg::Matrixf translateMatrix;

    private:
        //! the MersiveWarper
        MersiveRuntimeWarper *warper;
        //! deticates if the Callback is initialized
        bool m_inited;
        clipPlanes_t cp;
        clipPlanes_t new_cp;
        float old_near;
        float agl;
        osg::Matrixd pMatrix1;
        osg::Matrixd vMatrix1;


    public:
        //! constructor
        /**
         * the constructor to make this Callback
         * @param channelName the Channel on which the Callback works
         * @param screenWidth the screen width of the Channel
         * @param screenHeigh the screen height of the Channel
         * @param warperIP IP des SolServers
         * @param camera the Camera from this display
         * @param simName name of Display that Channel belongs to (for a sol server licensed for multiple Displays)
         **/
        MersiveOsgWarperCallback(std::string channelName, int screenWidth, int screenHeight, std::string warperIP, osgViewer::CompositeViewer *viewer=NULL, const char *simName=NULL);
        //! Destructor
        virtual ~MersiveOsgWarperCallback(){}
        //! the Callbackfunction
        /**
         * @param renderInfo some infos of rendering from OSG
         * do final initializing and the warping
         **/
        virtual void operator() (osg::RenderInfo& renderInfo) const;
    private:

};

//! Holds an Viewer the WarperCallback for 1 Screen
/**
 * @author Heni Ben Amor, Ralf Müller
 **/
class MersiveViewer
{
    protected:
        //! the screen
        /** one Viewer for every Display  **/
        osg::ref_ptr<osgViewer::Viewer> m_viewer;
        //! the MersiveOsgWarperCallback
        /** Do the warping for the Camera **/
        MersiveOsgWarperCallback *m_warperCallback;
        //! true if this is the Masterviewer
        bool m_masterView;
    public:
        //! Constructor
        /**
         * initialize the Viewer. If master is true, all other Viewers of the Client get the Cameraposition of this Viewer
         * @param screenNum the screen Number of the Channel of this Client (0 - 5)
         * @param channelName the name of the channel
         * @param screenWidth the screen width of the Channel
         * @param screenHeigh the screen height of the Channel
         * @param ip the IP of the solServer
         * @param root the Rootnode of the Scene for setting the SceneData
         * @param master deticates if this MersiveViewer is the MasterViewer of the Client
         * @param simName name of Display that Channel belongs to (for a sol server licensed for multiple Displays)
         **/
        MersiveViewer(int screenNum, std::string channelName, int screenWidth, int screenHeight, std::string ip, osg::Group *root, bool master = false, const char *simName=NULL);
        //! Realize the OSG Viewer
        void realize();
        //! get the OSG Viewer
        /**
         * @return the Viewer from OSG
         **/
        osgViewer::Viewer* getViewer();
        //! return im Master or not
        /**
         * @return if Master (true) or not (false)
         **/
        bool isMaster();
};

//! Holds the MersiveViewer's for 1 Client
/**
 * @author Ralf Müller
 * get Information from /usr/mersivescript.conf
 * */
class ViewerCluster
{
    public:
    private:
        //! The number of Screens of the Client
        int m_numberofscreens;
        //! The IP of the Server from where the Information comes
        std::string m_serverip;
            //! the Display name for a sol server licensed for multiple Displays
        const char *m_simname;
        //! the MersiveScreens for every Display
        std::vector<MersiveViewer*> m_mersiveScreens;

        //! trigger variable to set an Viewer ready
        bool m_ready;
        //! an Pointer to the Group of the scenedata
        osg::Group *m_root;

        //! the list of channel names
        std::vector<std::string> m_channelName;
        //! the list of channel screen numbers
        std::vector<int> m_screenNum;
        //! the list of channel widths
        std::vector<int> m_screenWidth;
        //! the list of channel heights
        std::vector<int> m_screenHeight;
    public:
        //! Constructor
        /**
         * initialize the cluster
         * @param serverIP the IP address of the warper server
         * @param simName name of Display that Channel belongs to (for a sol server licensed for multiple Displays)
         **/
        ViewerCluster(std::string serverIP, const char* simName=NULL);
        //! Destructor
        ~ViewerCluster();
        //! set the information for a Channel
        /**
         * @param channelName name of Channel
         * @param screenNum the screen Number of the Channel of this Client (0 - 5)
         * @param screenWidth screen width of Channel
         * @param screenHeight screen height of Channel
         **/
        void addChannel(std::string channelName, int screenNum, int screenWidth, int screenHeight);
        //! instantiate the MersiveViewer
        /**
         * @param root the Rootnode of the SceneGraph
         * @return true if successfull initialized false if not
         **/
        bool init(osg::Group *root);
        // //! sets the current scene
        // /**
        //  *  set the SceneData from every MersiveViewer
        //  * @param root the SceneData
        // **/
        //void set_scene(osg::Group *root);
        //! realize for every Viewer
        void realize();
        //! returns the Viewer of the Masterdisplay
        /**
         * @return the MasterViewer
         **/
        osgViewer::Viewer* get_masterviewer();
        std::vector<MersiveViewer*> getViewers( void );
        //! Synchronize all Cameras of the Viewers of the Client
        /**
         * Synchronize the Camera Position and Acceleration for every Viewer of this Client
         **/
        void synchronize_viewers();
    private:
    protected:
};

//! Baseclass for every Application
/**
 * @author Ralf Müller, Frank Gommlich
 * Have to be inherited from every Applicationclass
 * @todo should there be an function for setting the homeposition?
 * @todo whats with synchronice?
 * @todo whats with the registered Events? should they be in this Base Class or in an inheritet class?
 **/
class MersiveOSGApplicationBase
{
    public:
    protected:
        //! the Name of the Application
        std::string m_name;
        //! Vector for Events which come from an extern Application
        std::vector<std::string> m_incoming_events;
    private:
    public:
        //! destructor
        /** Have to be always virtual because there are virtual functions in this class **/
        virtual ~MersiveOSGApplicationBase() {}
        //! returns the Name of this Application
        /**
         * @return the name of this Application
         **/
        std::string get_name();
        //! set the name of this application
        /**
         * @param name the name of the Application
         * @todo is this function necessary ?
         **/
        void set_name(std::string name);
        //! adds the applicationscene to an existing Scenegraph
        /**
         * @param root the Group where the scene has to be added
         **/
        virtual void add_to_scene(osg::Group *root) = 0;
        //! shoud update the application
        /**
         * should do the synchronizing
         * @param frameStamp the current framestamp of the Viewer
         **/
        virtual void update(osg::FrameStamp* frameStamp) = 0;
        //! returns the implementet Events of this application
        /**
         * function to registrate some Events in the Main Application. this Events should be implementet in the Application
         * @return vector with name of events, standard is an empty vector
         **/
        virtual std::vector<std::string> get_events() {std::vector<std::string> r; return r;}

        //! adds an event to the incoming events
        /**
         * after adding the event could be processed
         * @param ev the eventname which is added
         **/
        void set_event(std::string ev)
        {
            m_incoming_events.push_back(ev);
        }

        //! adds an vector of events to incoming events
        /**
         * @param ev vector of new incoming events
         **/
        void set_events(std::vector<std::string> ev)
        {
            m_incoming_events.insert(m_incoming_events.begin(), ev.begin(), ev.end());
        }

        //! return the offset for standard home Position of Camera
        /**
         * returns the offsets for the standard home Position of the Camera in eyeposition, lookat and up format
         * @param eyepos_x X offset of eyeposition
         * @param eyepos_y Y offset of eyeposition
         * @param eyepos_z Z offset of eyeposition
         * @param lookat_x X offset of lookat
         * @param lookat_y Y offset of lookat
         * @param lookat_z Z offset of lookat
         * @param up_x X offset of up vector
         * @param up_y Y offset of up vector
         * @param up_z Z offset of up vector
         * @note returns an offset
        **/
        virtual void get_cameraoffset(double &eyepos_x, double &eyepos_y, double &eyepos_z, double &lookat_x, double &lookat_y, double &lookat_z, double &up_x, double &up_y, double &up_z)
        {
            // Attention: this is an offset
            eyepos_x = eyepos_y = eyepos_z = 0;
            lookat_x = lookat_y = lookat_z = 0;
            up_x = up_y = up_z = 0;
        }
    private:
};


//! Class for creating an instance of an MersiveOSGApplicationBase Object
/**
 * inherited classes have to create an instance of there Application
 * @author Ralf Müller
 **/
class MersiveOSGApplicationBaseFactory
{
    private:
        //! ID Counter for the FactoryObjects
        static int id;
        //! ID of this FactoryObject
        int m_id;
        static int get_next_id() {return id++;}
    protected:
        //! Name of the creating Applications
        std::string name;
    public:
        //! Constructor
        MersiveOSGApplicationBaseFactory() {m_id = get_next_id();}
        //! return the ID of this Factory
        /**
         * @return the ID of the Factory
         **/
        int get_id() {return m_id;}
        //! return the Name of the Applications
        /**
         *  @return the Application Name
         **/
        std::string get_name() { return name;}

        //! Destructor
        /**
         * have to be implemented because this class has virtual functions
         **/
        virtual ~MersiveOSGApplicationBaseFactory(void){}
        //! create an instance of the Application
        /**
         * @return an pointer of the new Application
         **/
        virtual MersiveOSGApplicationBase* create(void)=0;
    private:
};

