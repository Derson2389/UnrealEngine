//
// paImpact.hpp
//
// Describes the state of a continuous contact between two acoustic bodies.
//


#if !defined(__paImpact_hpp)
#define __paImpact_hpp

#include "System/paConfig.h"
#include "Utility/paObjectPool.hpp"
#include "Impact/paImpactGen.hpp"
#include "Impact/paImpactDynamicDataAPI.h"
#include "Scene/paFlags.hpp"
class paSurface;
class paRes;
class paBody;

template<class paImpact> class PHYA_API paObjectPool;

class PHYA_API paImpact
{
private:

	friend PHYA_API paBlock* paTick(void);		// Main audio tick function which updates the resonators and surface generators.

	paBody* m_body1;
	paBody* m_body2;

	paRes* m_resonator1;
	paRes* m_resonator2;

	paSurface* m_surface1;			// Used to provide a surface type for
	paSurface* m_surface2;			// freeing impactGens when the contact is freed.

	paImpactGen* m_impactGen1;
	paImpactGen* m_impactGen2;

	paImpactDynamicData m_dynamicData;


	// Impact couplings:
	// These are copied from the owning paBody, and can be altered.

	paFloat m_surface1ImpactCrossGain;
	paFloat m_surface2ImpactCrossGain;

	paFloat m_surface1ImpactDirectGain;
	paFloat m_surface2ImpactDirectGain;

	bool m_isReady;			// Used to prevent the audiothread processing the impact before any parameters have been set.

	int m_poolHandle;		// Handle used by the scene contact manager to release the contact.

	void* m_userData;

public:

	static paObjectPool<paImpact> pool;

	paImpact();

	int initialize();		// After taking from pool.
	int terminate();		// Before releasing back to pool.

	static paImpact* newImpact();
	static int deleteImpact(paImpact*);

	// Assign acoustic bodies to the contact.
	// Atleast one body should be set otherwise no sound will be generated by the contact!

	int setBody1(paBody*);
	int setBody2(paBody*);

	paBody* getBody1() { return m_body1; }
	paBody* getBody2() { return m_body2; }
	paRes* getRes1() { return m_resonator1; }
	paRes* getRes2() { return m_resonator2; }
	paSurface* getSurface1() { return m_surface1; }
	paSurface* getSurface2() { return m_surface2; }
	paImpactGen* getGen1() { return m_impactGen1; }
	paImpactGen* getGen2() { return m_impactGen2; }


	// Coupling set functions. Use straight after assigning bodies.
	// These couplings overide the coupling setting in the assigned bodies.

	// Master gain factors used at impact generation source.
	int setSurface1ImpactGain(paFloat s) {
		paAssert( m_impactGen1 && "Body1 has not been assigned with 'setBody1()' " );
		m_impactGen1->setImpactGain(s);
		return 0;
	}

	int setSurface2ImpactGain(paFloat s) {
		paAssert( m_impactGen2 && "Body2 has not been assigned with 'setBody2()' " );
		m_impactGen2->setImpactGain(s);
		return 0;
	}

	int setSurface1ImpactAmpMax(paFloat s) {
		paAssert( m_impactGen1 && "Body1 has not been assigned with 'setBody1()' " );
		m_impactGen1->setAmpMax(s);
		return 0;
	}

	int setSurface2ImpactAmpMax(paFloat s) {
		paAssert( m_impactGen2 && "Body2 has not been assigned with 'setBody2()' " );
		m_impactGen2->setAmpMax(s);
		return 0;
	}


	// These gain factors used in paTick
	int setSurface1ImpactCrossGain(paFloat s) { m_surface1ImpactCrossGain = s; return 0; }
	int setSurface2ImpactCrossGain(paFloat s) { m_surface2ImpactCrossGain = s; return 0;}

	int setSurface1ImpactDirectGain(paFloat s) { m_surface1ImpactDirectGain = s; return 0; }
	int setSurface2ImpactDirectGain(paFloat s) { m_surface2ImpactDirectGain = s; return 0; }

/*
	// Skid coupling. This scales the mixing to resonator1, resonator2 and direct output.
	// The balance between these mixes is as for impact.

	int setSurface1SkidContactGain(paFloat s) {
		paAssert( ("Body1 has not been assigned with 'setBody1()' ", m_impactGen1) );
		m_impactGen1->setSkidContactGain(s);
		return 0;
	}

	int setSurface2SkidContactGain(paFloat s) {
		paAssert(("Body2 has not been assigned with 'setBody2()' ", m_impactGen2));
		m_impactGen2->setSkidContactGain(s);
		return 0;
	}
*/

	// Regular dynamic data update.

	int setDynamicData(paImpactDynamicData* d);

	int getDyanmicDataIsSet() { return m_isReady; }

	void* getUserData() { return m_userData; }
	int setUserData(void* d) { m_userData = d; return 0; }

};

#endif
