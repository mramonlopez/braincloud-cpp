//--- PRECOMPILED HEADER: DO NOT REMOVE
// #include "hgcommon.h"
//--- END PRECOMPILED HEADER
#include "CThread.h"
#include "hgprocess.h"

#if defined(USING_PANTHEIOS)

#include <pantheios/pantheios.hpp>                      // Pantheios C++ main header
#include <pantheios/inserters/boolean.hpp>              // Needed for logging boolean values
#include <pantheios/inserters/integer.hpp>              // Needed for logging long long values
#else
#include "PantheiosStub.h"
#endif

#include <iostream>
#include <stdio.h>

CThread::CThread()
{
	m_processor = 0;
	m_thread = 0;
	m_locked = false;
	m_internLock = false;
	m_isDone = true;
	m_isRunning = false;
}

CThread::~CThread()
{
	hgprocessRemoveThread(m_thread);
}

void CThread::StartThread(const unsigned int processor, const char *name)
{
	//hgcLogInfo((CWString(L"starting thread ") + name + L" on cpu : " + (HGint)processor).Str());

	m_processor = processor;
	m_isRunning = true;
	m_isDone = false;
	#if defined(WIN32) || defined(_XBOX)
		m_thread = CreateThread(NULL, 0, (unsigned long (__stdcall*)(void*))RunProcess, (void*)this, CREATE_SUSPENDED, NULL);
		//hgcLogInfo(L"CreateThread done");
		//--- Give it Normal priority
		SetThreadPriority(m_thread, THREAD_PRIORITY_NORMAL);
		#ifdef _XBOX
			XSetThreadProcessor(m_thread, processor); // force execution on a particular hardware thread
			hgcLogInfo(L"XSetThreadProcessor done");
		#else
			if( hgprocessGetNbCore() < 2 ) m_processor = 0;
			if( SetThreadIdealProcessor( m_thread, m_processor) == (DWORD)(-1) )
			{
				pantheios::log_ERROR( "error setting priority" );
			}
		#endif
		ResumeThread(m_thread);
		//hgcLogInfo(L"ResumeThread done");
		hgprocessAddThread(m_thread,name);
	#else
		// We create the thread using POSIX thread system (Linux)
		m_thread = 0;

		int rc = pthread_create( &m_thread,0,(PTHREAD_START_ROUTINE)RunProcess,(void*)this);
		if (rc)
		{
			//hgcLogError( (CWString(L"return code from pthread_create() is ") + rc).Str());
			printf("return code from pthread_create() is %i\n", rc);
		}
	#endif
}

void CThread::SetProcessor( unsigned int in_processor )
{
#ifdef _XBOX
	if( XSetThreadProcessor( GetCurrentThread(), in_processor ) == (DWORD)(-1) )
	{
		hgcLogError((CWString(L"SetThreadIdealProcessor failed : ") + (HGint)GetLastError()).Str());
	}   
#elif defined(WIN32)
	if( SetThreadIdealProcessor( GetCurrentThread(), in_processor ) == (DWORD)(-1) )
	{
		pantheios::log_ERROR("SetThreadIdealProcessor failed : ",pantheios::integer(GetLastError()));
		// hgcLogError((CWString(L"SetThreadIdealProcessor failed : ") + (HGint)GetLastError()).Str());
	}
#endif
}

/** To lock the thread */
void CThread::Lock()
{
//	SuspendThread(m_thread);

	m_locked = true;
	while (!m_internLock)
	{
		hgtHSleep(1);
		if(m_isDone) break;
	}
}

/** To unlock the thread
*/
void CThread::Unlock()
{
//	ResumeThread(m_thread);
	m_locked = false;
}

//to check if the thread has finished running
bool CThread::IsDone()
{
	return m_isDone;
}


/** To lock and wait */
void CThread::LockingPoint()
{
	if (m_locked)
	{
		m_internLock = true;
		while (m_locked)
		{
			//--- Do something
			hgtHSleep( 1 );
		}
		m_internLock = false;
	}
}


void CThread::WaitForThreadComplete()
{
	while (!m_isDone)
	{
		hgtHSleep(1);
	}
}
