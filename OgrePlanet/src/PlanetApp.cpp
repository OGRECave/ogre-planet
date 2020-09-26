#include "PlanetApplication.h"
#include "PlanetLogger.h"

#include "PlanetPlanet.h"

#include <OgreTrays.h>
#include <OgreCameraMan.h>
#include <OgreAdvancedRenderControls.h>

#include <iostream>

/*
 * OgrePlanet dynamic level of detail for planetary rendering
 * Copyright (C) 2008 Beau Hardy 
 * http://www.gamepsychogony.co.nz
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in 
 * the Software without restriction, including without limitation the rights to 
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
 * of the Software, and to permit persons to whom the Software is furnished to do 
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 */

// Declare a subclass of the PlanetApplication class
class PlanetApp : public PlanetApplication 
{
public:
	PlanetApp() : mIcoSphere(NULL), mFreezeLOD(false) { };
	virtual ~PlanetApp() { };

protected:
	// OgreBites
	OgreBites::TrayManager* mTrayMgr;
	OgreBites::CameraMan* mCameraMan;       // basic camera controller
	OgreBites::AdvancedRenderControls* mAdvancedControls;     // sample details panel
	OgrePlanet::Planet *mIcoSphere;

	bool mFreezeLOD;

	bool frameEnded(const Ogre::FrameEvent& evt)
	{
		if ((mIcoSphere != NULL) && (mCamera != NULL))
		{
			if (mFreezeLOD == false)
			{
				mIcoSphere->render(mCamera);
			}
		}

		return true;
	}

	bool keyPressed(const OgreBites::KeyboardEvent& arg)
	{
		if (arg.keysym.sym == OgreBites::SDLK_ESCAPE)
		{
			getRoot()->queueEndRendering();
		}
		else if (arg.keysym.sym == 'l')
		{
			mFreezeLOD = !mFreezeLOD;
		}


		return true;
	}

   /** Define what is in the scene
	 */
   void createScene(void) 
   {
	   LOG("PlanetApp::createScene()");
		  
		// Global lighting
		mSceneMgr->setAmbientLight( ColourValue( Real(0.9), Real(0.9), Real(0.9) ) );

		// Create a spot light to give some depth
		Light *pLight = mSceneMgr->createLight("PlanetSpotLight");
		pLight->setType(Light::LT_SPOTLIGHT);
		pLight->setSpotlightRange(Degree(25), Degree(50));
		pLight->setDirection(-0.5, -0.5, 0);
		pLight->setPosition(0, -1000, -1000);		 


		// Set up some fog (darkness) XXX surplus to requirements for now
		// ColourValue fadeColour(Real(0.05), Real(0.05), Real(0.05));
        // mWindow->getViewport(0)->setBackgroundColour(fadeColour);
		// mSceneMgr->setFog(FOG_LINEAR, fadeColour, 0.0, 256, 1024);

		// Set up a stationary starfield texture
		// XXX surplus to requirements for now mSceneMgr->setSkyBox(true, "Quad/QuadSphereSkyBox", 10);
		
		// Create an instance of an IcoSphere and set material
		mIcoSphere = new OgrePlanet::Planet("Planet", 512, 2); // XXX 3);		
		mIcoSphere->build(mSceneMgr);
		mIcoSphere->finalise(2000, 350);
		mIcoSphere->setMaterial("Planet/Planet"); // XXX ("Planet/TestMaterial")
	 };

	
   virtual void destroyScene(void) 
	{
		delete mIcoSphere;
		mIcoSphere = NULL;
	};
  

	 /** Create and register the frame listener
	  */
	 void createFrameListener(void) 
	 {
		LOG("PlanetApp::createFrameListener()");

		mTrayMgr = new OgreBites::TrayManager("InterfaceName", getRenderWindow());
		mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
		mTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);
		mTrayMgr->hideCursor();
		addInputListener(mTrayMgr);

		mAdvancedControls= new OgreBites::AdvancedRenderControls(mTrayMgr, mCamera);
		addInputListener(mAdvancedControls);
	 };

	
	 /** Create and point the camera
	 */
	void createCamera(void)
	{
		LOG("PlanetApp::createCamera()");

		// Create the camera
		mCamera = mSceneMgr->createCamera("PlayerCam");
		mCameraNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

		// Position it at 800 in Z direction
		mCameraNode->setPosition(Vector3(0, 0, 2000));
		// Look back along -Z
		mCameraNode->lookAt(Vector3(0,0,0), Node::TS_WORLD);
		mCamera->setNearClipDistance(10);
		mCamera->setFarClipDistance(10000);

		mCameraNode->attachObject(mCamera);

		mCameraMan = new OgreBites::CameraMan(mCameraNode); // create a default camera controller
		mCameraMan->setStyle(OgreBites::CS_ORBIT);
		addInputListener(mCameraMan);
	};

         /** Create a single viewport
	  */
	 void createViewports(void)
     {
		 LOG("PlanetApp::createViewports()");

		// Create one viewport, entire window
		// Note that the size of the ORGE window / full screen is defined by OGRE setup dialogs
		// Can be in .cfg file also
		Viewport* vp = getRenderWindow()->addViewport(mCamera);
		vp->setBackgroundColour(ColourValue(0,0,0));

		// Alter the camera aspect ratio to match the viewport
		mCamera->setAspectRatio(Real(vp->getActualWidth()) / Real(vp->getActualHeight()));
    };
	
	void loadResources(void) 
	{
		LOG("PlanetApp::loadResources()");
 		
		// Initialise, parse scripts etc
		ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	};

};

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 

#define WIN32_LEAN_AND_MEAN 
#include "windows.h" 
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT) 
#else 
int main(int argc, char **argv) 
#endif 
{
    // Instantiate our subclass
    PlanetApp *myApp = new PlanetApp();

    try 
	{
        // PlanetApplication provides a go() method, which starts the rendering.
        myApp->go();
    }
	catch (Exception &e) 
	{
        std::cerr << "Application exception:\n";
		std::cerr << e.getFullDescription() << "\n";
        return 1;
	}
	
	delete myApp;
	myApp = NULL;
	
	return 0;
}
