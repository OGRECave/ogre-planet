#include "OgreCamera.h"
#include "OgreSceneNode.h"
#include "OgreSceneManager.h"

#include "PlanetMovableBox.h"

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

	MovableBox::MovableBox(const String &name, const QuadBounds &bounds) :
	mIndexData(NULL), mVertexData(NULL)
	{
		// LOG("MovableBox::MovableBox()");
		// Set bounds and name
			
		mName = name;
		updateBounds(bounds);

		// LOG("Bounding Box: " + StringOf(mBoundBox) + "rad : " + StringOf(mBoundingRadius));
	};


	MovableBox::~MovableBox() 
	{ 
		if (mIndexData)
		{
			delete mIndexData;
			mIndexData = NULL;
		}
		if (mVertexData)
		{
			delete mVertexData;
			mVertexData = NULL;
		}

	};


	void MovableBox::updateBounds(const QuadBounds &bounds)
	{
		// Note: No bounding box, no draw!
		Vector3 min(bounds.minX(), bounds.minY(), bounds.minZ());
		Vector3 max(bounds.maxX(), bounds.maxY(), bounds.maxZ());
		mBoundBox = AxisAlignedBox(min, max);
		mBoundingRadius = (max - min).length() * Real(0.5);
		mCenter = mBoundBox.getCenter();
	};


	// Movable ----------------------------------------------------------------
	void MovableBox::_updateRenderQueue( RenderQueue* queue ) 
	{
		mLightListDirty = true;
		queue->addRenderable(this, mRenderQueueID);
	};


	const String& MovableBox::getMovableType(void) const 
	{
		// Magic string
		static String movType = "TerrainMipMap"; // XXX "TerrainRenderable";
		return movType;
	};

	uint32 MovableBox::getTypeFlags(void) const
	{
		// Another 'magic' type
		return SceneManager::WORLD_GEOMETRY_TYPE_MASK;
	}

	const AxisAlignedBox& MovableBox::getBoundingBox( void ) const
	{
		return mBoundBox;
	};


	Real MovableBox::getBoundingRadius(void) const 
	{ 
		return mBoundingRadius; 
	};


	// Renderable -------------------------------------------------------------
	const MaterialPtr& MovableBox::getMaterial(void) const
	{
		return mMaterial;
	};


	void MovableBox::getRenderOperation( RenderOperation& op )
	{
		op.useIndexes = true;
		op.operationType = RenderOperation::OT_TRIANGLE_LIST;
		op.vertexData = mVertexData;
		op.indexData = mIndexData;
	};


	void MovableBox::getWorldTransforms( Matrix4* xform ) const
	{
		*xform = mParentNode->_getFullTransform();
	};


	const Quaternion& MovableBox::getWorldOrientation(void) const
	{
		return mParentNode->_getDerivedOrientation();
	};


	const Vector3& MovableBox::getWorldPosition(void) const
	{
		return mParentNode->_getDerivedPosition();
	};

	Real MovableBox::getSquaredViewDepth(const Camera* cam) const
	{
		// Cannot call mParentNode->getSquaredViewDepth() as this may be the same node for all
		// So apply transform of parent node to center of bounding box and calc distance
		const Vector3 &camCen = cam->getDerivedPosition();
        const Matrix4 &transform = mParentNode->_getFullTransform();
		Vector3 transformedCen = transform * mCenter;
        return (transformedCen - camCen).squaredLength();
	};


	const LightList& MovableBox::getLights(void) const
	{
		if (mLightListDirty)
		{
			getParentSceneNode()->getCreator()->_populateLightList(mCenter, mBoundingRadius, mLightList);
			mLightListDirty = false;
		}
		return mLightList;
	};

}