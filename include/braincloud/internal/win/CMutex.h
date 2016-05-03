#ifndef __CMUTEX_H__
#define __CMUTEX_H__

#include "HGType.h"
#include "CThread.h"

// --------------------------------------------------
// --------------------------------------------------

/**
 * CMutex provides a wrapper for critical sections
 */
class CLASS_DECL(CMutex)
{
private:

	static int m_entry;
	static int m_out;
	
protected:

#if !defined(HG_CONSOLE)
	#ifdef HG_MAC_OS_X
		pthread_mutex_t		m_section;
	#else
		CRITICAL_SECTION  m_section; // The critical section
	#endif
#endif

public:
    inline CMutex()
    {
		#if !defined(HG_CONSOLE)
			#ifdef HG_MAC_OS_X
				pthread_mutexattr_t attr;
				pthread_mutexattr_init(&attr);
				pthread_mutexattr_setprioceiling(&attr, sched_get_priority_max(SCHED_FIFO));
				pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
				pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
				pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
				pthread_mutex_init(&m_section,&attr);
			#else
				InitializeCriticalSectionAndSpinCount(&m_section, 4000);
			#endif
		#endif
    }
    inline CMutex(HGuint spinCount)
    {
		#if !defined(HG_CONSOLE)
			#ifdef HG_MAC_OS_X
				pthread_mutexattr_t attr;
				pthread_mutexattr_init(&attr);
				pthread_mutexattr_setprioceiling(&attr, sched_get_priority_max(SCHED_FIFO));
				pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
				pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
				pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
				pthread_mutex_init(&m_section,&attr);
			#else
				InitializeCriticalSectionAndSpinCount(&m_section, spinCount);
			#endif
		#endif
    }
    inline ~CMutex()
    {
		#if !defined(HG_CONSOLE)
			#ifdef HG_MAC_OS_X
				pthread_mutex_destroy(&m_section);
			#else
				DeleteCriticalSection(&m_section);
			#endif
		#endif
    }

	inline HGbool TryLock()
	{
#if !defined(HG_CONSOLE)
	#ifdef HG_MAC_OS_X
			return (pthread_mutex_trylock(&m_section)) ? false : true;
	#else
			return (TryEnterCriticalSection(&m_section)) ? true:false;
	#endif
#endif
		return true;
	}
	
	// Try to unlock the critical section
	inline HGvoid Unlock()
	{
#if !defined(HG_CONSOLE)
	#ifdef HG_MAC_OS_X
			pthread_mutex_unlock(&m_section);
	#else
			LeaveCriticalSection(&m_section);
	#endif
#endif
	}
	
    // Try to lock the critical section
	inline HGvoid Lock()
	{
#if !defined(HG_CONSOLE)
	#ifdef HG_MAC_OS_X
			pthread_mutex_lock(&m_section);
	#else
			EnterCriticalSection(&m_section);
	#endif
#endif
}
};


// --------------------------------------------------
// --------------------------------------------------

#endif
