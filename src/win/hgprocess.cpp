//--- PRECOMPILED HEADER: DO NOT REMOVE
// #include "hgcommon.h"
//--- END PRECOMPILED HEADER
#include "hgprocess.h"
// #include "hgconsole.h"
#include <errno.h>
// #include "std::string.h"

#if defined(USING_PANTHEIOS)
#include <pantheios/pantheios.hpp>                      // Pantheios C++ main header
#include <pantheios/inserters/boolean.hpp>              // Needed for logging boolean values
#include <pantheios/inserters/integer.hpp>              // Needed for logging long long values
#else
#include "PantheiosStub.h"
#endif

#ifdef WIN32
	#include "windows.h"
#elif defined(_XBOX)
#else
	#include <sched.h>
	#include <sys/types.h>
	#include <unistd.h>

	#define	HG_PID	pid_t

	//--- Keep the affinity mask of the current process
#if defined (__APPLE__) || defined (__ANDROID__)
	unsigned int		g_mask;
#else
	cpu_set_t	g_mask;
#endif

#endif

// currently running threads
std::vector<SThread> m_threads;


void hgprocessInit()
{
}

void hgprocessShutdown()
{
}

void hgprocessAddThread(HANDLE in_threadHandle, const char * in_name)
{
	m_threads.push_back(SThread());
	m_threads[m_threads.size()-1].m_handle = in_threadHandle;
	m_threads[m_threads.size()-1].m_name = in_name;
}

void hgprocessRemoveThread(HANDLE in_threadHandle)
{
	for( unsigned int i=0; i<m_threads.size(); i++ )
	{
		if( m_threads[i].m_handle == in_threadHandle )
		{
			m_threads.erase( m_threads.begin() + i );
			return;
		}
	}
	//hgcLogWarning(L"Trying to remove an unknown thread");
}

std::vector<SThread>* hgprocessGetThreads()
{
	return &m_threads;
}

void hgprocessGetThreadAffinity(HANDLE in_handle, std::string & out_affinityMask)
{
#if defined(WIN32)// | defined(_XBOX)
	DWORD taffinity = HG_CPU0;
	DWORD_PTR sysAM = SetThreadAffinityMask(in_handle, taffinity);
	SetThreadAffinityMask(in_handle, sysAM);

	std::string affinity;
		
	if( sysAM & HG_CPU0 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}

	if( sysAM & HG_CPU1 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}

	if( sysAM & HG_CPU2 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}

	if( sysAM & HG_CPU3 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}

	if( sysAM & HG_CPU4 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}

	if( sysAM & HG_CPU5 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}

	if( sysAM & HG_CPU6 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}

	if( sysAM & HG_CPU7 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}

	if( sysAM & HG_CPU8 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}

	if( sysAM & HG_CPU9 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}
	out_affinityMask = affinity;
#endif
}

void hgprocessGetProcessAffinity(HANDLE in_handle, std::string & out_affinityMask)
{
#if defined(WIN32)// | defined(_XBOX)
	DWORD procAM=0;
	DWORD sysAM=0;
	if( !GetProcessAffinityMask(in_handle, &procAM, &sysAM) )
	{
		// hgcLogError((std::string(L"GetProcessAffinityMask failed : ") + (int)GetLastError()).Str());
		pantheios::log_ERROR( "GetProcessAffinityMask failed : ", pantheios::integer(GetLastError()));
		return;
	}

	std::string affinity;
	if( sysAM & HG_CPU0 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}

	if( sysAM & HG_CPU1 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}

	if( sysAM & HG_CPU2 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}

	if( sysAM & HG_CPU3 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}

	if( sysAM & HG_CPU4 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}

	if( sysAM & HG_CPU5 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}

	if( sysAM & HG_CPU6 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}

	if( sysAM & HG_CPU7 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}

	if( sysAM & HG_CPU8 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}

	if( sysAM & HG_CPU9 )
	{
		affinity += "1";
	}
	else
	{
		affinity += "0";
	}
	out_affinityMask = affinity;
#endif
}

