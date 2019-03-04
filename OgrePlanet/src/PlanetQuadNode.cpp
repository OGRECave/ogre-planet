#include "OgreManualObject.h"
#include "OgreViewport.h"
#include "OgreVector2.h"

#include "PlanetQuadNode.h"
#include "PlanetLogger.h"
#include "PlanetQuad.h"

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


	// Enum iteration
	QuadPosition &operator++ (QuadPosition &position)
	{
		return enum_increment(position, QuadPosition_begin, QuadPosition_end);
	};

	QuadEdge &operator++ (QuadEdge &edge)
	{
		return enum_increment(edge, QuadEdge_begin, QuadEdge_end);
	};

	QuadFace &operator++ (QuadFace &face)
	{
		return enum_increment(face, QuadFace_begin, QuadFace_end);
	};
	
	String &toString(QuadFace &face)
	{
		static String out;
		switch(face)
		{
			case QF_FR: out = "_FR"; break;
			case QF_BK: out = "_BK"; break;
			case QF_LF: out = "_LF"; break;
			case QF_RT: out = "_RT"; break;
			case QF_UP: out = "_UP"; break;
			case QF_DN: out = "_DN"; break;
			default: out = "_BAD_FACE"; break;
		}
		return out;
	};



	/**
	 * A node of a quad tree
	 * One quad tree is constructed for each face of the cube.
	 * The tree is built once to down to mQuadDivs level and a renderable created at each node.
	 *
	 * During rendering, the visibility of a quad at a given level in the tree is determined
	 * by the projected size (based on distance of quad center from camera).
	 * If a quad is visible, all children are hidden and the children of any neighbours are reindexed 
	 * to point to the adjacent visible quad, rather than the adjacent child at a lower level in the tree.
	 *
	 * After each render pass, the 'default' linkage (ie. child at level X to child at level X) is reestablished
	 *
	 */
	QuadNode::QuadNode(QuadNode *parent, const QuadBounds &bounds, const QuadPosition position) :
	mParent(parent),
	mLevel((parent != NULL) ? (parent->mLevel+1) : 0), 
	mBounds(bounds),
	mPosition(position),
	mIsSplit(false),
	mQuad(NULL), 
	mRenderLod(0)
	{ 
		zeroPointers();
/* 
		LOG("QuadNode() parent: "  + StringOf(mParent) 
			+ " level: " + StringOf(mLevel) + " bounds: " 
			+ StringConverterEx::toString(mBounds) 
			+ " position: " + StringOf(mPosition));
*/
	};
	
	/// NULL internal child and neighbour pointers (called by constructor)
	void QuadNode::zeroPointers()
	{
		for (QuadPosition child=QuadPosition_begin; child!=QuadPosition_end; ++child)
		{
			mChildren[child] = NULL;
		}
		for(QuadEdge edge=QuadEdge_begin; edge!=QuadEdge_end; ++edge)
		{
			mEdge[edge] = NULL;
		}
	};


	void QuadNode::tearDownChildren()
	{
		if (hasChildren())
		{
			for (QuadPosition child=QuadPosition_begin; child!=QuadPosition_end; ++child)
			{
				if(mChildren[child]->hasChildren())
				{
					mChildren[child]->tearDownChildren();
				}
				delete mChildren[child];
				mChildren[child] = NULL;
			}
		}
	};

	
	void QuadNode::buildQuad(const uint32 triDivs, const long radius, const String &name, SceneManager *sceneMgr)
	{		
		// Build renderable
		String quadName = name + "+Quad" + StringOf(QuadRoot::getNextId()); 
		// LOG("Building Quad: " + quadName);
		mQuad = new Quad(quadName, mBounds, triDivs);
		mQuad->build(radius, mLevel, sceneMgr);

		if (hasChildren())
		{	
			// Recurse
			for(QuadPosition child=QuadPosition_begin; child!=QuadPosition_end; ++child)
			{
				mChildren[child]->buildQuad(triDivs, radius, name, sceneMgr);
			}	
		}			
		
		// Spherize bounds for frustum checks (all children now created)
		mBounds.spherise(radius); 			
	};
	
	
	void QuadNode::subDivide(const uint32 divides, const long radius)
	{
		if (divides > 0)
		{	
			// Split this node and recurse
			split(radius);  
			for(QuadPosition child=QuadPosition_begin; child!=QuadPosition_end; ++child)
			{
				mChildren[child]->subDivide(divides-1, radius);
			}
		}
	};

	
	/** Create four child nodes	and update 'internal' edge linkages
	 */
	void QuadNode::split(const long radius)
	{
		if (!mIsSplit)
		{
			// Calc stride
			uint32 stride = (uint32)radius;
			stride >>= mLevel;

			// Calc points of new quad
			QuadBounds nw(mBounds);
			QuadBounds sw(mBounds);		
			QuadBounds se(mBounds);
			QuadBounds ne(mBounds);		
			mBounds.getSplit(nw, sw, se, ne, stride);

			// Create new QuadNodes
			mChildren[QP_NW] = new QuadNode(this, nw, QP_NW);
			mChildren[QP_SW] = new QuadNode(this, sw, QP_SW);
			mChildren[QP_SE] = new QuadNode(this, se, QP_SE);
			mChildren[QP_NE] = new QuadNode(this, ne, QP_NE);
			mIsSplit = true;

			// Link the 'internals' of this quad
			mChildren[QP_NW]->mEdge[QE_S] = mChildren[QP_SW];
			mChildren[QP_SW]->mEdge[QE_N] = mChildren[QP_NW];
			mChildren[QP_SW]->mEdge[QE_E] = mChildren[QP_SE];
			mChildren[QP_SE]->mEdge[QE_W] = mChildren[QP_SW];
			mChildren[QP_SE]->mEdge[QE_N] = mChildren[QP_NE];
			mChildren[QP_NE]->mEdge[QE_S] = mChildren[QP_SE];
			mChildren[QP_NE]->mEdge[QE_W] = mChildren[QP_NW];
			mChildren[QP_NW]->mEdge[QE_E] = mChildren[QP_NE];
		}
	};


	/** Post tree construction, establish links between nodes (recurses)
	 */
	void QuadNode::link()
	{
		// After everything is split() link external QuadNode edges to each other
		if (hasChildren())
		{
			// Link edges to children that may be on other quads
			linkChildOnEdge(QP_NW, QE_N);
			linkChildOnEdge(QP_NW, QE_W);
			linkChildOnEdge(QP_SW, QE_W);
			linkChildOnEdge(QP_SW, QE_S);
			linkChildOnEdge(QP_SE, QE_S);
			linkChildOnEdge(QP_SE, QE_E);
			linkChildOnEdge(QP_NE, QE_E);
			linkChildOnEdge(QP_NE, QE_N);

			// Recurse
			for(QuadPosition child=QuadPosition_begin; child!=QuadPosition_end; ++child)
			{
				mChildren[child]->link();	
			}
		}
	};



	/** Link a child node to another child node (may be part of different quad trees - ie on different cube face)
	 */
	void QuadNode::linkChildOnEdge(const QuadPosition child, const QuadEdge edge)
	{	
		// Everything should have been split before reaching here
		const QuadPosition index = QuadNeighbour::GetNeighbour(mChildren[child], edge);			
		
		// Edge of this child to edge of adjacent child
		mChildren[child]->mEdge[edge] = const_cast<QuadNode *>(getEdge(edge)->getChild(index));		
	};
	

	/// XXX DEBUG TESTS
	void QuadNode::draw(ManualObject *manual, const long radius)
	{		
		if (hasChildren())
		{
			for(QuadPosition child=QuadPosition_begin; child!=QuadPosition_end; ++child)
			{
				mChildren[child]->draw(manual, radius);
			}
		}		
		if (mLevel == 3) 
		{			
			drawNeighbours(manual, radius);
			//drawBox(manual, radius);
		}
	};

	
	/// XXX DEBUG TESTS
	void QuadNode::drawNeighbours(ManualObject *manual, const long radius)
	{
		Vector3 center = mBounds.getCenter();
		Vector3 north, west, south, east;
		
		if (mEdge[QE_N])
		{
			north = mEdge[QE_N]->mBounds.getCenter();
		}
		if (mEdge[QE_W])
		{
			west = mEdge[QE_W]->mBounds.getCenter();
		}
		if (mEdge[QE_S])
		{
			south = mEdge[QE_S]->mBounds.getCenter();
		}
		if (mEdge[QE_E])
		{
			east = mEdge[QE_E]->mBounds.getCenter();
		}
		
		manual->colour(ColourValue::White);
		manual->position(center);				
		manual->position(north);		
		manual->colour(ColourValue::Red);
		manual->position(center);
		manual->position(west);				
		manual->colour(ColourValue::Green);
		manual->position(center);
		manual->position(south);
		manual->colour(ColourValue::Blue);
		manual->position(center);
		manual->position(east);		
	};


	/// XXX DEBUG TESTS
	void QuadNode::drawBox(ManualObject *manual, const long radius)
	{
		const long stride = ((mLevel > 0) ? (radius >> (mLevel-1)) : (radius << 1));
		AxisAlignedBox box = mBounds.getPlane();			
		Vector3 a = box.getCorner(AxisAlignedBox::NEAR_LEFT_BOTTOM);
		Vector3 b = box.getCorner(AxisAlignedBox::NEAR_RIGHT_BOTTOM);
		Vector3 c = box.getCorner(AxisAlignedBox::NEAR_RIGHT_TOP);
		Vector3 d = box.getCorner(AxisAlignedBox::NEAR_LEFT_TOP);	
		Vector3 e = box.getCorner(AxisAlignedBox::FAR_LEFT_TOP);
		Vector3 f = box.getCorner(AxisAlignedBox::FAR_LEFT_BOTTOM);
		Vector3 g = box.getCorner(AxisAlignedBox::FAR_RIGHT_BOTTOM);
		Vector3 h = box.getCorner(AxisAlignedBox::FAR_RIGHT_TOP);	
		switch(mPosition)
		{
			case QP_NW: manual->colour(ColourValue::White); break;
			case QP_SW: manual->colour(ColourValue::Red); break;
			case QP_SE: manual->colour(ColourValue::Green); break;
			case QP_NE: manual->colour(ColourValue::Blue); break;
			default : manual->colour(ColourValue::White); break;  // QP_ROOT
		}
		manual->position(a);
		manual->position(b);
		manual->position(b);
		manual->position(c);
		manual->position(c);
		manual->position(d);	
		manual->position(d);
		manual->position(a);


		manual->position(e);
		manual->position(f);
		manual->position(f);
		manual->position(g);
		manual->position(g);
		manual->position(h);	
		manual->position(h);
		manual->position(e);

		manual->position(b);
		manual->position(g);
		manual->position(g);
		manual->position(h);
		manual->position(h);
		manual->position(c);	
		manual->position(c);
		manual->position(b);

		manual->position(a);
		manual->position(f);
		manual->position(f);
		manual->position(e);
		manual->position(e);
		manual->position(d);	
		manual->position(d);
		manual->position(a);

		// Bottom + top inferred as are drawing wireframe
	};


	/** Establish which nodes are visible and update linkages
	 */
	void QuadNode::renderCache(const long radius, const uint32 quadDivs, const Camera *camera, const SceneNode *sceneNode)
	{
		// Frustum cull to speed up rendering (note mBounds spherised during buildQuad)
		// Don't bother continuing to children if parent not visible		
		AxisAlignedBox worldBox = mBounds.getPlane();
		worldBox.transform(sceneNode->_getFullTransform());
		if (camera->isVisible(worldBox))
		{
			// Determine the projected size of the Quad
			// Note '10' is near clip plane and a kludge based on what comes out of project function
			// TODO store oneToOne value at each node? - what about camera screen width changes?
			// TODO assumes fov of 45 degree (= 1.0) what if zooming et al.
			// Full perspective projection formulae = diameter * sceenWidth / (z * 2fov)

			// Calculate 1:1 render size for quad width diameter (diameter >> mLevel)
			const long screenWidth = camera->getViewport()->getActualWidth();		
			const long oneToOne = radius / (radius >> mLevel) * screenWidth / 10; 
			
			// Calculate projected size	
			// TODO sqrt() performance ouch...	
			const Vector3 worldBoxCen = worldBox.getCenter();
			const Vector3 &cameraCen = camera->getDerivedPosition();			
			const long distanceCenter = (worldBoxCen - cameraCen).length();
			const long projectedPixels = radius * screenWidth / distanceCenter;
		
			/*
			if (mBounds.face == QF_FR)
			{
				LOG("mLevel: " + StringOf(mLevel) + 
				" position: " + StringOf(mPosition) + 
				" distance: " + StringOf(distanceCenter) +
				" projected: " + StringOf(projectedPixels) + 
				" one: " + StringOf(oneToOne));
			}
			*/
				
			
			// Determine if we should draw at this lod		
			if ((projectedPixels < oneToOne) || (hasChildren() == false))
			{
				/*
				LOG("Rendered: " + StringOf(mPosition) + 
					" level: " + StringOf(mLevel) +
					" distance: " + StringOf(distanceCenter) +
					" projected: " + StringOf(projectedPixels) +
					" sizeLod: " + StringOf(oneToOne));
				*/

				// Flag as visible with given lod and hide children
				mRenderLod = mLevel;
				hideAllChildren();

				// Relink any children of neighbours to point directly to this node 
				// rather than to children of this node
				QuadPosition posA, posB;
				QuadEdge edge;
				if (mEdge[QE_N]->findChildPosOnEdge(this, edge, posA, posB))
				{
					mEdge[QE_N]->relink(this, edge, posA, posB);
				}
				if (mEdge[QE_W]->findChildPosOnEdge(this, edge, posA, posB))
				{
					mEdge[QE_W]->relink(this, edge, posA, posB);
				}
				if (mEdge[QE_S]->findChildPosOnEdge(this, edge, posA, posB))
				{
					mEdge[QE_S]->relink(this, edge, posA, posB);				
				}
				if (mEdge[QE_E]->findChildPosOnEdge(this, edge, posA, posB))
				{
					mEdge[QE_E]->relink(this, edge, posA, posB);
				}
			}
			else 
			{
				// Camera is too close to render at this (low) level, recurse to children
				mRenderLod = LOD_RENDER_CHILD;
				mQuad->hideQuad();
				for(QuadPosition child=QuadPosition_begin; child!=QuadPosition_end; ++child)
				{
					mChildren[child]->renderCache(radius, quadDivs, camera, sceneNode);
				}
			}
		}
		
		// Outside frustum, hide and hide all children
		else
		{	
			// LOG("Frustum culled: position: " + StringOf(mPosition) + " level: " + StringOf(mLevel));
			mRenderLod = LOD_NO_RENDER;
			mQuad->hideQuad();
			hideAllChildren();
		}
	};
	
	
	/** Post CacheRender draw visible quads
	 */
	void QuadNode::render()
	{
		// Check if inside frustum
		if (mRenderLod != LOD_NO_RENDER)
		{
			if (mRenderLod != LOD_RENDER_CHILD)
			{
				mQuad->showQuad(this);
			}			
			else
			{
				// Must have children or would have rendered
				for(QuadPosition child=QuadPosition_begin; child!=QuadPosition_end; ++child)
				{
					mChildren[child]->render();
				}
			}
		}
		// else Outside frustum and already hidden
	};
	
	
	/** Hide all renderables for children of a given node
	 */
	void QuadNode::hideAllChildren()
	{		
		// Recurse
		if (hasChildren())
		{
			for(QuadPosition child=QuadPosition_begin; child!=QuadPosition_end; ++child)
			{
				mChildren[child]->mRenderLod = LOD_NO_RENDER;				
				mChildren[child]->mQuad->hideQuad();
				mChildren[child]->hideAllChildren();
			}
		}
	};


	/** Set the lod for all children of a given node
	 */
	void QuadNode::setChildrenLod(const uint32 lod)
	{
		if (hasChildren())
		{
			for(QuadPosition child=QuadPosition_begin; child!=QuadPosition_end; ++child)
			{
				// Set immediate child lod
				mChildren[child]->mRenderLod = lod;

				// Recurse to children of children
				mChildren[child]->setChildrenLod(lod);
			}
		}		
	};

	
	/** Hide a node renderable and hide all children
	 */
	void QuadNode::hide()
	{
		mRenderLod = LOD_NO_RENDER;
		hideAllChildren();
	};

	
	/** Apply the given u, v values to the renderable of a node and all children
	*/
	void QuadNode::setUv(const Vector2 &min, const Vector2 &max)
	{	
		// Set u, v for this quad
		mQuad->setUv(min, max);
			
		if (hasChildren())
		{
			// Split
			/*
			 * min   d
			 *   +---+---+
			 *   |       |
			 *   |       |
			 * a +   e   + c
			 *   |       |
			 *   |       |
			 *   +---+---+
			 *       b   max
			 */
			Real halfStrideX = (max.x - min.x)/2;
			Real halfStrideY = (max.y - min.y)/2;
			Vector2 a(min.x, min.y+halfStrideY);
			Vector2 b(a.x+halfStrideX, a.y+halfStrideY);
			Vector2 d(min.x+halfStrideX, min.y);
			Vector2 e(a.x+halfStrideX, a.y);
			Vector2 c(e.x+halfStrideX, e.y);

			// Recurse
			mChildren[QP_NW]->setUv(min, e);
			mChildren[QP_SW]->setUv(a, b);
			mChildren[QP_SE]->setUv(e, max);
			mChildren[QP_NE]->setUv(d, c);
		}
	};


	void QuadNode::setHeights(const VectorVector3 &heightData, const Real magFactor)
	{
		// Finalise this quad
		mQuad->setHeights(heightData, magFactor);
		
		// Recurse
		if (hasChildren())
		{
			for(QuadPosition child=QuadPosition_begin; child!=QuadPosition_end; ++child)
			{
				mChildren[child]->setHeights(heightData, magFactor);
			}
		}		
	};


	void QuadNode::calcSlopeHeight(Real &minHeight, Real &maxHeight)
	{
		// Store min / max as will be overwritten each Quad calcSlopeHeight() call
		// Note: QuadRoot initalises min / max to radius
		Real lastMin = minHeight;
		Real lastMax = maxHeight;
		mQuad->calcSlopeHeight(minHeight, maxHeight);
		
		// Maintain min / max
		minHeight = (lastMin < minHeight) ? lastMin : minHeight;
		maxHeight = (lastMax > maxHeight) ? lastMax : maxHeight;
				
		// Recurse
		if (hasChildren())
		{
			for(QuadPosition child=QuadPosition_begin; child!=QuadPosition_end; ++child)
			{
				mChildren[child]->calcSlopeHeight(minHeight, maxHeight);
			}
		}		
	};

	
	void QuadNode::normaliseSlopeHeight(const Real minHeight, const Real heightDif, const Lut &lut)
	{
		mQuad->normaliseSlopeHeight(minHeight, heightDif, lut);
		
		// Recurse
		if (hasChildren())
		{
			for(QuadPosition child=QuadPosition_begin; child!=QuadPosition_end; ++child)
			{
				mChildren[child]->normaliseSlopeHeight(minHeight, heightDif, lut);
			}
		}
	};

	
	void QuadNode::setMaterial(MaterialPtr &material)
	{
		// Apply material to this quad
		mQuad->setMaterial(material);

		// Recurse
		if (hasChildren())
		{
			for(QuadPosition child=QuadPosition_begin; child!=QuadPosition_end; ++child)
			{
				mChildren[child]->setMaterial(material);
			}
		}		
	};


	/// Given an edge of a parent, determine the positions of the two children touching this edge and what the edge is
	bool QuadNode::findChildPosOnEdge(const QuadNode *link, QuadEdge &edge, QuadPosition &posA, QuadPosition &posB)
	{	
		posA = posB = QP_ROOT;
		for(QuadEdge thisEdge=QuadEdge_begin; thisEdge!=QuadEdge_end; ++thisEdge)
		{
			if (mEdge[thisEdge] == link)
			{
				switch(thisEdge)
				{
					// Record positions
					case QE_N: posA = QP_NW, posB = QP_NE; break;
					case QE_W: posA = QP_NW, posB = QP_SW; break;
					case QE_S: posA = QP_SW, posB = QP_SE; break;
					case QE_E: posA = QP_SE, posB = QP_NE; break;
				}
				// Record edge
				edge = thisEdge;
				break; // for loop
			}
		}

		// Check that the edge was actually found
		return(posA != QP_ROOT);
	};

	
	/// Force children of given node to attach to new edge
	// Use findChildPosOnEdge to establish posA, posB and edge
	void QuadNode::relink(const QuadNode *newLink, const QuadEdge edge, 
		const QuadPosition posA, const QuadPosition posB)
	{	
		// Check has children and relink
		if (hasChildren())
		{
			mChildren[posA]->mEdge[edge] = const_cast<QuadNode *>(newLink);
			mChildren[posB]->mEdge[edge] = const_cast<QuadNode *>(newLink);
			mChildren[posA]->relink(newLink, edge, posA, posB);
			mChildren[posB]->relink(newLink, edge, posA, posB);
		}
	};
};