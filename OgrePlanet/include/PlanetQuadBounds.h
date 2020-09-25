#ifndef __PLANET_QUAD_BOUNDS__
#define __PLANET_QUAD_BOUNDS__

#include "OgrePrerequisites.h"
#include "OgreVector3.h"
#include "OgreAxisAlignedBox.h"

#include "PlanetVector3Int.h"
#include "PlanetLogger.h"

namespace OgrePlanet
{

	using namespace Ogre;
	

	/** Face of cube whence this QuadBounds belongs
	 * Encapsulates all the work associated with different winding orders of faces
	 *
	 *    UP
	 * LF FR RT
	 *    DN
	 *    BK
	 */
	enum QuadFace
	{
		QuadFace_begin = 0,
		QF_FR = QuadFace_begin, 
		QF_BK, QF_LF, QF_RT, QF_UP, QF_DN,  // Front, back, left, right, top, bottom
		QuadFace_end
	};
	QuadFace &operator++ (QuadFace &face);
	String &toString(QuadFace &face);
	

	/** Bounds for a given QuadNode points must be square and coplainar
	 * Note that due to constraints regarding winding order and a convex shape the local coordinate system in the quad is messy
	 * Note that this is all long ints to prevent rounding errors
	 * bounding box lengths should all be powers of two
	 *
	 * This encapsulates as much of the hassle as possible
	 * Note that to make life more easy, QF_BK is flipped both horizontally and vertically
	 */
	class QuadBounds
	{
	public:
		Vector3Int a, b, c, d;
		QuadFace face;
	public:
		QuadBounds() : face(QF_FR) { };
		QuadBounds(const Vector3Int _a, const Vector3Int _b, const Vector3Int _c, const Vector3Int _d,
			const QuadFace _face) : a(_a), b(_b), c(_c), d(_d), face(_face) { };

		// Copy constructor - should be compiler generated anyway...
		//QuadBounds(QuadBounds const &bounds) :
		//a(bounds.a), b(bounds.b), c(bounds.c), d(bounds.d), face(bounds.face) { };


		// Given top left corner and width, populate other elements
		void generate(const long stride)
		{
			/* -,-
			 *     dc
			 *     ab
			 *        +,+
			 */


			Vector3Int width, height;
			if ((face == QF_FR) || (face == QF_BK))
			{
				// xy plane
				width.x = stride;
				height.y = ((face == QF_BK) ? stride : (-(stride)));
			}
			else if ((face == QF_LF)||(face == QF_RT))
			{
				// yz plane				
				height.y = (-(stride));
				width.z  = ((face == QF_LF) ? stride : (-(stride)));
			}
			else // assumed ((face == QF_UP)||(face == QF_DN))
			{
				// xz plane
				width.x = stride;
				height.z = ((face == QF_UP) ? stride : (-(stride)));
			}								
			a = d + height;
			b = a + width;
			c = d + width;
		};

		
		const Vector3 getDrawOrigin() const
		{
			return Vector3(d.x, d.y, d.z);
		};

		
		inline const long getStrideX() const
		{
			Vector3Int min(minX(), minY(), minZ());
			Vector3Int max(maxX(), maxY(), maxZ());
			if ((face == QF_FR) || (face == QF_BK) || (face == QF_DN) || (face == QF_UP))
			{
				return max.x - min.x;
			}
			else return 0;  // no x component
		};


		inline const long getStrideY() const
		{
			Vector3Int min(minX(), minY(), minZ());
			Vector3Int max(maxX(), maxY(), maxZ());
			if ((face == QF_BK))
			{
				return max.y - min.y;
			}
			else if ((face == QF_LF) || (face == QF_RT) || (face == QF_FR))
			{
				return min.y - max.y;
			}
			else return 0; // no y component
		};


		inline const long getStrideZ() const
		{
			Vector3Int min(minX(), minY(), minZ());
			Vector3Int max(maxX(), maxY(), maxZ());
			if ((face == QF_LF) || (face == QF_UP))
			{
				return max.z - min.z;
			}
			else if ((face == QF_DN) || (face == QF_RT))
			{
				return min.z - max.z;
			}
			else return 0;  // no z component
		};
		
		inline const Vector3 getCenter() const
		{
			Vector3Int center = (a+b+c+d);
			return Vector3(center.x/4, center.y/4, center.z/4);
		};
		
		// Take a copy of passed data
		inline QuadBounds& operator = (const QuadBounds &in)
        {
			a = in.a;
			b = in.b; 
			c = in.c;
			d = in.d;
			face = in.face;
			return *this;
		};
		