unsigned int hgprocessGetNbCore()
{
	unsigned int nbCore=1;
	#ifdef WIN32
		DWORD procAM=0;
		DWORD sysAM=0;
		if( !GetProcessAffinityMask(GetCurrentProcess(), &procAM, &sysAM) )
		{
			pantheios::log_ERROR( "GetProcessAffinityMask failed : ", pantheios::integer(GetLastError()));
			// hgcLogError((std::string(L"GetProcessAffinityMask failed : ") + (int)GetLastError()).Str());
			return nbCore;
		}

		// temporarly output affinity mask for testing purposes
		nbCore = 0;
		std::string affinity;
		if( sysAM & HG_CPU0 )
		{
			affinity += "1";
			nbCore++;
		}
		else
		{
			affinity += "0";
		}

		if( sysAM & HG_CPU1 )
		{
			affinity += "1";
			nbCore++;
		}
		else
		{
			affinity += "0";
		}

		if( sysAM & HG_CPU2 )
		{
			affinity += "1";
			nbCore++;
		}
		else
		{
			affinity += "0";
		}

		if( sysAM & HG_CPU3 )
		{
			affinity += "1";
			nbCore++;
		}
		else
		{
			affinity += "0";
		}

		if( sysAM & HG_CPU4 )
		{
			affinity += "1";
			nbCore++;
		}
		else
		{
			affinity += "0";
		}

		if( sysAM & HG_CPU5 )
		{
			affinity += "1";
			nbCore++;
		}
		else
		{
			affinity += "0";
		}

		if( sysAM & HG_CPU6 )
		{
			affinity += "1";
			nbCore++;
		}
		else
		{
			affinity += "0";
		}

		if( sysAM & HG_CPU7 )
		{
			affinity += "1";
			nbCore++;
		}
		else
		{
			affinity += "0";
		}

		if( sysAM & HG_CPU8 )
		{
			affinity += "1";
			nbCore++;
		}
		else
		{
			affinity += "0";
		}

		if( sysAM & HG_CPU9 )
		{
			affinity += "1";
			nbCore++;
		}
		else
		{
			affinity += "0";
		}

		//hgcLogInfo((std::string(L"Current system Affinity : ") + affinity).Str());
    #elif defined(_XBOX)
        nbCore = 6;
	#else
	#endif

	return nbCore;
}

// this will return how many core are giving attention(has affinity) to the current process
int hgprocessGetCurrentAffinity()
{
#if defined (__APPLE__) || defined (__ANDROID__)
	// temp
	return 1;
#else
	
	int nbCore = 0;

	// get current process affinity mask
	#ifdef WIN32
		DWORD procAM=0;
		DWORD sysAM=0;
		if( !GetProcessAffinityMask(GetCurrentProcess(), &procAM, &sysAM) )
		{
			pantheios::log_ERROR( "GetProcessAffinityMask failed : ",  pantheios::integer(GetLastError()));
			// hgcLogError((std::string(L"GetProcessAffinityMask failed : ") + (int)GetLastError()).Str());
			return nbCore;
		}
		return sysAM;
    #elif defined(_XBOX)
        return (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5);
	#else

		int rc = sched_getaffinity( 0, CPU_SETSIZE, &g_mask);
		if( rc == -1 )
		{
			hgcLogError((std::string(L"sched_getaffinity() failed, errno = ") + (int)errno).Str());
			return 0;
		}

		for( int i=0; i<CPU_SETSIZE; i++ )
		{
			if( CPU_ISSET(i, &g_mask))
			{
				nbCore++;
			}
		}

	#endif

	return nbCore;
#endif
}

//
//--- Common software sleep fonction, in milisec
//
void hgtSleep(unsigned long in_ms)
{
	#ifdef WIN32
		//INT64 startTime;
		//if (g_isPerformanceTimer)
		//{
		//	QueryPerformanceCounter((LARGE_INTEGER *) &startTime);
		//}
		//else
		//{
		//	startTime = GetTickCount();
		//}
		//INT64 targetDiff = (g_frequency * in_ms) / 1000;
		//INT64 currentTime = startTime;
		//while (currentTime - startTime < targetDiff)
		//{
		//	if (g_isPerformanceTimer)
		//	{
		//		QueryPerformanceCounter((LARGE_INTEGER *) &currentTime);
		//	}
		//	else
		//	{
		//		currentTime = GetTickCount();
		//	}
		//}
		Sleep(in_ms);
	#else
	// TO COMPLETE
// 		timespec ts;
// 
// 		ts.tv_sec = 0;
// 		ts.tv_nsec = in_ms * 1000000;
// 
// 		nanosleep(&ts,0);
	#endif
}

//
//--- Common hardware sleep fonction, in milisec
//
void hgtHSleep(unsigned long in_ms)
{
	#if defined(WIN32) || defined(_XBOX)
        // [ch] temp
    //    Sleep(0);
		Sleep( in_ms );
	#else
		timespec ts;

		ts.tv_sec = in_ms / 1000;
		ts.tv_nsec = ( in_ms % 1000 ) * 1000000;
		nanosleep(&ts,0);
	#endif
}


