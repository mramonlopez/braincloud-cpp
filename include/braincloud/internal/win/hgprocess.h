/**
* @file hgprocess.h
*
* The header file for process related api functions.
*/

#ifndef HGPROCESS_H_INCLUDED
#define HGPROCESS_H_INCLUDED


#if ( !defined (__APPLE__) || \
    defined (TARGET_OS_MAC) && !(defined (TARGET_OS_IPHONE) || defined (TARGET_IPHONE_SIMULATOR) || defined(WIN32)) )
//#   define USING_PANTHEIOS
#endif

// #include "HGType.h"
#include <string>
#include <vector>

#ifdef __ANDROID__
typedef pthread_t HANDLE;
#else
typedef void* HANDLE;
#endif

/*#ifdef WIN32
	#ifndef DLL_EXPORTS
		#define LIBPROCESS_DLL_API(returnType) __declspec(dllexport) returnType
	#else
		#define LIBPROCESS_DLL_API(returnType) __declspec(dllimport) returnType
	#endif
#else
	#define LIBPROCESS_DLL_API(returnType) extern "C" __attribute__ ((visibility("default"))) returnType
#endif*/

#if (defined(HG_LINUX)) || (defined(HG_MAC_OS_X) || (defined(HG_PLATFORM_BB)))
	#define HANDLE pthread_t
#endif

#define LIBPROCESS_DLL_API(returnType)  returnType


// defines for cpu affinity
#define		HG_CPU0		0x0001
#define		HG_CPU1		0x0002
#define		HG_CPU2		0x0004
#define		HG_CPU3		0x0008
#define		HG_CPU4		0x0010
#define		HG_CPU5		0x0020
#define		HG_CPU6		0x0040
#define		HG_CPU7		0x0080
#define		HG_CPU8		0x0100
#define		HG_CPU9		0x0200

// this will help us know what threads are running
struct SThread
{
	HANDLE		m_handle;
	std::string	m_name;

	SThread() {	m_handle = 0; }
	~SThread(){}
};

/** Returns number of cores in the current system */
LIBPROCESS_DLL_API(unsigned int)		hgprocessGetNbCore();
LIBPROCESS_DLL_API(void)		hgprocessGetProcessAffinity(HANDLE in_handle,std::string & out_affinityMask);
LIBPROCESS_DLL_API(void)		hgprocessGetThreadAffinity(HANDLE in_handle,std::string & out_affinityMask);


LIBPROCESS_DLL_API(void)		hgprocessInit();
LIBPROCESS_DLL_API(void)		hgprocessShutdown();

LIBPROCESS_DLL_API(int)		hgprocessGetCurrentAffinity();
LIBPROCESS_DLL_API(int)		hgprocessSetCurrentAffinity( const unsigned long & in_affinityMask );

LIBPROCESS_DLL_API(int)		hgprocessStartProcess( const char * appName, const char * in_arguments = 0, bool in_newWindow=false );

LIBPROCESS_DLL_API(void)		hgprocessAddThread(HANDLE in_threadHandle, const char * in_name);
LIBPROCESS_DLL_API(void)		hgprocessRemoveThread(HANDLE in_threadHandle);
LIBPROCESS_DLL_API(std::vector<SThread>*)		hgprocessGetThreads();

LIBPROCESS_DLL_API(void)  hgtSleep(unsigned long in_ms);
LIBPROCESS_DLL_API(void)  hgtHSleep(unsigned long in_ms);

#endif
