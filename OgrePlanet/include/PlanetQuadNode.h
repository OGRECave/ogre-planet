#ifndef __PLANET_QUAD_NODE__
#define __PLANET_QUAD_NODE__

#include "OgrePreRequisites.h"
#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreSceneNode.h"
#include "OgreManualObject.h" // XXX DEBUG

#include "PlanetEnum.h"
#include "PlanetQuadBounds.h"
#include "PlanetUtils.h"
#include "PlanetLut.h"

namespace OgrePlanet
{

	using namespace Ogre;

	
	/** Position of this quad on parent quad 
	 * NW NE
	 * SW SE
	 */
	enum QuadPosition
	{
		QuadPosition_begin = 0,
		QP_NW = QuadPosition_begin, QP_SW, QP_SE, QP_NE, 
		QuadPosition_end,
		QP_ROOT  // Special case for root faces - not iterated
	};
	QuadPosition &operator++ (QuadPosition &position);
	


	
	/** The edge neighbours of this QuadNode
	 *   N
	 * W o E
	 *   S
	 */
	enum QuadEdge
	{
		QuadEdge_begin = 0,
		QE_N = QuadEdge_begin, QE_W, QE_S, QE_E,
		QuadEdge_end
	};
	QuadEdge &operator++ (QuadEdge &edge);
	


	/** Helper function for neighbour map
	 * Linking across cube faces is messy
	 */
	class QuadNode;
	class QuadNeighbour
	{
	public:
		static const QuadPosition GetNeighbour(const QuadNode *quadNode, const QuadEdge edge);
		static const QuadEdge complement(const QuadEdge edge);
	private:
		static void initMap();
		static bool mReady;

		// Map of how neighbours link on the same cube face
		// [position][edge] = position
		static QuadPosition LocalMap[QuadPosition_end][QuadEdge_end];

		// Map of how neighbours link when spanning different cube face boundaries
		// [face][position][edge] = position
		static QuadPosition CubeMap[QuadFace_end][QuadPosition_end][QuadEdge_end];	
	};


	/** A QuadNode
	*/
	class QuadRoot;
	class Quad;
	class QuadNode
	{	
		friend QuadRoot;

	public:
		virtual ~QuadNode() { } ;
		
		// Accessors
		const QuadPosition getPosition() const { return mPosition; };
		const QuadFace getFace() const { return mBounds.face; };
		const QuadNode *getChild(const QuadPosition position) const { return mChildren[position]; };
		const QuadNode *getEdge(const QuadEdge edge) const { return mEdge[edge]; };
		const QuadNode *getParent() const
		{ 			
			return ((mParent != NULL) ? mParent : this); // Calls to parent of root bounce back
		};
		const bool getEdgeIsSplit(const QuadEdge edge) const 
		{ 			
			return ((mEdge[edge]) ? (mEdge[edge]->mIsSplit) : false); // Check that edge exists before returning 
		};
		const uint32 getNeighbourLod(const QuadEdge edge) const
		{
			return ((mEdge[edge]->mRenderLod != LOD_NO_RENDER) ? mEdge[edge]->mRenderLod : mLevel);
		};
		const bool hasChildren() const { return (mChildren[0] != NULL); };
		const uint32 getLod() const { return mRenderLod; };
		const Vector3 getCenter() const { return mBounds.getCenter(); };
		const uint32 getLevel() const { return mLevel; };
		
		// Actions
		void subDivide(const uint32 divide, const long radius);	
		void link();
		void setUv(const Vector2 &min, const Vector2 &max);
		void buildQuad(const uint32 triDivs, const long radius, const String &name, SceneManager *sceneMgr); // generate renderable
		void renderCache(const long radius, const uint32 quadDivs, const Camera *camera, const SceneNode *sceneNode);  // Set lods for render() pass
		void render();  // Update renderables
		void hide();
		void setHeights(const VectorVector3 &heightData, const Real magFactor);
		void calcSlopeHeight(Real &minHeight, Real &maxHeight);
		void normaliseSlopeHeight(const Real minHeight, const Real heightDif, const Lut &lut);
		void setMaterial(MaterialPtr &material);

		static const uint32 LOD_NO_RENDER    = 0xFFFFFFFF;
		static const uint32 LOD_RENDER_CHILD = 0xFFFFFFFE;
		
	private:		
		/// Child node constructor called by QuadRoot
		QuadNode(QuadNode *parent, const QuadBounds &bounds, const QuadPosition position);		
		
		// No copy constructor
		QuadNode(const QuadNode &rhs);
		QuadNode &operator=(const QuadNode &rhs);

	private:		
		void zeroPointers();  // Called by constructor
		void linkChildOnEdge(const QuadPosition child, const QuadEdge edge);  // Called when all children built		
		void split(const long radius); // Called by subdivide		
		void hideAllChildren();
		void setChildrenLod(const uint32 lod);
		void relink(const QuadNode *newLink, const QuadEdge edge, const QuadPosition posA, const QuadPosition posB);
		bool findChildPosOnEdge(const QuadNode *link, QuadEdge &edge, QuadPosition &posA, QuadPosition &posB);
		void tearDownChildren();

		// DEBUG functions
		void draw(ManualObject *manual, const long radius); // XXX DEBUG
		void drawNeighbours(ManualObject *manual, const long radius); // XXX DEBUG
		void drawBox(ManualObject *manual, const long radius);  // XXX DEBUG

		const QuadNode *mParent;
		QuadNode *mChildren[QuadPosition_end];
		QuadNode *mEdge[QuadEdge_end];
		const QuadPosition mPosition;
		QuadBounds mBounds;
		const uint32 mLevel;
		bool mIsSplit;
		Quad *mQuad;  // Renderable 
		uint32 mRenderLod; // Lod level for next frame
	};

	
	/** Container / initiator for six root QuadNodes (each cube side)
	*/
	class QuadRoot
	{		

	public:
		QuadRoot(const long radius, const uint32 quadDivs, const uint32 triDivs);
		virtual ~QuadRoot();
		void build(SceneManager *sceneMgr, SceneNode *sceneNode, const String &name);
		void finalise(const VectorVector3 &heightData, const Real magFactor);
		void render(Camera *camera);
		void setMaterial(const String &matName);
		static const uint32 getNextId() { return mNextId++; };
	private:
		const long getViewDepth(const QuadNode *quadNode, const Camera *camera) const;
		const long mRadius;
		const uint32 mQuadDivs;
		const uint32 mTriDivs;
		static uint32 mNextId;  // Used for distinct names of QuadNodes
		QuadNode *mRoots[QuadFace_end];
		SceneNode *mSceneNode;
	};


} // namespace
#endif
