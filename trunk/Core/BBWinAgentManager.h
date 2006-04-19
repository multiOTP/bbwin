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

#ifndef 	__BBWINAGENTMANAGER_H__
#define		__BBWINAGENTMANAGER_H__

#include "IBBWinException.h"
#include "IBBWinAgentManager.h"

#include "Logging.h"
#include "BBWinHandlerData.h"
#include "BBWinNet.h"

class   BBWinAgentManager : public IBBWinAgentManager {
	private :
		std::string			& m_agentName;
		std::string			& m_agentFileName;
		HANDLE				*m_hEvents;
		DWORD				m_hCount;
		Logging				*m_log;
		bbdisplay_t			& m_bbdisplay;
		setting_t			& m_setting;
		DWORD				m_timer;
		BBWinNet			m_net;
		bool				m_logReportFailure;
		bbwinagentconfig_t * m_conf;
		bbwinconfig_range_t	m_range;

	private :
		void	LoadFileConfiguration(const std::string & filePath, const std::string & nameSpace, bbwinagentconfig_t & config);
	
	public :
		BBWinAgentManager(const bbwinhandler_data_t & data);

		LPCTSTR	GetAgentName() { return m_agentName.c_str(); };
		LPCTSTR	GetAgentFileName() { return m_agentFileName.c_str(); };
		void			GetEnvironmentVariable(LPCTSTR varname, LPTSTR dest, DWORD size);
		void			GetLastErrorString(LPTSTR dest, DWORD size);
		DWORD			GetNbr(LPCTSTR str);
		DWORD			GetSeconds(LPCTSTR str);
		const std::string & GetSetting(LPCTSTR settingName);
		
		DWORD	GetAgentTimer() { return m_timer; };
		void	GetHandles(HANDLE * hEvents);
		
		DWORD	GetHandlesCount() { return m_hCount; } ;
		
		bbwinagentconfig_t * LoadConfiguration(LPCTSTR nameSpace);
		bbwinagentconfig_t * LoadConfiguration(LPCTSTR fileName, LPCTSTR nameSpace);
		void	FreeConfiguration(bbwinagentconfig_t * conf);
		bbwinconfig_range_t * GetConfigurationRange(bbwinagentconfig_t * conf, LPCTSTR name);
		LPCTSTR				GetConfigurationRangeValue(bbwinconfig_range_t *range, LPCTSTR name);
		void				FreeConfigurationRange(bbwinconfig_range_t *range);

		// hobbit protocol
		void 		Status(LPCTSTR testName, LPCTSTR color, LPCTSTR text, LPCTSTR lifeTime = "");
		void		Notify(LPCTSTR testName, LPCTSTR text);
		void		Data(LPCTSTR dataName, LPCTSTR text);
		void 		Disable(LPCTSTR testName, LPCTSTR duration, LPCTSTR text);
		void		Enable(LPCTSTR testName);
		void		Drop();
		void		Drop(LPCTSTR testName);
		void		Rename(LPCTSTR newHostName);
		void		Rename(LPCTSTR oldTestName, LPCTSTR newTestName);
		void		Message(LPCTSTR message, LPTSTR dest, DWORD size);
		void		Config(LPCTSTR fileName, LPTSTR dest, DWORD size);
		void		Query(LPCTSTR testName, LPTSTR dest, DWORD size);
		
		// Report Functions : report in the bbwin log file
		void 	ReportError(LPCTSTR str);
		void 	ReportInfo(LPCTSTR str);
		void 	ReportDebug(LPCTSTR str);
		void 	ReportWarn(LPCTSTR str);
		
		// Event Report Functions : report in the Windows event log
		void 	ReportEventError(LPCTSTR str);
		void 	ReportEventInfo(LPCTSTR str);
		void 	ReportEventWarn(LPCTSTR str);
};

#endif // __BBWINAGENTMANAGER_H__
