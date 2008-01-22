//this file is part of BBWin
//Copyright (C)2008 Etienne GRIGNON  ( etienne.grignon@gmail.com )
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
// 
// $Id$

#define BBWIN_AGENT_EXPORTS

#include <windows.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <list>
#include "utils.h"
#include "digest.h"
#include "filesystem.h"
#include "boost/format.hpp"

using namespace std;
using namespace boost;
using boost::format;

static const BBWinAgentInfo_t 		filesystemAgentInfo =
{
	BBWIN_AGENT_VERSION,				// bbwinVersion;
	"filesystem",    					// agentName;
	"agent used to monitor files and directories",        // agentDescription;
	BBWIN_AGENT_CENTRALIZED_COMPATIBLE			// flags
};                

static bool		IsBackQuotedString(const std::string & str) {
	if (str.substr(0, 1) == "`" && str.substr(str.size() - 1, 1) == "`")
		return true;
	return false;
}

// execute backquoted commands
void		AgentFileSystem::GetLinesFromCommand(const std::string & command, std::list<string> & list) {
	utils::ProcInOut		process;
	string					cmd, out;

	// remove backquoted from command
	cmd = command.substr(1, command.size() - 2);
	if (process.Exec(cmd, out, MAX_TIME_BACKQUOTED_COMMAND)) {
		// remove '\r; 
		out.erase(std::remove(out.begin(), out.end(), '\r'), out.end());
		std::string::size_type pos = 0;
		if (out.size() == 0)
			return ;
		for ( ;pos < out.size(); ) {
			std::string::size_type	next = out.find_first_of("\n", pos);
			string					line;
			if (next > 0 && next < out.size()) {
				line = out.substr(pos, (next - pos));
				if (line != "")
					list.push_back(line);
				pos += (next - pos) + 1;
			} else {
				line = out.substr(pos, out.size() - pos);
				if (line != "")
					list.push_back(line);
				pos = out.size();
			}
			m_mgr.Log(LOGLEVEL_DEBUG, "find line %s", line.c_str());
		}
	} else {
		string			err;

		utils::GetLastErrorString(err);
		m_mgr.Log(LOGLEVEL_WARN, "%s command failed : %s", cmd.c_str(), err.c_str());
	}
}

bool		AgentFileSystem::InitCentralMode() {
	string clientLocalCfgPath = m_mgr.GetSetting("tmppath") + (string)"\\clientlocal.cfg";

	m_mgr.Log(LOGLEVEL_DEBUG, "start %s", __FUNCTION__);
	m_files.clear();
	m_dirs.clear();
	m_mgr.Log(LOGLEVEL_DEBUG, "checking file %s", clientLocalCfgPath.c_str());
	ifstream		conf(clientLocalCfgPath.c_str());

	if (!conf) {
		string	err;

		utils::GetLastErrorString(err);
		m_mgr.Log(LOGLEVEL_INFO, "can't open %s : %s", clientLocalCfgPath.c_str(), err.c_str());
		return false;
	}
	m_mgr.Log(LOGLEVEL_DEBUG, "reading file %s", clientLocalCfgPath.c_str());
	string		buf, eventlog, ignore, trigger;
	while (!conf.eof()) {
		string		value;

		std::getline(conf, buf);
		if (utils::parseStrGetNext(buf, "file:", value)) {
			fs_file_t	file;
			string		hash;
			
			file.hashtype = NONE;
			m_mgr.Log(LOGLEVEL_DEBUG, "will check file %s", value.c_str());
			// substr(3) : we do not parse C:\ to get the next part to ':' separator
			if (utils::parseStrGetLast(value.substr(3), ":", hash)) {
				value.erase(value.end() - (hash.size() + 1), value.end());
				if (hash == "md5") {
					m_mgr.Log(LOGLEVEL_DEBUG, "will use md5 hash type", hash.c_str());
					file.hashtype = MD5;
				} else if (hash == "sha1") {
					m_mgr.Log(LOGLEVEL_DEBUG, "will use sha1 hash type", hash.c_str());
					file.hashtype = SHA1;
				} else {
					m_mgr.Log(LOGLEVEL_WARN, "Unknow hash type for file configuration : %s", hash.c_str());
				}
			}
			if (IsBackQuotedString(value)) {
				std::list<string>			list;
				std::list<string>::iterator	itr;

				GetLinesFromCommand(value, list);
				for (itr = list.begin(); itr != list.end(); ++itr) {
					file.path = *itr;
					m_files.push_back(file);
				}
			} else {
				file.path = value;
				m_files.push_back(file);
			}
		} else if (utils::parseStrGetNext(buf, "dir:", value)) {
			if (IsBackQuotedString(value)) {
				std::list<string>			list;
				std::list<string>::iterator	itr;

				GetLinesFromCommand(value, list);
				for (itr = list.begin(); itr != list.end(); ++itr) {
					m_dirs.push_back(*itr);
				}
			} else {
				m_dirs.push_back(value);
			}
		}
	}
	return false;
}

