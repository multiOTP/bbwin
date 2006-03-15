// Code by  By Dudi Avramov  
// http://www.codeproject.com/system/cpuusage.asp
// it replace the need of pdh.lib
// use wstring instead of bstr_t by etienne grignon
// autorization of use confirmed by email 


#ifndef _CPUUSAGE_H
#define _CPUUSAGE_H

#include <windows.h>

class CCpuUsage
{
public:
	CCpuUsage();
	virtual ~CCpuUsage();

// Methods
	int GetCpuUsage();
	int GetCpuUsage(DWORD dwProcessID);

	BOOL EnablePerformaceCounters(BOOL bEnable = TRUE);

// Attributes
private:
	bool			m_bFirstTime;
	LONGLONG		m_lnOldValue ;
	LARGE_INTEGER	m_OldPerfTime100nSec;
};


#endif