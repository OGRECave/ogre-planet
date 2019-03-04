#ifndef __PLANET_PLANET__
#define __PLANET_PLANET__

#include "OgrePrerequisites.h"

#include "PlanetStateObj.h"
#include "PlanetUtils.h"


namespace OgrePlanet
{
	using namespace Ogre;

	class QuadRoot;

	/**		
		An Octagon that is subdivided and smoothed to a sphere
		This isn't an Icosphere any more - it is an octagon
		IcoSpheres cause visibility / culling problems with octree scene graph
	*/
	class Planet : public StateObj
	{
	public:
		Planet(String name, const long radius, const uint32 quadDivs);
		virtual ~Planet();
		void build(SceneManager *sceneMgr);
		void finalise(const uint32 iterations = 200, const long magDivisor = 200);
		void render(Camera *camera);  // Per frame
		uint32 getQuadDivs() { return mQuadDivs; };
		uint32 getTriDivs() { return mTriDivs; };
		void setMaterial(const String &matName);
	
	protected:
		static const uint32 NUM_FRAME_LOD = 4;  // Frames rendered between LOD changes
		std::string mName;               // Of sphere (used in scene graph)
		const long mRadius;              // Of sphere
		uint32 mNextRender;              // Frames till next LOD update
		uint32 mQuadDivs;                // Quad divisons per base triangle pair 
		uint32 mTriDivs;                 // Tri divisions per quad
		QuadRoot *mQuadRoot;
		SceneManager *mSceneMgr;
		void generateHeighData(VectorVector3 &heightData, const uint32 iterations);
	private:
		 // No copy constructor
		Planet(Planet &rhs);
		Planet& operator=(const Planet &rhs);		
	};

}
#endif
