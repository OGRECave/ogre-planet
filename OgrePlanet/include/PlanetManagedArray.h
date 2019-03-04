#ifndef __PLANET_MANAGED_ARRAY__
#define __PLANET_MANAGED_ARRAY__

#include <map>

#include "OgrePlatform.h"


namespace OgrePlanet
{
	using namespace Ogre;
		
	// XXX TODO doxygen me
	/**  A psuedo [0..n-1][0..n-1] array of object pointers
		 Supports add(), remove() and get()
		 Life cycle of objects is managed by array
		 Deleting the managed array calls destructor on all contained objects		 
		 remove() calls object destructor
		 Adding to an element that already exists, deletes the existing object
		 Attempting to add() to an index outside the array throws an assertion failure
		 Attempting to get() an index that does not exist returns NULL

		 TODO Should probably be converted to boost::shared_ptr
	*/
	template<typename T>
	class ManagedArray
	{
	public:
		/** Constructor
			@param stride stride for the internal map (total = stride*stride)
		*/
		ManagedArray(const uint32 stride)
		{
			// Initalise stride for internal map
			mStride = stride+1;
		};

		/// Destructor
		virtual ~ManagedArray()
		{
			clear();
		};
		
		
		/// Add an object to the array, replacing (deleting any contents of the given slot)
		void add(T *t, const uint32 x, const uint32 y)
		{
			uint32 index = xyToKey(x, y);

			// Attempt to find in map
			std::map<uint32, T*>::iterator iter = mData.find(index);
			if (iter != mData.end())
			{
				// Found, delete object already in list
				delete iter->second;
				iter->second = NULL;
				iter->second = t;
				// LOG("Replacing..." +StringOf(x) + " " + StringOf(y));
			}
			else
			{
				// Add object
				// LOG("Inserting..." +StringOf(x) + " " + StringOf(y));
				mData.insert(std::pair<uint32, T*>(index, t));			
			}
		};
		
		
		/// Get a constant reference to an object in the array
		const T* get(const uint32 x, const uint32 y) const
		{
			const uint32 index = xyToKey(x, y);

			// Attempt to find in map (const iterator)
			std::map<uint32, T*>::const_iterator iter = mData.find(index);
			if (iter != mData.end())
			{
				// Found, return object
				return iter->second;
			}
			else
			{
				return NULL;
			}
		};


		/// Get writable access to an object in the array
		T* getWritable(const uint32 x, const uint32 y)
		{
			const uint32 index = xyToKey(x, y);

			// Attempt to find in map
			std::map<uint32, T*>::iterator iter = mData.find(index);
			if (iter != mData.end())
			{
				// Found, return object			
				return iter->second;
			}
			else
			{
				return NULL;
			}	
		};


		/// Delete the object at x, y
		void remove(const uint32 x, const uint32 y)
		{
			uint32 index = xyToKey(x, y);

			// Attempt to find in map
			std::map<uint32, T*>::iterator iter = mData.find(index);
			if (iter != mData.end())
			{
				// Found, delete object and remove entry from map
				delete iter->second;
				mData.erase(iter);
			}
		};

		
		/// Delete every object contained in array
		void clear()
		{			
			std::map<uint32, T*>::iterator iter = mData.begin();
			while(iter != mData.end())
			{				
				delete iter->second;
				iter->second = NULL;				
				++iter;
			}
			mData.clear();
		};
		
		
		/// current object count
		size_t size()
		{
			return mData.size();
		};  
	
	protected:
		std::map<uint32, T *> mData;
		uint32 mStride;
		const uint32 xyToKey(const uint32 x, const uint32 y) const
		{
			if ((x < mStride) && (y < mStride))
			{
				return (y*(mStride-1) + x);
			}
			else
			{
				throw std::out_of_range("ManagedArray() Access outside array");
			}
		};		
	
	private:
		// No copy constructor
		ManagedArray(ManagedArray &rhs);
		ManagedArray& operator =(ManagedArray &rhs);
	};	

} // namespace
#endif
