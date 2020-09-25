#ifndef __PLANET_VECTOR3INT__
#define __PLANET_VECTOR3INT__


#include "OgrePrerequisites.h"
#include "OgreVector3.h"

namespace OgrePlanet
{

	using namespace Ogre;

	
	/** Convience structure for an x, y pair of long ints
	 * Used to prevent rounding errors - must be even numbers
	 */
	class Vector3Int
	{
	// TODO flesh out operators
	public:
		long x, y, z;
	public:
		Vector3Int() : x(0), y(0), z(0) { };
		Vector3Int(const long _x, const long _y, const long _z) : x(_x), y(_y), z(_z) { };
		
		inline Vector3 toVector3() const
		{
			return Vector3(Real(x), Real(y), Real(z));
		};		

		inline Vector3Int operator - ( const Vector3Int& v ) const
        {
			return Vector3Int(x - v.x, y - v.y, z - v.z);
		};
		
		inline Vector3Int operator + ( const Vector3Int& v ) const
        {
			return Vector3Int(x + v.x, y + v.y, z + v.z);
		};

		inline Vector3Int& halve()
		{
			x >>= 1;
			y >>= 1;
			z >>= 1;
			return *this;
		};

		inline Vector3Int operator * ( const long s ) const
        {
             return Vector3Int(x * s, y * s, z * s);
        };

		inline Vector3Int& operator += ( const Vector3Int& v )
        {
			x += v.x;
            y += v.y;
			z += v.z;
            return *this;
        };

		inline Vector3Int& operator -= ( const Vector3Int& v )
        {
			x -= v.x;
            y -= v.y;
			z -= v.z;
            return *this;
        };

		inline Vector3Int& operator = (const Vector3Int& v)
		{
			x = v.x;
			y = v.y;
			z = v.z;
			return *this;
		};

		inline bool operator == ( const Vector3Int& v ) const
        {
			return ( x == v.x && y == v.y && z == v.z);
        };
 
		inline bool operator != ( const Vector3Int& v ) const
        {
			return ( x != v.x || y != v.y || z != v.z );
        };
		
		inline void spherise(const long radius)
		{
			// http://mathproofs.blogspot.com/2005/07/mapping-cube-to-sphere.html
			// Convert 3D point on surface of cube to point on surface of sphere
			// Normalisation (v / v.length() * radius) doesn't work well
			// Actually a 'smash -n- grab' from HexiDave Planet2 code

			double invRadius = double(double(1)/double(radius));

			// Down to unit cube
			double newX = double(x) * invRadius;
			double newY = double(y) * invRadius;
			double newZ = double(z) * invRadius;

			// Calc values used more than once
			const double dX2 = newX * newX;
			const double dY2 = newY * newY;
			const double dZ2 = newZ * newZ;

			const double dX2Half = dX2 * 0.5;
			const double dY2Half = dY2 * 0.5;
			const double dZ2Half = dZ2 * 0.5;

			// Spherize
			newX *= Math::Sqrt(1-(dY2Half)-(dZ2Half)+((dY2 * dZ2) * 0.33333333333333333333333333333333));
			newY *= Math::Sqrt(1-(dZ2Half)-(dX2Half)+((dZ2 * dX2) * 0.33333333333333333333333333333333));
			newZ *= Math::Sqrt(1-(dX2Half)-(dY2Half)+((dX2 * dY2) * 0.33333333333333333333333333333333));


			// Scale back up to radius
			newX *= double(radius);
			newY *= double(radius);
			newZ *= double(radius);			
			
			// Store values
			// TODO loose this rounding for speed?
			x = static_cast<long>(newX + ((newX > 0.0) ? 0.5 : -0.5));
			y = static_cast<long>(newY + ((newY > 0.0) ? 0.5 : -0.5));
			z = static_cast<long>(newZ + ((newZ > 0.0) ? 0.5 : -0.5));
		};

	}; // class Vector3Int
} // namespace
#endif