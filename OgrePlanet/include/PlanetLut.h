#ifndef __PLANET_LUT__
#define __PLANET_LUT__

#include "OgreVector2.h"
#include "OgreImage.h"

namespace OgrePlanet
{
	
	using namespace Ogre;


	/** Loads a lookup table from an image
		Allows any width / height to perform lookups on this table
	*/
	class Lut
	{
	public:		
		static Lut createLut(const String &name, const String &group="General");	
		virtual ~Lut() { };
		uint32 lookup(const Real x, const Real y) const;
		uint32 lookup(const Vector2 &xy) const;		
		void lookup(const Vector2 &xy, ColourValue &colour) const;
		void lookup(const Real x, const Real y, ColourValue &colour) const;		
	protected:
		Image mLutArray;
	private:
		// Creation via static createLut() method
		Lut(const Image& img);

		// No copy constructor
		//Lut(const Lut &rhs);
		Lut &operator=(const Lut &rhs);
	};

} // namespace
#endif
