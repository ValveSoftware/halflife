//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Extracted from dbg.c (Michael S. Booth)

#ifndef _PERF_COUNTER_H_
#define _PERF_COUNTER_H_

#ifdef _WIN32
	#include <windows.h>	// Currently needed for IsBadReadPtr and IsBadWritePtr
	#include <io.h>
	#include <direct.h>
#else
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <fcntl.h>
	#include <unistd.h>
	#ifdef OSX
		#include <limits.h>
	#else
		#include <linux/limits.h>
	#endif
	#define MAX_PATH PATH_MAX
	#include <sys/time.h>
#endif

#include <assert.h>
//#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>


//-----------------------------------------------------------------------------
// Purpose: quick hacky timer class
//-----------------------------------------------------------------------------
class CPerformanceCounter
{
public:
	CPerformanceCounter();
	void InitializePerformanceCounter();
	double GetCurTime();

private:
	int m_iLowShift;
	double m_flPerfCounterFreq;
	double m_flCurrentTime;
	double m_flLastCurrentTime;
};


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
inline CPerformanceCounter::CPerformanceCounter()
{
	InitializePerformanceCounter();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
inline void CPerformanceCounter::InitializePerformanceCounter()
{
#ifdef _WIN32
	LARGE_INTEGER performanceFreq;
	QueryPerformanceFrequency(&performanceFreq);

	// get 32 out of the 64 time bits such that we have around
	// 1 microsecond resolution
	unsigned int lowpart, highpart;
	lowpart = (unsigned int)performanceFreq.LowPart;
	highpart = (unsigned int)performanceFreq.HighPart;
	m_iLowShift = 0;

	while (highpart || (lowpart > 2000000.0))
	{
		m_iLowShift++;
		lowpart >>= 1;
		lowpart |= (highpart & 1) << 31;
		highpart >>= 1;
	}

	m_flPerfCounterFreq = 1.0 / (double)lowpart;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: returns the current time
//-----------------------------------------------------------------------------
inline double CPerformanceCounter::GetCurTime()
{
#ifdef _WIN32
	static int			sametimecount;
	static unsigned int	oldtime;
	static int			first = 1;
	LARGE_INTEGER		PerformanceCount;
	unsigned int		temp, t2;
	double				time;

	QueryPerformanceCounter(&PerformanceCount);
	if (m_iLowShift == 0)
	{
		temp = (unsigned int)PerformanceCount.LowPart;
	}
	else
	{
		temp = ((unsigned int)PerformanceCount.LowPart >> m_iLowShift) |
			   ((unsigned int)PerformanceCount.HighPart << (32 - m_iLowShift));
	}

	if (first)
	{
		oldtime = temp;
		first = 0;
	}
	else
	{
		// check for turnover or backward time
		if ((temp <= oldtime) && ((oldtime - temp) < 0x10000000))
		{
			oldtime = temp;	// so we can't get stuck
		}
		else
		{
			t2 = temp - oldtime;

			time = (double)t2 * m_flPerfCounterFreq;
			oldtime = temp;

			m_flCurrentTime += time;

			if (m_flCurrentTime == m_flLastCurrentTime)
			{
				sametimecount++;

				if (sametimecount > 100000)
				{
					m_flCurrentTime += 1.0;
					sametimecount = 0;
				}
			}
			else
			{
				sametimecount = 0;
			}

			m_flLastCurrentTime = m_flCurrentTime;
		}
	}

	return m_flCurrentTime;

#else
	struct timeval	tp;
	static int	secbase = 0;
    
	gettimeofday( &tp, NULL );
 
	if ( !secbase )
	{
		secbase = tp.tv_sec;
		return ( tp.tv_usec / 1000000.0 );
	}
 
	return ( ( tp.tv_sec - secbase ) + tp.tv_usec / 1000000.0 );
#endif /* _WIN32 */
}

#endif // _PERF_COUNTER_H_
