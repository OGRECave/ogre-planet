#ifndef __PLANET_QUAD__
#define __PLANET_QUAD__

#include <boost/multi_array.hpp>

#include "OgrePrerequisites.h"
#include "OgreSceneManager.h"

#include "PlanetMovableBox.h"
#include "PlanetQuadNode.h"
#include "PlanetUtils.h"
#include "PlanetLut.h"

namespace OgrePlanet
{
	using namespace Ogre;
	
	class QuadNode;
	class QuadVertex
	{
	public:
		Vector3 position;    // x, y, z
		Vector3 normal;      // Water x, y, z
		ColourValue diffuse; // Diffuse colours for detail texture blending
		Vector2 texCoord0;   // Texture coordinates
	};


	class Quad : public MovableBox 
	{
	public:
		Quad(const String &name, const QuadBounds &plane, const uint32 triDivs);
		virtual ~Quad();
		void build(const long radius, const long level, SceneManager *sceneMgr);
		void showQuad(const QuadNode *quadNode);
		void hideQuad();
		void _updateRenderQueue(RenderQueue* queue);
		void setMaterial(MaterialPtr &material) { mMaterial = material; };
		void setUv(const Vector2 &min, const Vector2 &max);
		void setHeights(const VectorVector3 &heightData, const Real magFactor);
		void calcSlopeHeight(Real &minHeight, Real &maxHeight);
		void normaliseSlopeHeight(const Real minHeight, const Real heightDif, const Lut &lut);
	protected:		
		typedef std::vector<uint16>IndexVector16;
		typedef boost::multi_array<QuadVertex, 2> VertexArray;
		VertexArray mVertexArray;
		const uint32 mTriDivs;
		const uint32 mVertexCount;
		const uint32 mMaxIndexCount;
		uint32 mLastLod;
		bool mVisibleCache;

		void generateVertexBuffer();  // Create vertex and index buffer in hardware
		void populateVertexBuffer();  // Populate vertex buffer
		void populateIndexBuffer(const IndexVector16 &indices);

	private:		
		const uint32 encodeLod(const QuadNode *quadNode);
		void stitchEdge(const QuadEdge edge, long hiLOD, long loLOD, bool omitFirstTri, bool omitLastTri, IndexVector16 &indices);
		inline const uint16 _index(const uint32 x, const uint32 y) { return ((x) + (y*mTriDivs)); }; // x + y*stride
		Quad(const Quad &rhs);
		Quad &operator=(const Quad &rhs);
	};


} // namespace
#endif
