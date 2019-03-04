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

	Lut::Lut(const Image *img, const uint32 width, const uint32 height) : 
	mLutArray(boost::extents[width][height]), mWidth(width), mHeight(height)
	{ 		
		// Copy image data to array
		const PixelBox &pb = img->getPixelBox();
		for (uint32 y=0; y<mHeight; y++)
		{
			uint32 *data = reinterpret_cast<uint32 *>(pb.data) + y*pb.rowPitch;
			for (uint32 x=0; x<mWidth; x++)
			{	
				// Set the colour components properly
				const uint32 pixel = *data++;
				mLutArray[x][y] = pixel; 
			}
		}
	};

	
	Lut Lut::createLut(const String &name, const String &group)
	{
		// Load the image
		Image *img = new Image();
		img->load(name, group);
		

		// Assert that the image loaded  TODO throw exceptions
		if (img == NULL)
		{
			LOG("Lut::createLut() " + name + ", " + group + " did not load");
		} 
		else if (img->getSize() == 0)
		{
			LOG("Lut::createLut() " + name + ", " + group + " did not load");
		} 
		else if(img->getFormat() != PF_A8R8G8B8)
		{
			LOG("Lut::createLut() " + name + ", " + group + " has no alpha channel");
		}
		else
		{
			// TODO throw exception if bad load
			LOG("Lut::createLut() " + name + ", " + group + " loaded sucessfully");
		}

		// Create new Lut
		Lut lut(img, img->getWidth(), img->getHeight());

		// Clean up image 
		delete img;
		img = NULL;

		return lut;
	};

	
	uint32 Lut::lookup(const Vector2 &xy) const
	{
		// Range check (0..1)
		assert((xy.x >= 0) && (xy.x <= 1.0));
		assert((xy.y >= 0) && (xy.y <= 1.0));
		
		const uint32 xLook = (uint32)float((mWidth-1) * xy.x);
		const uint32 yLook = (uint32)float((mHeight-1) * xy.y);
		return mLutArray[xLook][yLook];
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
		uint32ToColour(colour32, colour);
	};

		
	void Lut::uint32ToColour(const uint32 colour32, ColourValue &colour) const
	{
		// Handle endianness
#if OGRE_ENDIAN == OGRE_ENDIAN_LITTLE
		// ARGB
		uchar cA = uchar((colour32 & 0xFF000000) >> 24);
		uchar cR = uchar((colour32 & 0x00FF0000) >> 16);
		uchar cG = uchar((colour32 & 0x0000FF00) >> 8);
		uchar cB = uchar((colour32 & 0x000000FF));
#elif OGRE_ENDIAN == OGRE_ENDIAN_BIG
		// BGRA
		uchar cB = uchar((colour32 & 0xFF000000) >> 24);
		uchar cG = uchar((colour32 & 0x00FF0000) >> 16);
		uchar cR = uchar((colour32 & 0x0000FF00) >> 8);
		uchar cA = uchar((colour32 & 0x000000FF));
#endif
		
		/*
		// Handle endianness
#if OGRE_ENDIAN == OGRE_ENDIAN_LITTLE
		// ARGB
		uchar cA = uchar((colour32 & 0xFF000000) >> 24);
		uchar cR = uchar((colour32 & 0x00FF0000) >> 16);
		uchar cG = uchar((colour32 & 0x0000FF00) >> 8);
		uchar cB = uchar((colour32 & 0x000000FF));
#elif OGRE_ENDIAN == OGRE_ENDIAN_BIG
		// BGRA
		uchar cB = uchar((colour32 & 0xFF000000) >> 24);
		uchar cG = uchar((colour32 & 0x00FF0000) >> 16);
		uchar cR = uchar((colour32 & 0x0000FF00) >> 8);
		uchar cA = uchar((colour32 & 0x000000FF));
#endif
		*/

		// Store resultant colour value
		colour.a = float(cA) / 255;
		colour.r = float(cR) / 255;
		colour.g = float(cG) / 255;
		colour.b = float(cB) / 255;
	};


} // namespace
