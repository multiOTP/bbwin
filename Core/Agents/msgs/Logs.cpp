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

#include <windows.h>

#include <assert.h>

#include <string>    // inclut aussi <cctype> et donc tolower
#include <sstream>
#include <fstream>
#include <iostream>  // pour cout

#include <boost/regex.hpp>
#include "boost/format.hpp"

#include "Logs.h"

using namespace boost;
using namespace std;
using namespace Logs;


using boost::format;

//
// common bb colors
//
static const char	*bbcolors[] = {"green", "yellow", "red", NULL};


//
// generic method
//

static void			MyGetEnvironmentVariable(const string & varname, string & dest) {
	DWORD 		dwRet;
	char		buf[REG_BUF_SIZE + 1];
 
	SecureZeroMemory(buf, sizeof(buf));
	dwRet = ::GetEnvironmentVariable(varname.c_str(), buf, MAX_PATH);
	if (dwRet != 0) {
		dest = buf;
	}
}

static void			ReplaceEnvVariable(string & str) {
	size_t		resFirst, resSecond;
	string		envname;
	
	resFirst = str.find_first_of("%");
	if (resFirst >= 0 && resFirst < str.size()) {
		str.erase(resFirst, 1);
		resSecond = str.find_first_of("%");
		if (resSecond >= 0 && resSecond < str.size()) {
			string			var;

			envname = str.substr(resFirst, resSecond);
			str.erase(resSecond, 1);
			str.erase(resFirst, resSecond);
			MyGetEnvironmentVariable(envname, var);
			str.insert(resFirst, var);
			ReplaceEnvVariable(str);
		}
	}
}

static void				Convert1970ToSystemTime(LONG seconds, SYSTEMTIME * myTime) {
	FILETIME		fileTime;
	SYSTEMTIME		origin;

	assert(myTime != NULL);
	ZeroMemory(&fileTime, sizeof(fileTime));
	ZeroMemory(myTime, sizeof(*myTime));
	ZeroMemory(&origin, sizeof(origin));
	origin.wYear = 1970;
	origin.wMonth = 1;
	origin.wDay = 1;
	origin.wSecond = 1;
	origin.wHour = 0;
	origin.wMilliseconds = 0;
	origin.wMinute = 0;
	if (!SystemTimeToFileTime(&origin, &fileTime))
		return ;
	ULARGE_INTEGER	ulTmp, ulOrigin;
	ulOrigin.HighPart = fileTime.dwHighDateTime;
	ulOrigin.LowPart = fileTime.dwLowDateTime;
	ulTmp.QuadPart = (ulOrigin.QuadPart + (seconds * 10000000)); 
	ZeroMemory(&fileTime, sizeof(fileTime));
	fileTime.dwHighDateTime = ulTmp.HighPart;
	fileTime.dwLowDateTime = ulTmp.LowPart;
	FileTimeToSystemTime(&fileTime, myTime);
}


static void		TimeToFileTime(LONG t, LPFILETIME pft) {
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = (DWORD)(ll >> 32);
}

static void		TimeToSystemTime(LONG t, LPSYSTEMTIME pst) {
	FILETIME ft;

	TimeToFileTime(t, &ft);
	FileTimeToSystemTime(&ft, pst);
}

static void replace_all(string& str, const string& fnd, const string& rep ) {
	std::string::size_type pos = 0;
	std::string::size_type len = fnd.length();
	std::string::size_type rep_len = rep.length();

	while((pos = str.find(fnd,pos)) != std::string::npos) {
		str.replace(pos, len, rep);
		pos += rep_len;
	} 
} 


static void		clean_spaces(string & str) {
	std::string::size_type		begin = 0;
	std::string::size_type		end = 0;

	while ((begin = str.find("  ", begin)) != std::string::npos) {
		begin += 1;
		end = str.find_first_not_of(" ", begin);
		if (end != std::string::npos) {
			str.erase(begin, end - begin);
		}
	}
}


//
// Rule methods
//
//
Rule::Rule() :
		m_alarmColor(1),
		m_ignore(false),
		m_count(0),
		m_countTmp(0),
		m_priority(0)
{

}

Rule::~Rule() {
		
}


Rule::Rule(const Rule & rule) {
	m_alarmColor = rule.GetAlarmColor();
	m_ignore = rule.GetIgnore();
	m_value = rule.GetValue();
	m_count = rule.GetCount();
	m_countTmp = 0;
	m_priority = rule.GetPriority();
}



//
// Session methods
//



//
// Manager methods
//
Manager::Manager() {
	
}


Manager::~Manager() {
	//std::map<std::string, Session *>::iterator	itr;
	//
	//for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr) {
	//	delete (*itr).second;
	//}
}

//void		Manager::AddRule(const std::string & logfile, const Rule & rule) {

//}


//
//DWORD		Manager::Execute(stringstream & reportData) {
//	std::map<std::string, Session *>::iterator	itr;
//	DWORD	color = BB_GREEN;
//	DWORD	tmp;
//
//	for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr) {
//		tmp = (*itr).second->Execute(reportData);
//		if (tmp > color)
//			color = tmp;
//	}
//	AnalyzeLogFilesSize(reportData, false);
//	return color;
//}
//
//DWORD		Manager::GetMatchCount() {
//	std::map<std::string, Session *>::iterator	itr;
//	DWORD	inc = 0;
//
//	for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr) {
//		inc += (*itr).second->GetMatchCount();
//	}
//	return inc;
//}


static DWORD		myGetFileSize(LPCSTR path) {
	HANDLE			h;
	DWORD			size;

	if ((h = CreateFile(path, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL)) == INVALID_HANDLE_VALUE) {
		return 0;
	}
	size = GetFileSize(h, NULL);
	CloseHandle(h);
	return size;
}
