#include "OgreViewport.h"
#include "OgreMaterialManager.h"

#include "PlanetQuadNode.h"
#include "PlanetQuad.h"
#include "PlanetLut.h"
#include "PlanetLutGenerator.h"

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


// XXX DEBUG flag - use a ManualObject to draw the linkages between quads rather than doing render
//#define DRAW_NETWORKS

namespace OgrePlanet
{
	using namespace Ogre;


	// Generic id counter
	uint32 QuadRoot::mNextId = 0;

	
	// Quick and dirty structure to hold quad positions and view distances
	class QuadDistance
	{
	public:
		QuadDistance(const QuadNode *_node, const long _distance) : node(_node), distance(_distance) { };
		~QuadDistance() { };

		const QuadNode *node;
		const long distance;
		static bool compare(const QuadDistance *qd1, const QuadDistance *qd2)
		{
			return (qd1->distance < qd2->distance);
		};
	};

	
	QuadRoot::QuadRoot(const long radius, const uint32 quadDivs, const uint32 triDivs) :
	mQuadDivs(quadDivs), 
	mTriDivs(triDivs), 
	mRadius(radius),
	mSceneNode(NULL)
	{
		// Ramp up code for QuadNode network
		for(QuadFace face=QuadFace_begin; face!=QuadFace_end; ++face)
		{
			// Create a root for each face of cube
			mRoots[face] = new QuadNode(NULL, QuadBounds::parent(radius, face), QP_ROOT);
		}

		// Set inital linking of cube faces
		mRoots[QF_UP]->mEdge[QE_N] = mRoots[QF_BK];
		mRoots[QF_UP]->mEdge[QE_W] = mRoots[QF_LF];
		mRoots[QF_UP]->mEdge[QE_S] = mRoots[QF_FR];
		mRoots[QF_UP]->mEdge[QE_E] = mRoots[QF_RT];

		mRoots[QF_DN]->mEdge[QE_N] = mRoots[QF_FR];
		mRoots[QF_DN]->mEdge[QE_W] = mRoots[QF_LF];
		mRoots[QF_DN]->mEdge[QE_S] = mRoots[QF_BK];
		mRoots[QF_DN]->mEdge[QE_E] = mRoots[QF_RT];

		mRoots[QF_FR]->mEdge[QE_N] = mRoots[QF_UP];
		mRoots[QF_FR]->mEdge[QE_W] = mRoots[QF_LF];
		mRoots[QF_FR]->mEdge[QE_S] = mRoots[QF_DN];
		mRoots[QF_FR]->mEdge[QE_E] = mRoots[QF_RT];
		
		mRoots[QF_BK]->mEdge[QE_N] = mRoots[QF_DN];
		mRoots[QF_BK]->mEdge[QE_W] = mRoots[QF_LF];
		mRoots[QF_BK]->mEdge[QE_S] = mRoots[QF_UP];
		mRoots[QF_BK]->mEdge[QE_E] = mRoots[QF_RT];

		mRoots[QF_LF]->mEdge[QE_N] = mRoots[QF_UP];
		mRoots[QF_LF]->mEdge[QE_W] = mRoots[QF_BK];
		mRoots[QF_LF]->mEdge[QE_S] = mRoots[QF_DN];
		mRoots[QF_LF]->mEdge[QE_E] = mRoots[QF_FR];

		mRoots[QF_RT]->mEdge[QE_N] = mRoots[QF_UP];
		mRoots[QF_RT]->mEdge[QE_W] = mRoots[QF_FR];
		mRoots[QF_RT]->mEdge[QE_S] = mRoots[QF_DN];
		mRoots[QF_RT]->mEdge[QE_E] = mRoots[QF_BK];
	};

	
	QuadRoot::~QuadRoot()
	{
		for(QuadFace face=QuadFace_begin; face!=QuadFace_end; ++face)
		{
			// Subdivide each face			
			mRoots[face]->tearDownChildren();
			delete mRoots[face];
			mRoots[face] = NULL;
		}
	};


