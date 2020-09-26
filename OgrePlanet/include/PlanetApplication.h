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

#include <OgreApplicationContext.h>

using namespace Ogre;

/** Base class which manages the standard startup of an Ogre application.
    Designed to be subclassed for specific examples if required.
*/
class PlanetApplication : public OgreBites::ApplicationContext, public OgreBites::InputListener
{
public:
    /// Standard constructor
    PlanetApplication() : OgreBites::ApplicationContext() {}
    /// Standard destructor
    virtual ~PlanetApplication() {}

    /// Start the example
    virtual void go(void)
    {
        initApp();

        mRoot->startRendering();

        // clean up
        destroyScene();
        closeApp();
    }

protected:
    Camera* mCamera;
    SceneNode* mCameraNode;
    SceneManager* mSceneMgr;

    // These internal methods package up the stages in the startup process
    virtual void setup(void)
    {
        OgreBites::ApplicationContext::setup();

        addInputListener(this);

        chooseSceneManager();
        createCamera();
        createViewports();

		// Create the scene
        createScene();

        createFrameListener();
    }

    virtual void chooseSceneManager(void)
    {
        mSceneMgr = mRoot->createSceneManager();
        mSceneMgr->addRenderQueueListener(getOverlaySystem());
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

    }

    virtual void createScene(void) = 0;  // pure virtual - this has to be overridden

    virtual void destroyScene(void){}    // Optional to override this

    virtual void createViewports(void)
    {
        // Create one viewport, entire window
        Viewport* vp = getRenderWindow()->addViewport(mCamera);
        vp->setBackgroundColour(ColourValue(0,0,0));

        // Alter the camera aspect ratio to match the viewport
        mCamera->setAspectRatio(
            Real(vp->getActualWidth()) / Real(vp->getActualHeight()));
    }

};


#endif
