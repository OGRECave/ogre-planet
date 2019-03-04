#ifndef __PLANET_STATE_OBJ__
#define __PLANET_STATE_OBJ__

namespace OgrePlanet
{
	/** Basic state management to stop users doing nasty things to objects.
	    Used on top level objects that are complex or that manage child objects.
	*/
	class StateObj
	{	
	public:
		StateObj() { mState = STATE_UNINIT; };
		virtual ~StateObj() { };
	protected:
		enum StateType
		{
			STATE_UNINIT = 0,  // After constructor called
			STATE_PREBUILD,    // Any prebuild steps performed
			STATE_BUILT,	   // build() called
			STATE_READY		   // finalise() called, ready to render
		};
		void setState(StateType state) { mState = state; };
		StateType getState() { return mState; };
	private:
		StateType mState;
	};

} // namespace
#endif