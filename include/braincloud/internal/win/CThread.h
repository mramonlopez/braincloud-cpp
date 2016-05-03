/** 
 * CThread.h
 * Thread class to easy start/stop/lock/unlock threads
 *
 * 23/11/2006
 *
 * Copyright bitHeads 2006
 *
 *
 * Initial Revision
 * David St-Louis - 23/11/2006
 *			Initial code
 */


#ifndef CTHREAD_H_INCLUDED
#define CTHREAD_H_INCLUDED

// #include "CWString.h"
// #include "hgconsole.h"
// #include "hgtimer.h"
// #include "hgsystem.h"
#include "hgprocess.h"

#ifdef WIN32

	#undef _WIN32_WINNT
	#define _WIN32_WINNT 0x0501

	#include <windows.h>
#elif defined (_XBOX)
    #include <xtl.h>
#else
	#include <pthread.h>

	typedef void* (*PTHREAD_START_ROUTINE)( void* lpThreadParameter );
#endif


class CThread
{
private:

	/** Handle on the thread */
//	#if (!defined(WIN32)) && (!defined(_XBOX))
//		#define HANDLE pthread_t
//	#endif
	HANDLE m_thread;
	int m_processor;

	/** Is the thread is locked */
	volatile bool m_locked;

	/** Is internally locked */
	volatile bool m_internLock;

	/** keep if the thread is actually running */
	volatile bool m_isRunning;


public:

	HANDLE GetHandle() { return m_thread; }

	/** Is the thread finished */
	volatile bool m_isDone;

	/** Accessor to see if the thread exists */
	bool IsRunning() { return m_isRunning; }

	/** Constructor */
	CThread();
	//CThread() :
	//	m_processor((HGuint)-1)
	//{
	//	m_thread = 0;
	//	m_locked = false;
	//	m_internLock = false;
	//	m_isDone = false;
	//	m_isRunning = false;
	//}

	virtual ~CThread();

	/** Start the thread
	*/
	void StartThread(const unsigned int processor = 0, const char *name = "");

	/** To lock the thread */
	void Lock();
	
	/** To unlock the thread */
	void Unlock();
	
	// to check if the thread has finished running
	bool IsDone();

	/** Set the current cpu the thread should run on */
	void SetProcessor( unsigned int in_processor );

	/** Blocks the calling thread until this thread indicates that it is finished processing. */
	void WaitForThreadComplete();

private:
	/** Starting the real process here */
	static unsigned long RunProcess(void* arg)
	{
		#if defined(WIN32) || defined(_XBOX)
			unsigned long result = ((CThread*)arg)->Execute();
			((CThread*)arg)->m_isDone = true;
			
			// Preston: According to MSDN, we should not use ExitThread in C++: 
			//http://msdn.microsoft.com/en-us/library/ms682659(VS.85).aspx
			//
			//ExitThread(0); 
			return result;
		#else
			unsigned long result = ((CThread*)arg)->Execute();
			pthread_detach( ((CThread*)arg)->m_thread );
			((CThread*)arg)->m_isDone = true;
			return result;
		#endif
	}

protected:
	/** The execute thread */
	virtual unsigned long Execute()=0;

	/** To lock and wait */
	void LockingPoint();

public:
	// Execute the thread without starting it
	void ExecuteWithoutThreading()
	{
		Execute();
		m_isDone = true;
	}
};


#endif
