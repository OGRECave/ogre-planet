#include "Ogre.h"

#include "PlanetPlanet.h"
#include "PlanetLogger.h"
#include "PlanetQuadNode.h"


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

namespace OgrePlanet
{
	
	using namespace Ogre;

	Planet::Planet(String name, const long radius, const uint32 quadDivs) :
	mNextRender(0),  
	mName(name), 
	mRadius(radius),
	mTriDivs(4),  // 33x33 vertex - batch size of 1089 = about optimal with shaders
	mQuadDivs(quadDivs), 
	mQuadRoot(NULL),
	mSceneMgr(NULL)
	{	
		LOG("Planet::Planet() " + mName);
		
		// Sanity check on arguments
		assert(radius != 0);
		
		// Idiot proofing
		if (mQuadDivs == 0)
		{
			// Need at least one division
			mQuadDivs = 1;
		}				
		LOG("QuadDivs: " + StringOf(mQuadDivs) + " TriDivs: " + StringOf(mTriDivs));

		// Initalise Quad manager
		mQuadRoot = new QuadRoot(mRadius, mQuadDivs, mTriDivs);
		
		setState(STATE_PREBUILD);
	};


	Planet::~Planet() 
	{
		LOG("Planet::~Planet()");
		
		// TODO revisit and do properly
		// Scene node clean up
		SceneNode *node = mSceneMgr->getSceneNode(mName);
		node->detachAllObjects();
		// sceneMgr->destroySceneNode(mName);  // XXX 'not recomended' in Ogre docs

		if (mQuadRoot)
		{
			delete mQuadRoot;
			mQuadRoot = NULL;
		}
	};



	/**	Create the base Planet
	*/	
	void Planet::build(SceneManager *sceneMgr) 
	{	
		LOG("Planet::build()");
		mSceneMgr = sceneMgr;

		if (getState() != STATE_PREBUILD)
		{			
			LOG("Planet::build() stat is not STATE_PREBUILD...");			
		}
		else
		{
			// Create a 'root' scene node for this object (SubQuads will attach)	
			SceneNode *sceneNode = sceneMgr->getRootSceneNode()->createChildSceneNode(mName);
			LOG("Planet::build() Created object 'root' scene node: " + mName);
			mQuadRoot->build(sceneMgr, sceneNode, mName);			
			setState(STATE_BUILT);
		}
	};


	void Planet::finalise(const uint32 iterations, const long magDivisor)
	{	
		if (getState() != STATE_BUILT)
		{
			LOG("Planet::finalise() called and state is not STATE_BUILT");
			return;
		}

		// TODO serialisaton - to rebuild same planet, data in this vector needs to be saved/loaded
		// Fractalise heights to make landscape
		VectorVector3 heightData;
		generateHeighData(heightData, iterations);
		Real magFactor = Real(mRadius/magDivisor);
		mQuadRoot->finalise(heightData, magFactor);
		heightData.clear();

		setState(STATE_READY);
	};


	/** Generate random data for sphere -> planet deformation
	*/
	void Planet::generateHeighData(VectorVector3 &heightData, const uint32 iterations)
	{			 
		// For a number of iterations
		for (uint32 i=0; i<iterations; i++) 
		{
			// Generate a random vector through the sphere
			Vector3 r = Utils::randVector(1); 
			 
			// Select direction of move constant for this iteration
			Vector3 c;
			if (Utils::randReal() > 0) 
			{
				c = Vector3(1, 0, 0);
			} 
			else 
			{
				c = Vector3(-1, 0, 0);
			}				
			heightData.push_back(r);
			heightData.push_back(c);
		}
	};
	


	void Planet::render(Camera *camera)
	{	
		if (getState() != STATE_READY)
		{
			LOG("Planet::render() called and state is not STATE_READY");
			return;
		}

		// Only bother rendering (updating LOD via indexes) every NUM_FRAME_LOD frames
		mNextRender ++;
		if (mNextRender > NUM_FRAME_LOD)
		{
			mNextRender = 0;
			mQuadRoot->render(camera);
		}	
	};


	void Planet::setMaterial(const String &matName)
	{
		if (getState() != STATE_READY)
		{
			LOG("Planet::setMaterial() called and state is not STATE_READY");
			return;
		}
		mQuadRoot->setMaterial(matName);
	};	
}
