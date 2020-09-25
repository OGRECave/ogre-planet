#ifndef __PLANET_LOGGER__
#define __PLANET_LOGGER__

#include "OgrePrerequisites.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"


namespace OgrePlanet
{

	/** Quick and dirty #define for ogre logging
	 */
#define LOG( x ) Ogre::LogManager::getSingleton().logMessage(x)
#ifdef _DEBUG 
	#define DEBUG_LOG( x ) Ogre::LogManager::getSingleton().logMessage(x) 
#else 
	#define DEBUG_LOG(x) // Nothing (x)
#endif
	
	
	/** Quick and dirty string conversion for bounding box
	 */
	
	class Vector3Int;
	class QuadBounds;

	class StringConverterEx
	{
	public:
		static Ogre::String toString(const Vector3Int &v);
		static Ogre::String toString(const QuadBounds &qb);
	};


	/** Quick and dirty for basic type to string conversion for logging
	   something that seems to be missing from ogre::StringConverter ... 
	  */
	template <class X>
	std::string StringOf(X object)
	{
		std::ostringstream os;
		os << object;
		return(os.str());
	};	

} // namespace
#endif