bool		AgentFileSystem::GetTimeString(const FILETIME & filetime, string & output) {
	__int64	ftime = (filetime.dwHighDateTime * MAXDWORD) + filetime.dwLowDateTime;
	SYSTEMTIME	fsystime;
	FILETIME	floctime;
	time_t		epoch;

	if (::FileTimeToLocalFileTime(&filetime, &floctime) == 0) {
		return false;
	}
	if (::FileTimeToSystemTime(&floctime, &fsystime) == 0) {
		return false;
	}
	utils::SystemTimeToTime_t(&fsystime, &epoch);
	stringstream reportData;
	reportData << format("%llu (%02d/%02d/%d-%02d:%02d:%02d)") 
					% (unsigned __int64)epoch
					% fsystime.wYear 
					% fsystime.wMonth
					% fsystime.wDay
					% fsystime.wHour
					% fsystime.wMinute
					% fsystime.wSecond;
	output = reportData.str();
	return true;
}

bool		AgentFileSystem::GetFileAttributes(const string & path, stringstream & reportData) {
	HANDLE							handle;
	BY_HANDLE_FILE_INFORMATION		handle_file_info;
	__int64							fsize;   

	handle = CreateFile(path.c_str(), 
				FILE_SHARE_READ, 
				0,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		string		err;

		utils::GetLastErrorString(err);
		reportData << "ERROR: " << err << endl;
		return false;
	}
	if (::GetFileInformationByHandle(handle, &handle_file_info) == 0) {
		string		err;

		utils::GetLastErrorString(err);
		reportData << "ERROR: " << err << endl;
		CloseHandle(handle);
		return false;
	}
	fsize = (handle_file_info.nFileSizeHigh * MAXDWORD) + handle_file_info.nFileSizeLow;
	reportData << format("type:0x%05x ") % handle_file_info.dwFileAttributes;
	if (handle_file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		reportData << "(dir)" << endl;
	else
		reportData << "(file)" << endl;
	reportData << "mode:777 (not implemented)" << endl;
	reportData << format("linecount:%u") % handle_file_info.nNumberOfLinks << endl;
	reportData << "owner:0 (not implemented)" << endl;
	reportData << "group:0 (not implemented)" << endl;
	reportData << format("size:%lu") % fsize << endl;
	string output;
	GetTimeString(handle_file_info.ftLastAccessTime, output);
	reportData << "atime:" << output << endl;
	GetTimeString(handle_file_info.ftCreationTime, output);
	reportData << "ctime:" << output << endl;
	GetTimeString(handle_file_info.ftLastWriteTime, output);
	reportData << "mtime:" << output << endl;
	CloseHandle(handle);
	return true;
}

void	AgentFileSystem::ListFiles(const std::string & path, std::stringstream & report, __int64 & size) {
	WIN32_FIND_DATA		find_data;
	string				mypath = path + "\\*";
	
	HANDLE handle = FindFirstFile(mypath.c_str(), &find_data);
	if (handle != INVALID_HANDLE_VALUE) {
		 // skip "." and ".." directories
		FindNextFile(handle, &find_data);
		while (FindNextFile(handle, &find_data)) {
			string	newpath = path + "\\" + find_data.cFileName;
			__int64 tmpsize = ((find_data.nFileSizeHigh * MAXDWORD) + find_data.nFileSizeLow);
			report << format("%lu\t %s") % ((tmpsize < 1024 && tmpsize != 0) ? 1 : tmpsize / 1024) % newpath << endl;
			size += tmpsize;
			if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				ListFiles(newpath , report, size);
			}
		}
		FindClose(handle);
	} else {
		string		err;

		utils::GetLastErrorString(err);
		report << "ERROR: " << err << endl;
		return ;
	}
}

