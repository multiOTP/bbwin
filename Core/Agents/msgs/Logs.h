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

#ifndef __LOGS_H__
#define __LOGS_H__

#include <string>
#include <list>
#include <map>

#define	REG_BUF_SIZE		1024

namespace Logs {

enum BBLevels { BB_GREEN, BB_YELLOW, BB_RED };

//
// Log Rule class
// 
class Rule {
	private :
		DWORD			m_alarmColor; // alarm color for the rules
		bool			m_ignore;  // is it an ignore rule
		std::string		m_value; // value to match
		DWORD			m_count; // Maximum number of matches for the rules. Default is 0 which mean rule is checked all time
		DWORD			m_countTmp; // Current number of matches
		DWORD			m_priority; // priority of the rule. Default is 0

	public :
		Rule();
		~Rule();
		Rule(const Rule & rule);
		bool				GetIgnore() const { return m_ignore; } 
		void				SetIgnore(bool ignore) { m_ignore = ignore; } 
		void				SetAlarmColor(DWORD alarm) { m_alarmColor = alarm; }
		DWORD				GetAlarmColor() const { return m_alarmColor; }
		const std::string & GetValue() const { return m_value; }
		void				SetValue(const std::string & value) { m_value = value; } 
		DWORD				GetCount() const { return m_count; }
		void				SetCount(DWORD count) { m_count = count; }
		void				IncrementCurrentCount() { m_countTmp++; }
		DWORD				GetCurrentCount() const { return m_countTmp; }
		void				SetCurrentCount(DWORD currentCount) { m_countTmp = currentCount; }
		DWORD				GetPriority() const { return m_priority; }
		void				SetPriority(DWORD priority) { m_priority = priority; }
};


typedef std::multimap < std::string, HMODULE > 						log_mod_t;
typedef log_mod_t::iterator											log_mod_itr_t;
typedef std::pair< log_mod_itr_t, log_mod_itr_t >			log_mode_range_t;

//
//  Session class
//
class Session {
	private:
		std::string							m_logfile; // countains the file path to parse
		std::list<Rule>						m_matchRules; // countains the match rules
		std::list<Rule>						m_ignoreRules; // countains the ignore rules
		DWORD								m_maxDelay;
		DWORD								m_now;
		
		// counters 
		DWORD								m_total;
		DWORD								m_match;
		DWORD								m_ignore;

	private :
		void			FreeSources();
		DWORD			GetMaxDelay();
		bool			ApplyRule(const Rule & rule, const EVENTLOGRECORD * ev);
		void			InitCounters();
	public :
		DWORD			Execute(std::stringstream & reportData);
		void			GetReport() {};
		static LONG		Now();
		void			AddRule(const Rule & rule);

		Session(const std::string logfile);
		~Session();
};


//
//  Manager 
//
class Manager {
	private:
		std::map< std::string, Session * >			m_sessions;
		std::list< std::string >					m_logFileList;

	private:
	//	DWORD				AnalyzeLogFilesSize(std::stringstream & reportData, bool checking);
	//	void				GetLogFileList();
	//	void				FreeSessions() {};

	public :
	//	void		AddRule(const std::string & logfile, const Rule & rule);
	//	DWORD		Execute(std::stringstream & reportData);
	//	DWORD		GetMatchCount();
	//	DWORD		GetIgnoreCount();
	//	DWORD		GetTotalCount();
		Manager();
		~Manager();
};


}; // namespace Logs

#endif // !__LOGS_H__

