/*
Pulled straight from Samples/Common/ExampleApplication.h
Class renamed and moved local to avoid incendental editing.


-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/
/*
-----------------------------------------------------------------------------
Filename:    PlanetApplication.h
Description: Base class for all the OGRE examples
-----------------------------------------------------------------------------
*/

#ifndef __PLANET_APPLICATION__
#define __PLANET_APPLICATION__
#include "Ogre.h"
#include "OgreOverlaySystem.h"

#include "PlanetFrameListener.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#include <CoreFoundation/CoreFoundation.h>

// This function will locate the path to our application on OS X,
// unlike windows you can not rely on the curent working directory
// for locating your configuration files and resources.
std::string macBundlePath()
{
    char path[1024];
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    assert(mainBundle);

    CFURLRef mainBundleURL = CFBundleCopyBundleURL(mainBundle);
    assert(mainBundleURL);

    CFStringRef cfStringRef = CFURLCopyFileSystemPath( mainBundleURL, kCFURLPOSIXPathStyle);
    assert(cfStringRef);

    CFStringGetCString(cfStringRef, path, 1024, kCFStringEncodingASCII);

    CFRelease(mainBundleURL);
    CFRelease(cfStringRef);

    return std::string(path);
}
#endif

using namespace Ogre;

/** Base class which manages the standard startup of an Ogre application.
    Designed to be subclassed for specific examples if required.
*/
class PlanetApplication
{
public:
    /// Standard constructor
    PlanetApplication()
    {
        mFrameListener = 0;
        mRoot = 0;
		// Provide a nice cross platform solution for locating the configuration files
		// On windows files are searched for in the current working directory, on OS X however
		// you must provide the full path, the helper function macBundlePath does this for us.
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
		mResourcePath = macBundlePath() + "/Contents/Resources/";
#else
		mResourcePath = "";
#endif
    }
    /// Standard destructor
    virtual ~PlanetApplication()
    {
        if (mFrameListener)
            delete mFrameListener;
        if (mRoot)
            delete mRoot;		
    }

    /// Start the example
    virtual void go(void)
    {
        if (!setup())
            return;

        mRoot->startRendering();

        // clean up
        destroyScene();
    }

protected:
    Root *mRoot;
    Camera* mCamera;
    SceneManager* mSceneMgr;
    PlanetFrameListener* mFrameListener;
    RenderWindow* mWindow;
	Ogre::String mResourcePath;

    // These internal methods package up the stages in the startup process
    /** Sets up the application - returns false if the user chooses to abandon configuration. */
    virtual bool setup(void)
    {

		String pluginsPath;
		// only use plugins.cfg if not static
#ifndef OGRE_STATIC_LIB
		pluginsPath = mResourcePath + "plugins.cfg";
#endif
		
        mRoot = new Root(pluginsPath, 
            mResourcePath + "ogre.cfg", mResourcePath + "Ogre.log");

        setupResources();

        bool carryOn = configure();
        if (!carryOn) return false;

        chooseSceneManager();
        createCamera();
        createViewports();

        // Set default mipmap level (NB some APIs ignore this)
        TextureManager::getSingleton().setDefaultNumMipmaps(5);

		// Create any resource listeners (for loading screens)
		createResourceListener();
		// Load resources
		loadResources();

		// Create the scene
        createScene();

        createFrameListener();

        return true;

    }
    

	 /** Configures the application - returns false if the user chooses to abandon configuration. 
		  Modded - if _DEBUG defined ie in debug mode always accepts previous values in ogre.cfg
	  */
    virtual bool configure(void)
    {
		  // Attempt to load last known good from ogre.cfg when in debug mode
		  // On failure, show config dialog and initialise the system
		  // Time saving only - one less click
#ifdef _DEBUG
		  if (mRoot->restoreConfig()) 
		  {
				mWindow = mRoot->initialise(true);
				return true;
		  }
#endif
			if(mRoot->showConfigDialog(NULL))
			{
				 // If returned true, user clicked OK so initialise
				 // Here we choose to let the system create a default rendering window by passing 'true'
				 mWindow = mRoot->initialise(true);
				 return true;
			}
			else
			{
				 return false;
			}
    }

    virtual void chooseSceneManager(void)
    {
        // Create the SceneManager, in this case the Octree scene manager
        mSceneMgr = mRoot->createSceneManager("OctreeSceneManager");
        Ogre::OverlaySystem* pOverlaySystem = new Ogre::OverlaySystem();
        mSceneMgr->addRenderQueueListener(pOverlaySystem);
    }
    virtual void createCamera(void)
    {
        // Create the camera
        mCamera = mSceneMgr->createCamera("PlanetCam");

        // Position it at 500 in Z direction
        mCamera->setPosition(Vector3(0,0,300));
        // Look back along -Z
        mCamera->lookAt(Vector3(0,0,-300));
        mCamera->setNearClipDistance(5);

    }
    virtual void createFrameListener(void)
    {
        mFrameListener= new PlanetFrameListener(mWindow, mCamera);
        mFrameListener->showDebugOverlay(true);
        mRoot->addFrameListener(mFrameListener);
    }

    virtual void createScene(void) = 0;  // pure virtual - this has to be overridden

    virtual void destroyScene(void){}    // Optional to override this

    virtual void createViewports(void)
    {
        // Create one viewport, entire window
        Viewport* vp = mWindow->addViewport(mCamera);
        vp->setBackgroundColour(ColourValue(0,0,0));

        // Alter the camera aspect ratio to match the viewport
        mCamera->setAspectRatio(
            Real(vp->getActualWidth()) / Real(vp->getActualHeight()));
    }

    /// Method which will define the source of resources (other than current folder)
    virtual void setupResources(void)
    {
        // Load resource paths from config file
        ConfigFile cf;
        cf.load(mResourcePath + "resources.cfg");

        // Go through all sections & settings in the file
        ConfigFile::SectionIterator seci = cf.getSectionIterator();

        String secName, typeName, archName;
        while (seci.hasMoreElements())
        {
            secName = seci.peekNextKey();
            ConfigFile::SettingsMultiMap *settings = seci.getNext();
            ConfigFile::SettingsMultiMap::iterator i;
            for (i = settings->begin(); i != settings->end(); ++i)
            {
                typeName = i->first;
                archName = i->second;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
                // OS X does not set the working directory relative to the app,
                // In order to make things portable on OS X we need to provide
                // the loading with it's own bundle path location
                ResourceGroupManager::getSingleton().addResourceLocation(
                    String(macBundlePath() + "/" + archName), typeName, secName);
#else
                ResourceGroupManager::getSingleton().addResourceLocation(
                    archName, typeName, secName);
#endif
            }
        }
    }

	/// Optional override method where you can create resource listeners (e.g. for loading screens)
	virtual void createResourceListener(void)
	{		 
	}

	/// Optional override method where you can perform resource group loading
	/// Must at least do ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	virtual void loadResources(void)
	{
		// Initialise, parse scripts etc
		ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	}



};


#endif
