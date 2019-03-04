#ifndef __PLANET_VECTOR2UINT__
#define __PLANET_VECTOR2UINT__


#include "OgrePrerequisites.h"
#include "OgreVector2.h"



namespace OgrePlanet
{

	using namespace Ogre;
	
	/** Unsigned integer x / y pair
	*/
	class Vector2UInt 
	{
	public:
		inline Vector2UInt(): x(0),y(0) { };
		inline Vector2UInt(const uint32 ix, const uint32 iy): x(ix), y(iy) { };
		inline Vector2UInt(const Vector2UInt &v): x(v.x), y(v.y) { };
		uint32 x;
		uint32 y;

        inline Vector2UInt& operator = ( const Vector2UInt& v )
        {
            x = v.x;
            y = v.y;
            return *this;
        }

        inline bool operator == ( const Vector2UInt& v ) const
        {
            return ( x == v.x && y == v.y );
        }

        inline bool operator != ( const Vector2UInt& v ) const
        {
            return ( x != v.x || y != v.y  );
        }
    
        inline Vector2UInt operator + ( const Vector2UInt& v ) const
        {
            return Vector2UInt(x + v.x, y + v.y);
        }

        inline Vector2UInt operator - ( const Vector2UInt& v ) const
        {
			// Prevent underflow
            return Vector2UInt(((x - v.x < x) ? x - v.x : 0), ((y - v.y < y) ? (y - v.y) : 0));
        }
	};

} // namespace
#endif
