#include "OgreImage.h"
#include "OgreColourValue.h"

#include "PlanetLut.h"
#include "PlanetLogger.h"

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

	Lut::Lut(const Image& img)
	{ 		
		// Copy image data to array
		mLutArray = img;
	};

	
	Lut Lut::createLut(const String &name, const String &group)
	{
		// Load the image
		Image img;
		img.load(name, group);
		

		// Assert that the image loaded  TODO throw exceptions
		if (img.getData() == NULL)
		{
			LOG("Lut::createLut() " + name + ", " + group + " did not load");
		} 
		else if (img.getSize() == 0)
		{
			LOG("Lut::createLut() " + name + ", " + group + " did not load");
		} 
		else if(img.getFormat() != PF_A8R8G8B8)
		{
			LOG("Lut::createLut() " + name + ", " + group + " has no alpha channel");
		}
		else
		{
			// TODO throw exception if bad load
			LOG("Lut::createLut() " + name + ", " + group + " loaded sucessfully");
		}

		// Create new Lut
		return Lut(img);
	};

	
	uint32 Lut::lookup(const Vector2 &xy) const
	{
		// Range check (0..1)
		assert((xy.x >= 0) && (xy.x <= 1.0));
		assert((xy.y >= 0) && (xy.y <= 1.0));

		const uint32 xLook = (uint32) float((mLutArray.getWidth() - 1) * xy.x);
		const uint32 yLook = (uint32) float((mLutArray.getHeight() - 1) * xy.y);
		uint32 ret;
		memcpy(&ret, mLutArray.getData(xLook, yLook), sizeof(uint32));
		return ret;
	};
	
	
	uint32 Lut::lookup(const Real x, const Real y) const
	{
		return lookup(Vector2(x, y));
	};

	
	void Lut::lookup(const Real x, const Real y, ColourValue & colour) const
	{		
		lookup(Vector2(x, y), colour);
	};

	
	void Lut::lookup(const Vector2 &xy, ColourValue &colour) const
	{
		uint32 colour32 = lookup(xy);
		PixelUtil::unpackColour(&colour, mLutArray.getFormat(), &colour32);
	};

} // namespace
