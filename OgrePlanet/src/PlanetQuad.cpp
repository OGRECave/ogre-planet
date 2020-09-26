#include "OgreHardwareBufferManager.h"
#include "OgreVector2.h"

#include "PlanetQuad.h"
#include "PlanetQuadNode.h"
#include "PlanetUtils.h"

// XXX DEBUG - Don't spherise vertex (note bounds used for lod and frustum are still spherised)
// #define NO_SPHERISE

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


	Quad::Quad(const String &name, const QuadBounds &plane, const uint32 triDivs) :
	MovableBox(name, plane), 
	mVertexCount((triDivs+1)*(triDivs+1)), 
	mTriDivs(triDivs+1), 
	mMaxIndexCount(6*((triDivs+1)*(triDivs+1))),
	mVertexArray(mVertexCount),
	mLastLod(0xFFFFFFFF),
	mVisibleCache(false)
	{
		// Populate mVertexArray from provided plane
		long strideX, strideY;
		if ((plane.face == QF_FR)||(plane.face == QF_BK))
		{
			// xy plane
			strideX = plane.getStrideX();
			strideY = plane.getStrideY();
		}
		else if ((plane.face == QF_LF) || (plane.face == QF_RT))
		{
			// zy plane
			strideX = plane.getStrideZ();
			strideY = plane.getStrideY();
		}
		else // assume ((plane.face == QF_UP) || (plane.face == QF_DN))
		{
			// xz plane
			strideX = plane.getStrideX();
			strideY = plane.getStrideZ();
		}

		Real xStep = Real(strideX) / Real(mTriDivs-1);
		Real yStep = Real(strideY) / Real(mTriDivs-1);		
		for(uint32 x=0; x<mTriDivs; x++)
		{
			for (uint32 y=0; y<mTriDivs; y++)
			{
				
				Vector3 v = plane.getDrawOrigin();
				switch(plane.face)
				{
				case QF_FR: 
				case QF_BK:
					v.x += xStep*x;
					v.y += yStep*y;
					break;
				case QF_LF:
				case QF_RT:
					v.z += xStep*x;
					v.y += yStep*y;
					break;
				case QF_UP: 
				case QF_DN:
					v.x += xStep*x;
					v.z += yStep*y;
					break;
				default:
					break;
				}
				mVertexArray[x*mTriDivs + y].position = v;				
			}
		}		
	};

	
	Quad::~Quad() 
	{ 
		// TODO and ... ??
	}; 


	void Quad::_updateRenderQueue( RenderQueue* queue ) 
	{		
		mLightListDirty = true;
		queue->addRenderable(this, mRenderQueueID);
	};

	
	void Quad::showQuad(const QuadNode *quadNode)
	{			
		// Check if anything has changed (this or neighbours), if so update indexes
		const uint32 lod = encodeLod(quadNode);
		if (lod != mLastLod)
		{
			
			IndexVector16 indices;  // Container for indices
			const uint32 localLod = quadNode->getLod();
			const uint32 north = ((quadNode->getNeighbourLod(QE_N) < localLod) ? 1 : 0);
			const uint32 west  = ((quadNode->getNeighbourLod(QE_W) < localLod) ? 1 : 0);
			const uint32 south = ((quadNode->getNeighbourLod(QE_S) < localLod) ? 1 : 0);
			const uint32 east  = ((quadNode->getNeighbourLod(QE_E) < localLod) ? 1 : 0);

			// Do 'central' chunk of vertex at this quad LOD
			for (uint32 x=west; x<mTriDivs-1-east; x++)
			{
				for (uint32 y=north; y<mTriDivs-1-south; y++)
				{	
					// Tri one
					indices.push_back(_index(x, y)); 					
					indices.push_back(_index(x, y+1));  					
					indices.push_back(_index(x+1, y)); 

					// Tri Two
					indices.push_back(_index(x, y+1)); 					
					indices.push_back(_index(x+1, y+1));					
					indices.push_back(_index(x+1, y)); 
				}
			}
			
			
			// Stitch to lower neighbour LOD's as required
			// Smash, grab -n- merge from Ogre Terrain Scene Manager
			// North stitching
			if ( north > 0 )
			{
				uint32 lowLod = (localLod - quadNode->getNeighbourLod(QE_N));
				stitchEdge(QE_N, 0, lowLod, west > 0, east > 0, indices);
			}

			// East stitching
			if ( east > 0 )
			{
				uint32 lowLod = (localLod - quadNode->getNeighbourLod(QE_E));
				stitchEdge(QE_E, 0, lowLod, north > 0, south > 0, indices);
			}
			// South stitching
			if ( south > 0 )
			{
				uint32 lowLod = (localLod - quadNode->getNeighbourLod(QE_S));
				stitchEdge(QE_S, 0, lowLod,	east > 0, west > 0, indices);
			}
			// West stitching
			if ( west > 0 )
			{
				uint32 lowLod = (localLod - quadNode->getNeighbourLod(QE_W));
				stitchEdge(QE_W, 0, lowLod, south > 0, north > 0, indices);
			}
			
			// Write generated data
			populateIndexBuffer(indices);
			indices.clear();

			// Register this change
			mLastLod = lod;
		}

		// Set visible if hidden
		if (mVisibleCache == false)
		{									
			setVisible(true);
			mVisibleCache = true;
		}
	};

	
	void Quad::hideQuad()
	{
		// Wrapped to reduce scene graph queries
		if (mVisibleCache == true)
		{			
			setVisible(false);
			mVisibleCache = false;
		}
	};


	void Quad::build(const long radius, const long level, SceneManager *sceneMgr)
	{
		// TODO spherise first position first
		Vector3 min;
		Vector3 max;
		for(uint32 x=0; x<mTriDivs; x++)
		{
			for (uint32 y=0; y<mTriDivs; y++)
			{	
#ifndef NO_SPHERISE
				Utils::spherise(mVertexArray[x*mTriDivs + y].position, radius);
#endif
				if ((x == 0) && (y == 0))
				{
					// Pickup first vertex as min / max
					min = max = mVertexArray[x*mTriDivs + y].position;
				}
				else
				{					
					// Save min and max for bounds update
					min.x = ((min.x > mVertexArray[x*mTriDivs + y].position.x) ? mVertexArray[x*mTriDivs + y].position.x : min.x);
					min.y = ((min.y > mVertexArray[x*mTriDivs + y].position.y) ? mVertexArray[x*mTriDivs + y].position.y : min.y);
					min.z = ((min.z > mVertexArray[x*mTriDivs + y].position.z) ? mVertexArray[x*mTriDivs + y].position.z : min.z);

					max.x = ((max.x < mVertexArray[x*mTriDivs + y].position.x) ? mVertexArray[x*mTriDivs + y].position.x : max.x);
					max.y = ((max.y < mVertexArray[x*mTriDivs + y].position.y) ? mVertexArray[x*mTriDivs + y].position.y : max.y);
					max.z = ((max.z < mVertexArray[x*mTriDivs + y].position.z) ? mVertexArray[x*mTriDivs + y].position.z : max.z);
				}
			}
		}
		updateBounds(QuadBounds(Vector3Int(min.x, min.y, min.z), Vector3Int(min.x, min.y, min.z),
			Vector3Int(max.x, max.y, max.z), Vector3Int(max.x, max.y, max.z), QuadFace_end));

		generateVertexBuffer();
		populateVertexBuffer();
		
		mParentNode = sceneMgr->getSceneNode(mName.substr(0, mName.find_first_of("+")));
		mParentNode->attachObject(this);		
		setRenderQueueGroup(sceneMgr->getWorldGeometryRenderQueue());
		
		hideQuad();
	};

	
	void Quad::generateVertexBuffer()
	{
		// LOG("SubQuad::generateVertexBuffer()");
		// Lock so can only create once		
		if (mVertexData != NULL)
		{
			// TODO LRU structures for Vertex / Index buffers to save video RAM ?
			// XXX This should never happen 
			
			LOG("SubQuad::generateVertexBuffer() Attempted to regenerate an existing vertex buffer");
			return;
		}


		// Create vertex data object
		mVertexData = new VertexData();
		mVertexData->vertexStart = 0;
		mVertexData->vertexCount = mVertexCount;

		VertexDeclaration *pVertexDecl = mVertexData->vertexDeclaration;
		size_t curOffset = 0;
		pVertexDecl->addElement(0, curOffset, VET_FLOAT3, VES_POSITION);
		curOffset += VertexElement::getTypeSize(VET_FLOAT3);
		pVertexDecl->addElement(0, curOffset, VET_FLOAT3, VES_NORMAL);
		curOffset += VertexElement::getTypeSize(VET_FLOAT3);
		pVertexDecl->addElement(0, curOffset, VET_FLOAT4, VES_DIFFUSE);
		curOffset += VertexElement::getTypeSize(VET_FLOAT4);
		pVertexDecl->addElement(0, curOffset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
		curOffset += VertexElement::getTypeSize(VET_FLOAT2);


		// Allocate vertex buffer in hardware (written to once)
		HardwareVertexBufferSharedPtr pVertBuf =
		  HardwareBufferManager::getSingleton().createVertexBuffer(pVertexDecl->getVertexSize(0), 
		  mVertexData->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
		VertexBufferBinding *pBinding = mVertexData->vertexBufferBinding;
		pBinding->setBinding(0, pVertBuf);  

		// Allocate index buffer in hardware (written to every frame)
		mIndexData = new IndexData;
		mIndexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
		HardwareIndexBuffer::IT_16BIT, mMaxIndexCount, 
		HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
		mIndexData->indexCount = 0;
		mIndexData->indexStart = 0;
	};


	void Quad::populateVertexBuffer()
	{
		HardwareVertexBufferSharedPtr pVertBuf = mVertexData->vertexBufferBinding->getBuffer(0);
		float *pVertex = static_cast<float *>(pVertBuf->lock(HardwareBuffer::HBL_DISCARD));
		
		for (uint32 y=0; y<mTriDivs; y++)
		{
			for (uint32 x=0; x<mTriDivs; x++)					
			{		  
				// Position
				Vector3 v = mVertexArray[x*mTriDivs + y].position;
				*pVertex++ = (float)v.x;
				*pVertex++ = (float)v.y;
				*pVertex++ = (float)v.z;

				// Normal (water level) 
				Vector3 vn = mVertexArray[x*mTriDivs + y].normal; 
				*pVertex++ = (float)vn.x;
				*pVertex++ = (float)vn.y;
				*pVertex++ = (float)vn.z;
				
				// Diffuse (texture blending) // XXX TODO OpenGL BGRA ??
				*pVertex++ = (float)mVertexArray[x*mTriDivs + y].diffuse.r;
				*pVertex++ = (float)mVertexArray[x*mTriDivs + y].diffuse.g;
				*pVertex++ = (float)mVertexArray[x*mTriDivs + y].diffuse.b;
				*pVertex++ = (float)mVertexArray[x*mTriDivs + y].diffuse.a;
				
				// Texcoords 
				Vector2 t = mVertexArray[x*mTriDivs + y].texCoord0;
				*pVertex++ = (float)t.x;
				*pVertex++ = (float)t.y;	
			}
		}
		pVertBuf->unlock();
	};

	
	void Quad::populateIndexBuffer(const IndexVector16 &indices)
	{
		HardwareIndexBufferSharedPtr pIndex = mIndexData->indexBuffer;
		uint16 *pIndexPtr = static_cast<uint16 *>(pIndex->lock(0, mMaxIndexCount, HardwareBuffer::HBL_DISCARD));
		for (IndexVector16::const_iterator iter = indices.begin(); iter != indices.end(); ++iter)
		{
			*pIndexPtr++ = *iter;
		}
		pIndex->unlock();
		mIndexData->indexStart = 0;
		mIndexData->indexCount = indices.size();
	};

	
	const uint32 Quad::encodeLod(const QuadNode *quadNode)
	{
		// TODO assumes not more than 64 Lod
		uint32 local = quadNode->getLod();
		uint32 north = quadNode->getNeighbourLod(QE_N) << 6;
		uint32 west  = quadNode->getNeighbourLod(QE_W) << 12;
		uint32 south = quadNode->getNeighbourLod(QE_S) << 18;
		uint32 east  = quadNode->getNeighbourLod(QE_E) << 24;
		uint32 out = (local | north | west | south | east);
		return out;
	};

	
	// Another smash, grab -n- merge from Ogre Terrain Scene Manager
	void Quad::stitchEdge(const QuadEdge edge, long hiLOD, long loLOD, bool omitFirstTri, 
		bool omitLastTri, IndexVector16 &indices)
	{
		assert(loLOD > hiLOD);
		/* 
		Now do the stitching; we can stitch from any level to any level.
		The stitch pattern is like this for each pair of vertices in the lower LOD
		(excuse the poor ascii art):

		lower LOD
		*-----------*
		|\  \ 3 /  /|
		|1\2 \ / 4/5|
		*--*--*--*--*
		higher LOD

		The algorithm is, for each pair of lower LOD vertices:
		1. Iterate over the higher LOD vertices, generating tris connected to the 
		first lower LOD vertex, up to and including 1/2 the span of the lower LOD 
		over the higher LOD (tris 1-2). Skip the first tri if it is on the edge 
		of the tile and that edge is to be stitched itself.
		2. Generate a single tri for the middle using the 2 lower LOD vertices and 
		the middle vertex of the higher LOD (tri 3). 
		3. Iterate over the higher LOD vertices from 1/2 the span of the lower LOD
		to the end, generating tris connected to the second lower LOD vertex 
		(tris 4-5). Skip the last tri if it is on the edge of a tile and that
		edge is to be stitched itself.

		The same algorithm works for all edges of the patch; stitching is done
		clockwise so that the origin and steps used change, but the general
		approach does not.
		*/

		
		// Work out the steps ie how to increment indexes
		// Step from one vertex to another in the high detail version
		long step = 1 << hiLOD;
		// Step from one vertex to another in the low detail version
		long superstep = 1 << loLOD;
		// Step half way between low detail steps
		long halfsuperstep = superstep >> 1;

		// Work out the starting points and sign of increments
		// We always work the strip clockwise
		long startx, starty, endx, rowstep;
		bool horizontal;
		switch(edge)
		{
		case QE_N:
			startx = starty = 0;
			endx = mTriDivs-1;
			rowstep = step;
			horizontal = true;
			break;
		case QE_W: 
			startx = mTriDivs-1;
			endx = 0;
			starty = 0;
			rowstep = step;
			step = -step;
			superstep = -superstep;
			halfsuperstep = -halfsuperstep;
			horizontal = false;
			break;
		case QE_S: 
			// invert x AND y direction, helps to keep same winding
			startx = starty = mTriDivs-1;
			endx = 0;
			rowstep = -step;
			step = -step;
			superstep = -superstep;
			halfsuperstep = -halfsuperstep;
			horizontal = true;
			break;
		case QE_E:
			startx = 0;
			starty = endx = mTriDivs-1;
			rowstep = -step;
			horizontal = false;
			break;
		};

		long numIndexes = 0;

		for ( int j = startx; j != endx; j += superstep )
		{
			long k;
			for (k = 0; k != halfsuperstep; k += step)
			{
				long jk = j + k;
				//skip the first bit of the corner?
				if ( j != startx || k != 0 || !omitFirstTri )
				{
					if (horizontal)
					{
						indices.push_back(_index(j , starty ));
						indices.push_back(_index(jk, starty + rowstep ));
						indices.push_back(_index(jk + step, starty + rowstep ));
					}
					else
					{
						indices.push_back(_index(starty, j ));
						indices.push_back(_index(starty + rowstep, jk ));
						indices.push_back(_index(starty + rowstep, jk + step));
					}
				}
			}

			// Middle tri
			if (horizontal)
			{
				indices.push_back(_index(j, starty ));
				indices.push_back(_index(j + halfsuperstep, starty + rowstep));
				indices.push_back(_index(j + superstep, starty ));
			}
			else
			{
				indices.push_back(_index(starty, j ));
				indices.push_back(_index(starty + rowstep, j + halfsuperstep ));
				indices.push_back(_index(starty, j + superstep ));
			}

			for (k = halfsuperstep; k != superstep; k += step)
			{
				long jk = j + k;
				if ( j != endx - superstep || k != superstep - step || !omitLastTri )
				{
					if (horizontal)
					{
						indices.push_back(_index(j + superstep, starty ));
						indices.push_back(_index(jk, starty + rowstep ));
						indices.push_back(_index(jk + step, starty + rowstep ));
					}
					else
					{
						indices.push_back(_index(starty, j + superstep ));
						indices.push_back(_index(starty + rowstep, jk ));
						indices.push_back(_index(starty + rowstep, jk + step ));
					}
				}
			}
		}
	};

	
	void Quad::setUv(const Vector2 &min, const Vector2 &max)
	{
		Real strideX = max.x - min.x;
		Real strideY = max.y - min.y;
		Real xStep = Real(strideX) / Real(mTriDivs-1);
		Real yStep = Real(strideY) / Real(mTriDivs-1);		
		for(uint32 x=0; x<mTriDivs; x++)
		{
			for (uint32 y=0; y<mTriDivs; y++)
			{
				mVertexArray[x*mTriDivs + y].texCoord0.x = min.x+xStep*x;
				mVertexArray[x*mTriDivs + y].texCoord0.y = min.y+yStep*y;
			}
		}
		populateVertexBuffer();
	};

	
	void Quad::setHeights(const VectorVector3 &heightData, const Real magFactor)
	{
		/* 
		 *  Basic method http://freespace.virgin.net/hugo.elias/models/m_landsp.htm
		 *  For a given number of iterations
		 *		Create a random vector through the sphere, use random vector and origin as extents of a plane
		 *			For each vertex
		 *				Using dot product establish which side of created plane this vertex lies on
		 *					Move vertex either 'in' a little or 'out' at little
		 */


		// Should have vertex / direction pairs in passed vector
		assert(heightData.size() %2 == 0);

		// Create a temp buffer to reduce floating point math
		std::vector<long> offset;
		for(uint32 x=0; x<mTriDivs; x++)
		{
			for (uint32 y=0; y<mTriDivs; y++)
			{
				offset.push_back(0);
			}
		}


		// Compare the random data to the vertex of this quad updating 'offset'
		VectorVector3::const_iterator iter = heightData.begin(); 
		while(iter != heightData.end())			
		{
			// Pull out random vertex and direction
			Vector3 rand = *iter;
			++iter;
			Vector3 dir = *iter;
			++iter;
			
			const long c = ((dir == Vector3(1, 0, 0)) ? 1 : -1);
		
			for(uint32 x=0; x<mTriDivs; x++)
			{
				for (uint32 y=0; y<mTriDivs; y++)
				{
					// Determine which side of the plane defined by 
					// random vector 'rand' this vertex lies on
					Vector3 d(mVertexArray[x*mTriDivs + y].position - rand);
					if (d.dotProduct(rand) > 0) 
					{
						// Increase the 'height' of this vertex					
						offset.at(x+y*mTriDivs) += c;
					} 
					else 
					{
						 // decrease the 'height' of this vertex
						offset.at(x+y*mTriDivs) -= c;
					}
				}
			}
		}
		
		// Apply the generated 'offset' to the vertex of this quad
		for(uint32 x=0; x<mTriDivs; x++)
		{
			for (uint32 y=0; y<mTriDivs; y++)
			{
				// Save the original sphere vertex position as water level
				mVertexArray[x*mTriDivs + y].normal = mVertexArray[x*mTriDivs + y].position;

				// Get a normal and project distance speced in offset, add to original vertex
				Vector3 project = mVertexArray[x*mTriDivs + y].position.normalisedCopy() * magFactor;
				project *= offset.at(x+y*mTriDivs);
				mVertexArray[x*mTriDivs + y].position += project;
			}
		}

		offset.clear();
	};

	
	void Quad::calcSlopeHeight(Real &minHeight, Real &maxHeight)
	{
		/*
		 *  0 7 6 
		 *  1   5 
		 *  2 3 4
		 *
		 */
		const uint32 NUM_SLOPE = 8;
		for(uint32 x=0; x<mTriDivs; x++)
		{
			for (uint32 y=0; y<mTriDivs; y++)
			{	
				
				// Calculate slope & absolute height
				Vector3 bias = mVertexArray[x*mTriDivs + y].position;
				Real height = bias.length();
				Real slope = 0;
				Vector3 slopes[NUM_SLOPE];							
				slopes[0] = ((x != 0) && (y != 0)) ? mVertexArray[(x-1)*mTriDivs + y-1].position : mVertexArray[x*mTriDivs + y].position;
				slopes[1] = (x != 0) ? mVertexArray[(x-1)*mTriDivs + y].position : mVertexArray[x*mTriDivs + y].position;
				slopes[2] = ((x != 0) && (y != mTriDivs-1)) ? mVertexArray[(x-1)*mTriDivs + y+1].position : mVertexArray[x*mTriDivs + y].position;
				slopes[3] = (y != mTriDivs-1) ? mVertexArray[x*mTriDivs + y+1].position : mVertexArray[x*mTriDivs + y].position;
				slopes[4] = ((x != mTriDivs-1) && (y != mTriDivs-1)) ? mVertexArray[(x+1)*mTriDivs + y+1].position : mVertexArray[x*mTriDivs + y].position;
				slopes[5] = (x != mTriDivs-1) ? mVertexArray[(x+1)*mTriDivs + y].position : mVertexArray[x*mTriDivs + y].position;
				slopes[6] = ((x != mTriDivs-1) && (y != 0)) ? mVertexArray[(x+1)*mTriDivs + y - 1].position : mVertexArray[x*mTriDivs + y].position;
				slopes[7] = (y != 0) ? mVertexArray[x*mTriDivs + y-1].position : mVertexArray[x*mTriDivs + y].position;				
				for (uint32 i=0; i<NUM_SLOPE; i++)
				{
					slopes[i] -= bias;					
					slope += slopes[i].length();
				}
				slope /= (NUM_SLOPE * height);
				if (slope < 0)
				{
					// Slopes always +ve
					slope = -(slope);
				}
				if (height < 0)
				{
					// Heights always +ve
					height = -(height);
				}
								
				// The height values need to be normalised, but min/max heights for sphere currently unknown
				// Hacky store in a, r in interum
				mVertexArray[x*mTriDivs + y].diffuse.a = slope;
				mVertexArray[x*mTriDivs + y].diffuse.r = height;

				// Record min/max height as we go
				if ((x == 0) && (y == 0))
				{
					// Set on first pass
					minHeight = height;
					maxHeight = height;
				}
				else
				{
					// Update
					minHeight = (minHeight > height) ? height : minHeight;
					maxHeight = (maxHeight < height) ? height : maxHeight;
				}
			}
		}
	};

	
	void Quad::normaliseSlopeHeight(const Real minHeight, const Real heightDif, const Lut &lut)
	{
		assert(heightDif != 0);
		for(uint32 x=0; x<mTriDivs; x++)
		{
			for (uint32 y=0; y<mTriDivs; y++)
			{	
				// Pickup stored slope, height values set range (0..1) for lut lookup
				Real slope = mVertexArray[x*mTriDivs + y].diffuse.a;
				Real height = mVertexArray[x*mTriDivs + y].diffuse.r;
				height -= minHeight;
				height /= heightDif;
				
				// Do lookup and assign
				Vector2 xy(height, slope);
				lut.lookup(xy, mVertexArray[x*mTriDivs + y].diffuse);
			}
		}
		populateVertexBuffer();
	};
}
