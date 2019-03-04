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



	// Flag for one time init
	bool QuadNeighbour::mReady = false;
	
	// Convience map of how positions connect to neighbours
	QuadPosition QuadNeighbour::LocalMap[QuadPosition_end][QuadEdge_end];

	// Map of what neighbours link when spanning different cube face boundaries
	// [face][position][edge] = position
	QuadPosition QuadNeighbour::CubeMap[QuadFace_end][QuadPosition_end][QuadEdge_end];	

	/// Return the complement of the given edge 
	// ie. N -> S, S -> N, E ->W, W -> E
	const QuadEdge QuadNeighbour::complement(const QuadEdge edge)
	{
		QuadEdge out;
		switch(edge)
		{
			case QE_N: out = QE_S; break;
			case QE_S: out = QE_N; break;
			case QE_W: out = QE_E; break;
			case QE_E: out = QE_W; break;
			default: out = QE_N; break;
		}
		return out;
	};

	
	const QuadPosition QuadNeighbour::GetNeighbour(const QuadNode *quadNode, const QuadEdge edge)
	{
		// First time initalisation
		if (!mReady)	
		{ 
			initMap(); 
			mReady = true;
		}

	
		// Save some pointer navigation
		const QuadPosition position = quadNode->getPosition();
		const QuadFace face = quadNode->getFace();
		const QuadFace edgeFace = quadNode->getParent()->getEdge(edge)->getFace();		


		// Determine which position this QuadNode resides at
		if (position == QP_NW)
		{
			// Check if this edge touches another cube face
			if ((edge == QE_N) || (edge == QE_W))
			{
				return ((face != edgeFace) ? CubeMap[face][position][edge] : LocalMap[position][edge]);
			}
			else return LocalMap[position][edge];

		}
		else if (position == QP_SW)
		{
			// Check if this edge touches another cube face
			if ((edge == QE_W) || (edge == QE_S))
			{
				return ((face != edgeFace) ? CubeMap[face][position][edge] : LocalMap[position][edge]);
			}
			else return LocalMap[position][edge];
		}
		else if (position == QP_SE)
		{
			// Check if this edge touches another cube face
			if ((edge == QE_S) || (edge == QE_E))
			{
				return ((face != edgeFace) ? CubeMap[face][position][edge] : LocalMap[position][edge]);
			}
			else return LocalMap[position][edge];
		}
		else // assume (position == QP_NE)
		{
			// Check if this edge touches another cube face
			if ((edge == QE_N) || (edge == QE_E))
			{
				return ((face != edgeFace) ? CubeMap[face][position][edge] : LocalMap[position][edge]);
			}
			else return LocalMap[position][edge];
		}		
	};


	void QuadNeighbour::initMap()
	{
		// --------------------------------------------------------------------
		// How neighbours link on the same cube face ie 'internal mapping'
		//
		//	NW -- NE
		//  |     |
		//  SW -- SE
		//

		// North west quadrant
		LocalMap[QP_NW][QE_N] = QP_SW;
		LocalMap[QP_NW][QE_W] = QP_NE;
		LocalMap[QP_NW][QE_S] = QP_SW;
		LocalMap[QP_NW][QE_E] = QP_NE;

		// South west quadrant
		LocalMap[QP_SW][QE_N] = QP_NW;
		LocalMap[QP_SW][QE_W] = QP_SE;
		LocalMap[QP_SW][QE_S] = QP_NW;
		LocalMap[QP_SW][QE_E] = QP_SE;

		// South east quadrant
		LocalMap[QP_SE][QE_N] = QP_NE;
		LocalMap[QP_SE][QE_W] = QP_SW;
		LocalMap[QP_SE][QE_S] = QP_NE;
		LocalMap[QP_SE][QE_E] = QP_SW;

		// North east quadrant
		LocalMap[QP_NE][QE_N] = QP_SE;
		LocalMap[QP_NE][QE_W] = QP_NW;
		LocalMap[QP_NE][QE_S] = QP_SE;
		LocalMap[QP_NE][QE_E] = QP_NW;


		
		// --------------------------------------------------------------------
		// How neighbours link when spanning different cube faces
		// [face][position][neighbour] = new position
		
		// Note that this is a sparse array - not all possible indexes are used		
		// Initalise everything with 'internal mapping' as a starting point
		
		for (QuadFace face=QuadFace_begin; face!=QuadFace_end; ++face)
		{			
			CubeMap[face][QP_NW][QE_N] = QP_SW;
			CubeMap[face][QP_NW][QE_W] = QP_NE;
			CubeMap[face][QP_NW][QE_S] = QP_SW;
			CubeMap[face][QP_NW][QE_E] = QP_NE;

			CubeMap[face][QP_SW][QE_N] = QP_NW;
			CubeMap[face][QP_SW][QE_W] = QP_SE;
			CubeMap[face][QP_SW][QE_S] = QP_NW;
			CubeMap[face][QP_SW][QE_E] = QP_SE;

			CubeMap[face][QP_SE][QE_N] = QP_NE;
			CubeMap[face][QP_SE][QE_W] = QP_SW;
			CubeMap[face][QP_SE][QE_S] = QP_NE;
			CubeMap[face][QP_SE][QE_E] = QP_SW;

			CubeMap[face][QP_NE][QE_N] = QP_SE;
			CubeMap[face][QP_NE][QE_W] = QP_NW;
			CubeMap[face][QP_NE][QE_S] = QP_SE;
			CubeMap[face][QP_NE][QE_E] = QP_NW;
		}

		// Add special cases for when sides are spanning two cube faces
		// Arrived at via very a hightek solution - making a cube with graph paper and tape...

		// FR
		CubeMap[QF_FR][QP_NW][QE_N] = QP_SW;
		CubeMap[QF_FR][QP_NW][QE_W] = QP_NE;
		CubeMap[QF_FR][QP_SW][QE_W] = QP_SE;
		CubeMap[QF_FR][QP_SW][QE_S] = QP_NW;
		CubeMap[QF_FR][QP_SE][QE_S] = QP_NE;
		CubeMap[QF_FR][QP_SE][QE_E] = QP_SW;
		CubeMap[QF_FR][QP_NE][QE_E] = QP_NW;
		CubeMap[QF_FR][QP_NE][QE_N] = QP_SE;

		// BK
		CubeMap[QF_BK][QP_NW][QE_N] = QP_SW;
		CubeMap[QF_BK][QP_NW][QE_W] = QP_SW;
		CubeMap[QF_BK][QP_SW][QE_W] = QP_NW;
		CubeMap[QF_BK][QP_SW][QE_S] = QP_NW;
		CubeMap[QF_BK][QP_SE][QE_S] = QP_NE;
		CubeMap[QF_BK][QP_SE][QE_E] = QP_NE;
		CubeMap[QF_BK][QP_NE][QE_E] = QP_SE;
		CubeMap[QF_BK][QP_NE][QE_N] = QP_SE;
		

		// LF
		CubeMap[QF_LF][QP_NW][QE_N] = QP_NW;
		CubeMap[QF_LF][QP_NW][QE_W] = QP_SW;
		CubeMap[QF_LF][QP_SW][QE_W] = QP_NW;
		CubeMap[QF_LF][QP_SW][QE_S] = QP_SW;
		CubeMap[QF_LF][QP_SE][QE_S] = QP_NW;
		CubeMap[QF_LF][QP_SE][QE_E] = QP_SW;
		CubeMap[QF_LF][QP_NE][QE_E] = QP_NW;
		CubeMap[QF_LF][QP_NE][QE_N] = QP_SW;

		// RT
		CubeMap[QF_RT][QP_NW][QE_N] = QP_SE;
		CubeMap[QF_RT][QP_NW][QE_W] = QP_NE;
		CubeMap[QF_RT][QP_SW][QE_W] = QP_SE;
		CubeMap[QF_RT][QP_SW][QE_S] = QP_NE;
		CubeMap[QF_RT][QP_SE][QE_S] = QP_SE;
		CubeMap[QF_RT][QP_SE][QE_E] = QP_NE;		
		CubeMap[QF_RT][QP_NE][QE_E] = QP_SE;
		CubeMap[QF_RT][QP_NE][QE_N] = QP_NE;

		// UP
		CubeMap[QF_UP][QP_NW][QE_N] = QP_SW;
		CubeMap[QF_UP][QP_NW][QE_W] = QP_NW;
		CubeMap[QF_UP][QP_SW][QE_W] = QP_NE;
		CubeMap[QF_UP][QP_SW][QE_S] = QP_NW;
		CubeMap[QF_UP][QP_SE][QE_S] = QP_NE;
		CubeMap[QF_UP][QP_SE][QE_E] = QP_NW;		
		CubeMap[QF_UP][QP_NE][QE_E] = QP_NE;
		CubeMap[QF_UP][QP_NE][QE_N] = QP_SE;

		// DN
		CubeMap[QF_DN][QP_NW][QE_N] = QP_SW;
		CubeMap[QF_DN][QP_NW][QE_W] = QP_SE;
		CubeMap[QF_DN][QP_SW][QE_W] = QP_SW;
		CubeMap[QF_DN][QP_SW][QE_S] = QP_NW;
		CubeMap[QF_DN][QP_SE][QE_S] = QP_NE;
		CubeMap[QF_DN][QP_SE][QE_E] = QP_SE;
		CubeMap[QF_DN][QP_NE][QE_E] = QP_SW;
		CubeMap[QF_DN][QP_NE][QE_N] = QP_SE;

	};


} // namespace