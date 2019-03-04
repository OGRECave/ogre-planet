#ifndef __PLANET_PERLIN__
#define __PLANET_PERLIN__

#include "OgrePrerequisites.h"



namespace OgrePlanet
{

	using namespace Ogre;

	// Heavily based on work/tutorials by Hugo Elias
	// http://freespace.virgin.net/hugo.elias/models/m_perlin.htm
	class PrimeGenerator
	{
	public:
		PrimeGenerator() { };

		/// Get the next prime from start inclusive
		const uint32 getPrime(const uint32 start) const
		{
			uint32 i = ((start > 1) ? start : 2);
			while(!isPrime(i))
			{
				i++;
			}
			return i;
		};

		/// Get the next n primes from start inclusive and store in primes
		void getPrimes(std::vector<float> &primes, const uint32 start, uint32 num) const
		{
			// Find primes from start up
			uint32 i = ((start > 1) ? start : 2);
			uint32 count = 0;		
			while ((count < num) && (i<0xFFFFFFFF))
			{
				if (isPrime(i))
				{
					count++;
					primes.push_back(i);
				}
				i++;
			}
			

			// If ran out of int space
			i = 2;
			while (count < num)
			{
				if (isPrime(i))
				{
					count++;
					primes.push_back(i);
				}
				i++;
			}
		};
	private:
		inline uint32 sqrt(const uint32 i) const
		{
			uint32 r = 0;
			uint32 rnew = 1;
			uint32 rold = r;
			do
			{
				rold = r;
				r = rnew;
				rnew = (r+(i/r));
				rnew >>= 1;
			}
			while(rold != rnew);
			return rnew;
		};

		inline bool isPrime(const uint32 i) const
		{
			uint32 si;

			si = sqrt(i);

			for (uint32 j=2; (j <= si); j++)
			{
				if (i%j == 0)
					return false;
			}

			return true;
		};
	};


	// Heavily based on work/tutorials by Hugo Elias
	// http://freespace.virgin.net/hugo.elias/models/m_perlin.htm
	class PerlinNoise
	{
	public:
		PerlinNoise() : mPersistence(1), mNumOctaves(1), 	
		// Hard coding for 'non random' results - randomise with randomizse()
		mPrime0(15731), mPrime1(789221), mPrime2(1376312589)
		{ 
			
		};
		
		void setPersistence(const float persistence)
		{
			mPersistence = persistence;
		};

		void setNumOctaves(const uint32 numOctaves)
		{
			mNumOctaves = numOctaves;
		};

		/// Ranomise seed primes
		void randomise()
		{
			// Set up primes by getting next prime based on a random number with the correct number of digits
			mPrime0 = mPrimeGenerator.getPrime(uint32(Math::RangeRandom(10000, 100000)));
			mPrime1 = mPrimeGenerator.getPrime(uint32(Math::RangeRandom(100000, 1000000)));
			mPrime2 = mPrimeGenerator.getPrime(uint32(Math::RangeRandom(1000000000, 100000000000)));
		};

		/// Get the value of perlin noise at x, y
		float getNoise(const float x, const float y) const
		{
			float total = 0.0f;	  
			for(uint32 i=0; i<mNumOctaves-1; i++)
			{
				long frequency = Math::Pow(2,i);
				float amplitude = Math::Pow(mPersistence, i);
				total += interpolatedNoise(x*frequency, y*frequency) * amplitude;			
			}    
			return total;
		};

		/// Get the value of perlin noise at x, y normalized (0..1)
		float getNormalizedNoise(const float x, const float y) const
		{
			
			float noiseVal = getNoise(x, y);
			
			// Values tend to swing back and fourth close to zero
			noiseVal += 0.5f;
			if (noiseVal < 0)
			{
				noiseVal = 0;
			}
			else if (noiseVal > 1.0f)
			{
				noiseVal = 1.0f;
			}

			return noiseVal;
		};


	private:
		static const uint32 NUM_PRIMES = 4;
		/// Psuedo random noise with gaussian distribution same x, y = same out
		float noise(const long x, const long y) const
		{
			long iPart = x + y * 57;
			iPart = (iPart<<13) ^ iPart;		
			iPart = (iPart * (iPart * iPart * mPrime0 + mPrime1) + mPrime2) & 0x7fffffff;
			float retVal = (1.0f - (iPart) / 1073741824.0f); // 1.0f - 0x40000000
			return retVal;
		};

		/// Filter to smooth noise values
		float smoothNoise(const long x, const long y) const
		{
			float corners = (noise(x-1, y-1) + noise(x+1, y-1) + noise(x-1, y+1) + noise(x+1, y+1)) / 16.0f;
			float sides   = (noise(x-1, y) + noise(x+1, y) + noise(x, y-1) + noise(x, y+1)) /  8.0f;
			float center  = noise(x, y) / 4.0f;
			return (corners + sides + center);
		};

		
		/// Cosine interpolation between two points - could use linear interpolation for speed
		float interpolate(const float a, const float b, const float x) const
		{		
			float ft = x * Math::PI;
			float f = (1.0f - Math::Cos(ft)) * 0.5;
			return  a*(1.0f-f) + b*f;
		};
		
		/// Sample and filter noise
		float interpolatedNoise(const float x, const float y) const
		{
			long intX = long(x);     // Integer part
			float fracX = x - intX;  // Factional part

			long intY = long(y);
			float fracY = y - intY;

			float v1 = smoothNoise(intX,   intY);
			float v2 = smoothNoise(intX+1, intY);
			float v3 = smoothNoise(intX,   intY+1);
			float v4 = smoothNoise(intX+1, intY+1);

			float i1 = interpolate(v1 , v2 , fracX);
			float i2 = interpolate(v3 , v4 , fracX);

			return interpolate(i1 , i2 , fracY);
		};
		
		float mPersistence;
		uint32 mNumOctaves;
		PrimeGenerator mPrimeGenerator;
		uint32 mPrime0;
		uint32 mPrime1; 
		uint32 mPrime2;	
	};
} // namespace
#endif
