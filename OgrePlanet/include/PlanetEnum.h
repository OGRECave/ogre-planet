#ifndef __PLANET_ENUM__
#define __PLANET_ENUM__

namespace OgrePlanet
{

	/** One way to use enums properly (Stroustrup)
	 */
	template <class Enum>
	Enum & enum_increment(Enum & value, Enum begin, Enum end)
	{
		return value = (value == end) ? begin : Enum(value + 1);
	};
} // namespace
#endif