void		AgentFileSystem::ExecuteDirRule(const std::string & dir) {
	stringstream 		reportData;	
	__int64				size = 0;

	string title = "dir:" + dir;
	ListFiles(dir, reportData, size);
	if (reportData.str().substr(0, 5) != "ERROR")
		reportData << format("%lu\t %s") % ((size < 1024 && size != 0) ? 1 : size / 1024) % dir << endl;
	reportData << endl;
	m_mgr.ClientData(title.c_str(), reportData.str().c_str());
}

void		AgentFileSystem::ExecuteFileRule(const fs_file_t & file) {
	stringstream 		reportData;	
	
	string	title = "file:" + file.path;
	if (this->GetFileAttributes(file.path, reportData) && file.hashtype != NONE) {
		digestctx_t			*dig;
		HANDLE				hFile;
		string				digstr;
		char				buf[1024];
		DWORD				read;

		if (file.hashtype == MD5) {
			dig = digest_init("md5");
		} else {
			dig = digest_init("sha1");
		}
		if (dig != NULL) {
			hFile = CreateFile(file.path.c_str(),     // file to open
				GENERIC_READ,         // open for reading
				NULL, // do not share
				NULL,                   // default security
				OPEN_EXISTING, // default flags
				0,   // asynchronous I/O
				0);                // no attr. template

			if (hFile != INVALID_HANDLE_VALUE) {
				while (ReadFile(hFile, buf, 1024, &read, NULL)) {
					if (read == 0)
						break ;
					digest_data(dig, (unsigned char *)buf, read);
				}
				CloseHandle(hFile);
				digstr = digest_done(dig);
				reportData << digstr << endl;
			}
		}
	}
	reportData << endl;
	m_mgr.ClientData(title.c_str(), reportData.str().c_str());
}

void		AgentFileSystem::ExecuteRules() {
	for (std::list<fs_file_t>::iterator file_itr = m_files.begin(); file_itr != m_files.end(); ++file_itr) {
		ExecuteFileRule(*file_itr);
	}
	for (std::list<string>::iterator dir_itr = m_dirs.begin(); dir_itr != m_dirs.end(); ++ dir_itr) {
		ExecuteDirRule(*dir_itr);
	}
}

void 		AgentFileSystem::Run() {
	stringstream 		reportData;	
	if (m_mgr.IsCentralModeEnabled() == false)
		return ;
	InitCentralMode();
	ExecuteRules();
}

bool AgentFileSystem::Init() {
	return true;
}

AgentFileSystem::AgentFileSystem(IBBWinAgentManager & mgr) : m_mgr(mgr) {
}

BBWIN_AGENTDECL IBBWinAgent * CreateBBWinAgent(IBBWinAgentManager & mgr)
{
	return new AgentFileSystem(mgr);
}

BBWIN_AGENTDECL void		 DestroyBBWinAgent(IBBWinAgent * agent)
{
	delete agent;
}

BBWIN_AGENTDECL const BBWinAgentInfo_t * GetBBWinAgentInfo() {
	return &filesystemAgentInfo;
}
