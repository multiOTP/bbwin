//this file is part of BBWin
//Copyright (C)2006 Etienne GRIGNON  ( etienne.grignon@gmail.com )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef		__CPU_H__
#define		__CPU_H__

#include <string>
#include <map>
#include <set>

#include "IBBWinAgent.h"

#define	 CPU_DELAY		0
#define	MAX_TABLE_PROC	512

//
// Default alerts values
//
#define DEF_CPU_WARN	90
#define DEF_CPU_PANIC	95
#define DEF_CPU_DELAY	3

enum BBAlarmType { GREEN, YELLOW, RED };


class	CpuRule {
	private :
		DWORD		m_delay;
		DWORD		m_curDelay;
		DWORD		m_warnPercent;
		DWORD		m_panicPercent;
		
	public :
		
};

class UsageProc {
	private :
		DWORD			m_pid;
		std::string		m_name;
		bool			m_exists;
		CCpuUsage		m_usage;
		double			m_lastUsage;
		size_t			m_mem;

	public :
		__stdcall UsageProc(DWORD pid);
		bool	__stdcall GetExists() const { return m_exists; };
		void	__stdcall SetExists(const bool exists) { m_exists = exists; };
		double	__stdcall ExecGetUsage();
		double	__stdcall GetUsage() const { return m_lastUsage; } ;
		DWORD	__stdcall GetPid() const { return m_pid; };
		void	__stdcall SetMemUsage(const size_t & mem) { m_mem = mem; };
		size_t	__stdcall GetMemUsage() const { return m_mem; };
		void	__stdcall SetName(const std::string & name) { m_name = name; };
		const std::string &  __stdcall GetName() const { return m_name; };
};

//  
// Comparative function used for the set container (procs sorted by usage)
struct cpuusagecomp
{
  bool operator()(const UsageProc * p1, const UsageProc * p2) const
  {
	return (p1->GetUsage() >= p2->GetUsage());
  }
};

// typedefs of the set ( procs sorted by usage)
typedef set<const UsageProc *, cpuusagecomp>			procs_sorted_t;
typedef set<const UsageProc *, cpuusagecomp>::iterator	procs_sorted_itr_t;


typedef std::map<DWORD, UsageProc *>::iterator 	procs_itr;

class AgentCpu : public IBBWinAgent
{
	private :
		IBBWinAgentManager 				& m_mgr;
		CCpuUsage						m_usage; // cpu usage used when psmode is off
		DWORD							m_procCount; // number of processors on the server
		bool							m_alwaysgreen; // if no status check is done
		bool							m_firstPass; // to know if it is the first passage
		bool							m_psMode;
		DWORD							m_pageColor;
		std::map<DWORD, UsageProc *>	m_procs;
		procs_sorted_t					m_procsSorted; // set container with sorted procs
		double							m_systemWideCpuUsage; //Cpu usage (sum of all proc usages)
		
		DWORD							m_warnPercent;
		DWORD							m_panicPercent;
		DWORD							m_delay;
		DWORD							m_curDelay;
		
	private :
		void		GetProcsData();
		void		GetCpuData();
		void		DeleteOlderProcs();
		void		InitProcs();

		//void		ApplyRules();
		void		SendStatusReport();
	
	public :
		AgentCpu(IBBWinAgentManager & mgr);
		~AgentCpu();
		const BBWinAgentInfo_t & About();
		bool Init();
		void Run();
};


#endif 	// !__CPU_H__