int hgprocessSetCurrentAffinity( const unsigned long & in_affinityMask )
{
#if defined (__APPLE__) || defined (__ANDROID__)
	// temp
	return 1;
#else
	#ifdef WIN32
		if( !SetProcessAffinityMask( GetCurrentProcess(), in_affinityMask ) )
		{
			pantheios::log_ERROR( "hgprocessSetCurrentAffinity failed err:  : ",  pantheios::integer(GetLastError()));

			//hgcLogError( (std::string(L"hgprocessSetCurrentAffinity failed, err : ") + (int)GetLastError()).Str() );
		}
    #elif defined(_XBOX)
        
	#else
		// build current mask according to the parameter
		//hgcLogInfo((std::string(L"in_affinityMask ") + (int)in_affinityMask).Str());

		// fill the affinity mask struct first
		int rc = sched_getaffinity( 0, CPU_SETSIZE, &g_mask);
		if( rc == -1 )
		{
			hgcLogError((std::string(L"sched_getaffinity() failed, errno = ") + (int)errno).Str());
			return 0;
		}

		CPU_ZERO( &g_mask );
		for( int i=0; i<10; i++ )
		{
			HGulong bitMask = (HGulong)1 << i;
			//hgcLogInfo((std::string(L"bitMask ") + (int)bitMask).Str());
			if( in_affinityMask & bitMask )
			{
				hgcLogInfo((std::string(L"Setting up affinity for HG_CPU") + i).Str());
				CPU_SET( i, &g_mask );
			}
		}

		rc = sched_setaffinity( 0, CPU_SETSIZE, &g_mask);
		if( rc == -1 )
		{
			hgcLogError((std::string(L"sched_setaffinity() failed, errno = ") + errno).Str());
			return 0;
		}
	#endif
	return 1;
#endif
}

/***
int hgprocessStartProcess( const char * appName, const char * in_arguments, bool in_newWindow)
{
	// convert the appName and arguments to ascii
	std::string uniAppName = appName;
	std::string uniArgs;
	if( in_arguments ) uniArgs = in_arguments;

	// char * asciiAppName = new char[ uniAppName.Length() + 1 ];
	// uniAppName.ToString( asciiAppName, uniAppName.Length() + 1 );

	//create list of arguments
	int nbArgs=0;
	std::string tokenize = uniArgs;
	std::string token = tokenize.ExtractLeftToken(L' ');
	while (token != L"")
	{
		nbArgs++;
		token = tokenize.ExtractLeftToken(L' ');
	}

	// create char array of arguments
	HGchar ** asciiArgs = 0;
	if( nbArgs )
	{
		asciiArgs = new HGchar*[nbArgs + 1];
		tokenize = uniArgs;
		for( int i=0; i<nbArgs; i++ )
		{
			token = tokenize.ExtractLeftToken(L' ');
			asciiArgs[i] = new HGchar[ token.Length() + 1 ];
			token.ToString( asciiArgs[i], token.Length() + 1 );
		}
		asciiArgs[nbArgs] = (char*) 0;
	}

	//--- let's start a new process completly independant from the calling process, and set his
	//		affinity level depending on the number of cores we have
	#ifdef WIN32
		STARTUPINFO				si = { sizeof(si) };
		PROCESS_INFORMATION		pi;

		std::string apName = asciiAppName;
		std::string apArg = nbArgs ? asciiArgs[0] : "";

		DWORD flags = NORMAL_PRIORITY_CLASS;
		if( in_newWindow )
		{
			#ifdef HG_CONSOLE
				flags |= CREATE_NEW_CONSOLE;
			#endif
		}

		if(!CreateProcess(
			apName.Str(),
			(char*)apArg.Str(),
			NULL,
			NULL,
			NULL,
			flags,
			NULL,
			NULL,
			&si,
			&pi ))
		{
			hgcLogError((std::string(L"Problem starting app ") + asciiAppName + L" " + asciiArgs[0]).Str());
		}

    #elif defined(_XBOX)

	#else
		// first we vfork from the main process
		HG_PID pid = vfork();
	
		if( pid == 0 )
		{
			//--- the forking worked, we are in the child process, let's boot the new process from here
			// prepare list of argument, for the moment, we only support 1
			
			int rc = execve( asciiAppName, asciiArgs, 0);
			if( rc == -1 )
			{
				hgcLogError((std::string(L"problem trying to boot '") + asciiAppName + L"', errno = " + (int)errno).Str());
			}
			_exit(0);
		}
		else if( pid < 0 )
		{
			// failed to fork
			hgcLogError((std::string(L"forking failed, errno = ") + (int)errno).Str());
			return 0;
		}
		else
		{
			// we are back in the current calling process
		}
	#endif

	delete [] asciiAppName;

	if( asciiArgs )
	{
		for( int z=0; z<nbArgs; z++ )
		{
			delete [] asciiArgs[z];
		}
		delete [] asciiArgs;
		asciiArgs = 0;
	}

	return 1;
}

**/
