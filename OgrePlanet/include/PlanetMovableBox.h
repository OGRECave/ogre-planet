#ifndef __PLANET_MOVABLE_BOX__
#define __PLANET_MOVABLE_BOX__


#include "OgreMovableObject.h"
#include "OgreRenderable.h"

#include "PlanetQuadBounds.h"

namespace OgrePlanet
{

	using namespace Ogre;

	/** Wrapper / specialisation of MovableObject, Renderable
	*/
	class MovableBox : public Ogre::MovableObject, public Ogre::Renderable
	{
	public:		
		MovableBox(const String &name, const QuadBounds &bounds);
		virtual ~MovableBox(); 
		void updateBounds(const QuadBounds &bounds);
	protected:
				

		// Renderable
		VertexData *mVertexData;
		IndexData  *mIndexData;
		HardwareVertexBufferSharedPtr mBuffer;
		SceneNode *mParentNode;
		mutable bool mLightListDirty;
		MaterialPtr mMaterial;

		// MovableObject
		AxisAlignedBox mBoundBox;  // AABB of this object
		Real mBoundingRadius;      // Bounding radius of this object
		Vector3 mCenter;           // Center of the AABB
		
	public:
		// Required virtuals --------------------------------------------------
		// Renderable
		Real getSquaredViewDepth(const Camera* cam) const;
		const MaterialPtr& getMaterial(void) const;
		void getRenderOperation( RenderOperation& op );
		void getWorldTransforms( Matrix4* xform ) const;
		const Quaternion& getWorldOrientation(void) const;
		const Vector3& getWorldPosition(void) const;
		const LightList& getLights(void) const;										
		
		// MovableObject
		void _updateRenderQueue( RenderQueue* queue );
		const String& getMovableType(void) const;
		uint32 getTypeFlags(void) const;
		const AxisAlignedBox& getBoundingBox( void ) const;
		Real getBoundingRadius(void) const;
	
	};
}
#endif