	void QuadRoot::build(SceneManager *sceneMgr, SceneNode *sceneNode, const String &name)
	{
		// Save off scene node 
		// Used to apply node transforms to bounding boxes when frustum checking 
		mSceneNode = sceneNode;


		// Split all faces down to mQuadDivs
		for(QuadFace face=QuadFace_begin; face!=QuadFace_end; ++face)
		{
			// Subdivide each face			
			mRoots[face]->subDivide(mQuadDivs, mRadius);
		}

		
		// Link all external faces down to mQuadDivs
		for(QuadFace face=QuadFace_begin; face!=QuadFace_end; ++face)
		{
			// Link external neighbours (which will exist due to build subDivide pass above)
			mRoots[face]->link();
		}

		
		const uint32 triDivs = Math::Pow(2, mTriDivs);
		for(QuadFace face=QuadFace_begin; face!=QuadFace_end; ++face)
		{
			// Finally build Quads (renderables)
			String nodeName = name + toString(face);
			mSceneNode->createChildSceneNode(nodeName);
			mRoots[face]->buildQuad(triDivs, mRadius, nodeName, sceneMgr);
			if (face != QF_BK)
			{
				mRoots[face]->setUv(Vector2(0, 0), Vector2(1, 1));
			}
			else
			{
				// QF_BK is flipped horizontal and vertical
				mRoots[face]->setUv(Vector2(1, 1), Vector2(0, 0));
			}

		}
#ifdef DRAW_NETWORKS
		// XXX DEBUG draw bounding boxes, neighbours etc 
		ManualObject* manual = sceneMgr->createManualObject("TEST_MANUAL");
		manual->begin("BaseWhiteNoLighting", RenderOperation::OT_LINE_LIST);							
			for(QuadFace face=QuadFace_begin; face!=QuadFace_end; ++face)
			{
				mRoots[face]->draw(manual, mRadius);
			}
		manual->end();
		sceneMgr->getSceneNode(name)->attachObject(manual);	
#endif
	};

	
	void QuadRoot::finalise(const VectorVector3 &heightData, const Real magFactor)
	{
		/* Create lookup table XXX only needs to be created once */
		#if 0
		LutGenerator lutGenerator(512, 0.05f);		
		Vector2 peak(0.70f, 0.90f);
		Vector3 trough(0.65f, 0.80f, 0.95f);		
		lutGenerator.generate(peak, trough, true, true, true, 0.2f);
		lutGenerator.save("../Media/materials/textures/lookup.png"); 
		#endif

		// Set heights and establish min / max height of each face
		Real minHeight[QuadFace_end], maxHeight[QuadFace_end];		
		for(QuadFace face=QuadFace_begin; face!=QuadFace_end; ++face)
		{
			mRoots[face]->setHeights(heightData, magFactor);
			minHeight[face] = maxHeight[face] = mRadius;
			mRoots[face]->calcSlopeHeight(minHeight[face], maxHeight[face]);
		}

		// Establish global min / max of cube from each face
		// Does first index twice - but cleanest way to iterate
		Real globalMin = minHeight[QuadFace_begin];
		Real globalMax = maxHeight[QuadFace_begin];		
		for(QuadFace face=QuadFace_begin; face!=QuadFace_end; ++face)
		{
			globalMin = (minHeight[face] < globalMin) ? minHeight[face] : globalMin;
			globalMax = (maxHeight[face] > globalMax) ? maxHeight[face] : globalMax;
		}

		// Normalise height/slope data
		Real globalHeightDif = globalMax - globalMin;
		Lut lut = Lut::createLut("lookup.png");
		for(QuadFace face=QuadFace_begin; face!=QuadFace_end; ++face)
		{
			mRoots[face]->normaliseSlopeHeight(globalMin, globalHeightDif, lut);
		}
	};

	
	void QuadRoot::render(Camera *camera)
	{
		/*
		 * Do a bit of basic occulsion culling
		 * Work out the distance to faces and sort by distance, rendering closest X only
		 *
		 * Only every bother rendering the closest 5 faces, as that is all that is ever visible
		 * If we are less than 3/2 radius away, the first 4 faces are all that are visible 
		 *
		 * Note this technique also forces rendering front to back
		 * TODO Further optimisations can be made by drilling down to first level of child quads
		 * This would make top level parent faces 'dummy quads' with no renderables
		 *
		 */
		

		// Store faces in list and sort by depth decending
		std::list<QuadDistance *> viewDepth;		
		for(QuadFace face=QuadFace_begin; face!=QuadFace_end; ++face)
		{			
			const QuadNode *quadNode = mRoots[QuadFace(face)];
			long distance = getViewDepth(quadNode, camera);		
			viewDepth.push_back(new QuadDistance (quadNode, distance));
		}
		viewDepth.sort(QuadDistance::compare);
		


		// Pop the faces out of the list, updating Lod and linkages on the visible and hiding the invisible
		std::list<QuadDistance *>::iterator iter = viewDepth.begin();
		uint32 lastOut = 6-5;
		long distance = ((QuadDistance *)(*iter))->distance;		
		if (distance < mRadius*3/2)
		{
			// Check if only the closest six are visible
			lastOut = 6-4;
			// LOG("Rendering first four");
		}
		
		while (iter != viewDepth.end())
		{
			// Get handle to top QuadDistance
			QuadDistance *qd = *iter;
			QuadNode *face = const_cast<QuadNode *>(qd->node);

			// Get next face and render
			if (viewDepth.size() > lastOut)
			{	
				face->renderCache(mRadius, mQuadDivs, camera, mSceneNode);
			}
			else
			{
				face->hide();
			}

			// Clean up
			viewDepth.pop_front();
			delete qd;
			qd = NULL;
			iter = viewDepth.begin();
		}


		/*
		// XXX TEST - draw all unconditionally
		for(QuadFace face=QuadFace_begin; face!=QuadFace_end; ++face)
		{
			mRoots[face]->renderCache(mRadius, mQuadDivs, camera, mSceneNode);
		}
		*/

#ifndef DRAW_NETWORKS
		// Do the render (note hidden faces don't get rendered)
		for(QuadFace face=QuadFace_begin; face!=QuadFace_end; ++face)
		{			
			mRoots[face]->render();
		};
#endif		
		
#ifdef DRAW_NETWORKS
		// XXX DEBUG 
		ManualObject* manual = (ManualObject *)mSceneNode->detachObject("TEST_MANUAL");
		manual->clear();  // XXX Should really be using beginUpdate()
		manual->begin("BaseWhiteNoLighting", RenderOperation::OT_LINE_LIST);							
			for(QuadFace face=QuadFace_begin; face!=QuadFace_end; ++face)
			{
				mRoots[face]->draw(manual, mRadius);
			}			
		manual->end();
		mSceneNode->attachObject(manual);
#endif

		// Restablish default quad network for next pass
		for(QuadFace face=QuadFace_begin; face!=QuadFace_end; ++face)
		{			
			mRoots[face]->link();
		}
	};


	const long QuadRoot::getViewDepth(const QuadNode *quadNode, const Camera *camera) const 
	{
		// Apply scene node transform and work out distance to camera
		const Vector3 &camCen = camera->getDerivedPosition();
        const Matrix4 &transform = mSceneNode->_getFullTransform();
		Vector3 transformedCen = transform * quadNode->getCenter();
        return (transformedCen - camCen).length();
	};

	
	void QuadRoot::setMaterial(const String &matName)
	{
		for(QuadFace face=QuadFace_begin; face!=QuadFace_end; ++face)
		{			
			String fullMatName = matName + toString(face);
			MaterialPtr material = MaterialManager::getSingleton().getByName(fullMatName);
			mRoots[face]->setMaterial(material);
		}
	};


}  // namespace