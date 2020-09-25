#include "OgreAxisAlignedBox.h"

#include "PlanetLogger.h"
#include "PlanetQuadBounds.h"

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

	Ogre::String StringConverterEx::toString(const Vector3Int &v)
	{
		Ogre::String out;
		out = StringOf(v.x) + ", ";
		out += StringOf(v.y) + ", ";
		out += StringOf(v.z);
		return out;
	};

	
	Ogre::String StringConverterEx::toString(const QuadBounds &qb)
	{
		Ogre::String out;
		out = StringConverterEx::toString(qb.a) + " | ";
		out += StringConverterEx::toString(qb.b) + " | ";
		out += StringConverterEx::toString(qb.c) + ": ";
		out += StringConverterEx::toString(qb.d) + ": ";
		out += StringOf(qb.face);
		return out;
	};

} // namespace