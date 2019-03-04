#ifndef __PLANET_LUT_GENERATOR_
#define __PLANET_LUT_GENERATOR_

#include <boost/multi_array.hpp>

#include "OgrePrerequisites.h"

#include "PlanetPerlin.h"

namespace OgrePlanet
{

	// TODO pull this out to C++ file for build speed

	using namespace Ogre;

	/** Build a lookup table which blends between layers, mix in some perlin noise.
		This lookup table must then be saved to disk and loaded to a Lut object.
	    Channels R & G are chained (sand+grass)=1
		Channels G & B are chained (grass+rock)=1
		Channels B & A are chained (rock+snow)=1
		The start, peaks and endpoints of each arc can be moved
	*/

	class LutGenerator
	{
	public:
		/** Constructor
			@param texStride output texture width / height
			@param perlinScale how many times noise repeats <1 less (cloudy), >1 more(grainy)
		*/
		LutGenerator(const uint32 texStride, const float perlinScale = 1.0f) :
		mTexStride(texStride), mPerlinStep(texStride /(texStride * 1 / perlinScale)),
		mColourArray(boost::extents[texStride][texStride])
		{
			assert(perlinScale	> 0);
		};


		/** Make the lookup table
			Note that peaks shouldn't overlap ie (trough.x < peak.x < trough.y < peak.y < trough.z)
			@param peak positions of blending peaks
			@param trough positions of blending troughs
			@param baseSteep interpolate 'base' midpoint to end point vertically on texture if true, else move 'base' midpoint to start point
			@param dirtSteep as baseSteep for 'dirt'
			@param rockSteep as baseSteep for 'rock'
			@param noiseFactor percentage perlin noise - vs cosine interpolation


			base          dirt           rock         snow
			0                                            1
			+------+-------+------+-------+------+-------+--------> plus
			|   trough.x   |   trough.y   |   trough.z   |
			|      /\      |      /\      |      /\      |
			|     /  \     |     /  \     |     /  \     |
			|    /    \    |    /    \    |    /    \    |
			|   /      \   |   /      \   |   /      \   |
			|  /        \  |  /        \  |  /        \  |
			| /          \ | /          \ | /          \ |
			|/            \|/            \|/            \|
			0            peak.x         peak.y           1
		*/
		void generate(const Vector2 &peak, const Vector3 &trough, 
			const bool baseSteep, const bool dirtSteep,	const bool rockSteep, 
			const float noiseFactor = 0.5f)
		{		
			
			// Sanity check inputs
			assertOhToOne(peak);
			assertOhToOne(trough);

			// Create the three crests
			Vector3 baseDirt(0, trough.x, peak.x);
			Vector3 dirtRock(peak.x, trough.y, peak.y);
			Vector3 rockSnow(peak.y, trough.z, 1);

			// Zero array - default is (1, 1, 1, 1)
			for (uint32 x=0; x<mTexStride; x++)
			{
				for (uint32 y=0; y<mTexStride; y++)
				{
					mColourArray[x][y] = ColourValue(0, 0, 0, 0);
				}
			}

			// Set up perlin noise generator
			mPerlinNoise.setNumOctaves(20);
			mPerlinNoise.setPersistence(0.5f);
			mPerlinNoise.randomise();

			// Make the passes
			pass(baseDirt, CH_RED_GREEN, baseSteep, noiseFactor);
			pass(dirtRock, CH_GREEN_BLUE, dirtSteep, noiseFactor);
			pass(rockSnow, CH_BLUE_ALPHA, rockSteep, noiseFactor);

			// Fill any blanks with 'sand'
			finalise();
		};
		

		/// Save Lut to disk
		void save(const String &fileName)
		{
			// Copy contents of mColourArray to new char array
			// Note: will be cleaned up when Image destructor called as 'autoDelete = true'
			uchar *dump;
			dump = new uchar[mTexStride*mTexStride*4];
			uchar *dumpPtr = dump;
			for (uint32 y=0; y<mTexStride; y++)
			{
				for (uint32 x=0; x<mTexStride; x++)
				{
#if  OGRE_ENDIAN == OGRE_ENDIAN_LITTLE
					*dumpPtr++ = uchar(mColourArray[x][y].b * 255);
					*dumpPtr++ = uchar(mColourArray[x][y].g * 255);
					*dumpPtr++ = uchar(mColourArray[x][y].r * 255);
					*dumpPtr++ = uchar(mColourArray[x][y].a * 255);	
#elif  OGRE_ENDIAN == OGRE_ENDIAN_BIG
					// XXX Not tested					
					*dumpPtr++ = uchar(mColourArray[x][y].a * 255);	
					*dumpPtr++ = uchar(mColourArray[x][y].r * 255);
					*dumpPtr++ = uchar(mColourArray[x][y].g * 255);
					*dumpPtr++ = uchar(mColourArray[x][y].b * 255);
#endif
				}
			}

			Image img;
			img.loadDynamicImage(dump, mTexStride, mTexStride, 1, PF_A8R8G8B8, true);
			img.save(fileName);
		};

	private:
		enum Channel
		{
			CH_BLUE_ALPHA = 0, CH_RED_GREEN, CH_GREEN_BLUE
		};

		
		void assertOhToOne(const float val) const
		{
			assert((val >=0)&&(val<=1.0f));
		};

		
		void assertOhToOne(const Vector3 &val) const
		{
			assertOhToOne(val.x);
			assertOhToOne(val.y);	
			assertOhToOne(val.z);
		};

		
		void assertOhToOne(const Vector2 &val) const
		{
			assertOhToOne(val.x);
			assertOhToOne(val.y);	
		};