		/// Create an object that is passed to QuadNode constructor initaliser list
		static QuadBounds parent(const long radius, const QuadFace _face)
		{
		
			// Dont mess with winding order - no really ...
			if (_face == QF_FR)
			{
				return QuadBounds(					 
					Vector3Int(-radius, -radius,  radius), 
					Vector3Int( radius, -radius,  radius), 					
					Vector3Int( radius,  radius,  radius), 
					Vector3Int(-radius,  radius,  radius),
					_face);
			}
			else if (_face == QF_BK)
			{			
				return QuadBounds(					
					Vector3Int(-radius,  radius, -radius), 
					Vector3Int( radius,  radius, -radius), 				
					Vector3Int( radius, -radius, -radius), 				
					Vector3Int(-radius, -radius, -radius), 
					_face);
			}			
			else if (_face == QF_LF)
			{				
				return QuadBounds(																				
					Vector3Int(-radius, -radius, -radius), 
					Vector3Int(-radius, -radius,  radius), 
					Vector3Int(-radius,  radius,  radius), 
					Vector3Int(-radius,  radius, -radius), 					
					_face);
			}
			else if (_face == QF_RT)
			{				
				return QuadBounds(																																																											
					Vector3Int(radius, -radius,  radius), 
					Vector3Int(radius, -radius, -radius), 					
					Vector3Int(radius,  radius, -radius), 
					Vector3Int(radius,  radius,  radius), 
					_face);

			}
			else if (_face == QF_UP)
			{
				return QuadBounds(					
					Vector3Int(-radius, radius,  radius),
					Vector3Int( radius, radius,  radius), 
					Vector3Int( radius, radius, -radius),				
					Vector3Int(-radius, radius, -radius), 
					_face);
			}
			else // assumed (_face == QF_DN)
			{
				return QuadBounds(
					Vector3Int(-radius, -radius, -radius), 				
					Vector3Int( radius, -radius, -radius), 
					Vector3Int( radius, -radius,  radius), 				
					Vector3Int(-radius, -radius,  radius), 
					_face);
			}
		};
	
		
		void inline getSplit(QuadBounds &nw, QuadBounds &sw, QuadBounds &se, QuadBounds &ne, const long stride) const
		{
			// Local coordinate system is different for each face type
			// This is due to convex shape of cube and required winding order
			nw.d = d;
			nw.generate(stride);

			sw.d = nw.a;
			sw.generate(stride);

			ne.d = nw.c;
			ne.generate(stride);

			se.d = nw.b;
			se.generate(stride);
		};


		/** Spherize the points
		    Note: Can only be done *after* any subdivision et al.
		*/
		void inline spherise(const long radius)
		{
			a.spherise(radius);
			b.spherise(radius);
			c.spherise(radius);
			d.spherise(radius);
		};
		
			
		/** Get bounding plane for this face
		 * Note lenghts are as follows for different faces of the cube:
		 * QF_UP|QF_DN x=z, y=0
		 * QF_FR|QF_BK x=y, z=0
		 * QF_LF|QF_RT y=z, x=0
		 */	
		AxisAlignedBox getPlane() const
		{	
			return AxisAlignedBox(Vector3(minX(), minY(), minZ()), Vector3(maxX(), maxY(), maxZ()));
		};

		
		inline long minX() const
		{
			long min = a.x;
			min = (b.x < min) ? b.x : min;
			min = (c.x < min) ? c.x : min;
			min = (d.x < min) ? d.x : min;			
			return min;
		};

		
		inline long minY() const
		{
			long min = a.y;
			min = (b.y < min) ? b.y : min;
			min = (c.y < min) ? c.y : min;
			min = (d.y < min) ? d.y : min;			
			return min;
		};

		
		inline long minZ() const
		{
			long min = a.z;
			min = (b.z < min) ? b.z : min;
			min = (c.z < min) ? c.z : min;
			min = (d.z < min) ? d.z : min;			
			return min;
		};

		
		inline long maxX() const
		{
			long max = a.x;
			max = (b.x > max) ? b.x : max;
			max = (c.x > max) ? c.x : max;
			max = (d.x > max) ? d.x : max;			
			return max;
		};

		
		inline long maxY() const
		{
			long max = a.y;
			max = (b.y > max) ? b.y : max;
			max = (c.y > max) ? c.y : max;
			max = (d.y > max) ? d.y : max;			
			return max;
		};

		
		inline long maxZ() const
		{
			long max = a.z;
			max = (b.z > max) ? b.z : max;
			max = (c.z > max) ? c.z : max;
			max = (d.z > max) ? d.z : max;			
			return max;
		};

	}; // class QuadBounds

} // namespace
#endif