		void write(const ColourValue colour, const uint32 x, const uint32 y)
		{
			// A+R+G+B must = 1, channel not empty - halve all values
			// XXX TODO Doesn't work well - another technique required ?		
			if (!empty(x, y))
			{
				mColourArray[x][y] += colour;
				mColourArray[x][y] *= 0.5f;
			}
			else
			{
				mColourArray[x][y] = colour;
			}
		};

		
		bool empty(const uint32 x, const uint32 y)
		{
			return ((mColourArray[x][y].a == 0) && (mColourArray[x][y].r == 0)
				 && (mColourArray[x][y].g == 0) && (mColourArray[x][y].b == 0));
		};

		
		void makeColour(ColourValue &colour, const Channel channel, const float first, const float second) const
		{
			switch(channel)
			{
			case CH_BLUE_ALPHA:
				colour.r = 0;
				colour.g = 0;
				colour.b = first;
				colour.a = second;
				break;
			case CH_RED_GREEN:
				colour.r = first;
				colour.g = second;
				colour.b = 0;
				colour.a = 0;
				break;
			case CH_GREEN_BLUE:
				colour.r = 0;
				colour.g = first;
				colour.b = second;
				colour.a = 0;
				break;
			default:
				colour.r = first;
				colour.g = second;
				colour.b = 0;
				colour.a = 0;
				break;
			}
		};

		
		void finalise()
		{
			// Anything zero gets full sand
			for (uint32 x=0; x<mTexStride; x++)
			{
				for (uint32 y=0; y<mTexStride; y++)
				{
					if (empty(x, y))
					{
						mColourArray[x][y] = ColourValue(1, 0, 0, 0);
					}
				}
			}
		};
		
			
		void pass(const Vector3 &in, const Channel channel, const bool up, const float noiseFactor)
		{		
			// if up == true transition mid point to endpoint
			// else transition mid point to startpoint
			float start = in.x;
			float mid = in.y;
			float end = in.z;

			if (up == true)
			{
				float yStep = (float(end - mid) / float(mTexStride));
				for (uint32 y=0; y<mTexStride; y++)
				{
					float newMid = float(yStep*y + mid);
					Vector3 thisPass(start, newMid, end);
					innerPass(thisPass, channel, y, noiseFactor);
				}
			}
			else
			{
				float yStep = (float(mid - start) / float(mTexStride));
				for (uint32 y=0; y<mTexStride; y++)
				{
					float newMid = float(mid - yStep*y);
					Vector3 thisPass(start, newMid, end);
					innerPass(thisPass, channel, y, noiseFactor);
				}
			}
		};
		
		
		void innerPass(const Vector3 &in, const Channel channel, const uint32 y, const float noiseFactor)
		{
			// x = height, y = slope
			// Interpolate via sin from start to end
			uint32 start = (uint32)float(in.x*mTexStride);
			uint32 mid = (uint32)float(in.y*mTexStride);
			uint32 end = (uint32)float(in.z*mTexStride);
			float xStep = (float (end - start) / float(mid - start)); 
			xStep /=  float (end - start);
			const Real halfPi = Math::PI * 0.5;

			
			float angle = 0;
			for (uint32 x=start; x<mid; x++)
			{			
				// Calculate based on rules (cosine interpolate and supplied points)
				Radian radian = Radian(Real(angle * halfPi));
				float first = (Math::Cos(radian) + 1.0f)* 0.5f;  // (-1..1) -> (0..1)
				
				// Other channel is complement of result
				float second = 1.0f - first;

				// Mix in a touch of perlin noise
				float perlin = mPerlinNoise.getNormalizedNoise(float(x)*mPerlinStep, float(y)*mPerlinStep);
				float ruleFactor = 1.0f - noiseFactor;
				first = ruleFactor * first + noiseFactor * perlin;
				second = ruleFactor * second + noiseFactor * perlin;
				float sum = first+second;
				first /= sum;
				second /= sum;

				// Store calculated colour
				ColourValue colour;
				makeColour(colour, channel, first, second);
				write(colour, x, y);
				angle += xStep;
			}

			xStep = (float (end - start) / float(end - mid));
			xStep /= float(end - start);
			angle = 0;
			for (uint32 x=mid; x<end; x++)
			{			
				// Calculate based on rules (cosine interpolate and supplied points)
				Radian radian = Radian(Real(angle * halfPi + halfPi));
				float first = (Math::Cos(radian) + 1.0f)* 0.5f; // (-1..1) -> (0..1)

				// Other channel is complement of result		
				float second = 1.0f - first;


				// Mix in a touch of perlin noise
				float perlin = mPerlinNoise.getNormalizedNoise(float(x)*mPerlinStep, float(y)*mPerlinStep);
				float ruleFactor = 1.0f - noiseFactor;
				first = ruleFactor * first + noiseFactor * perlin;
				second = ruleFactor * second + noiseFactor * perlin;
				float sum = first+second;
				first /= sum;
				second /= sum;

				// Store calculated colour
				ColourValue colour;
				makeColour(colour, channel, first, second);
				write(colour, x, y);

				angle += xStep;
			}

		};

		const uint32 mTexStride;  // (x, y) stride of texture
		typedef boost::multi_array<ColourValue, 2> ColourArray;
		ColourArray mColourArray; // Texture made up of colour values
		PerlinNoise mPerlinNoise; // Perlin noise generator
		const float mPerlinStep;  // Scaling factor for perlin noise lookup
	};
} // namespace
#endif
