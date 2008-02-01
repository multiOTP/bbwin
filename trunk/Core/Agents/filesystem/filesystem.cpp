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
#include <io.h>
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
	// clear existing rules. We parse client-local.cfg each run time
	m_files.clear();
	m_dirs.clear();
	m_logfiles.clear();
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
	bool			skipNextLineFlag = false;
	while (!conf.eof()) {
		string			value;

		if (skipNextLineFlag == true) {
			skipNextLineFlag = false;
		} else {
			std::getline(conf, buf);
		}
		if (utils::parseStrGetNext(buf, "file:", value)) {
			fs_file_t		file;
			string			hash;
			
			file.hashtype = NONE;
			file.logfile = false;
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
					m_mgr.Log(LOGLEVEL_DEBUG, "will check file %s", file.path.c_str());
					m_files.push_back(file);
				}
			} else {
				file.path = value;
				m_mgr.Log(LOGLEVEL_DEBUG, "will check file %s", file.path.c_str());
				m_files.push_back(file);
			}
		} else if (utils::parseStrGetNext(buf, "dir:", value)) {
			if (IsBackQuotedString(value)) {
				std::list<string>			list;
				std::list<string>::iterator	itr;

				GetLinesFromCommand(value, list);
				for (itr = list.begin(); itr != list.end(); ++itr) {
					m_mgr.Log(LOGLEVEL_DEBUG, "will check directory %s", (*itr).c_str());
					m_dirs.push_back(*itr);
				}
			} else {
				m_mgr.Log(LOGLEVEL_DEBUG, "will check directory %s", value.c_str());
				m_dirs.push_back(value);
			}
		} else if (utils::parseStrGetNext(buf, "log:", value)) {
			fs_logfile_t	logfile;
			string			sizeStr;
			
			if (utils::parseStrGetLast(value.substr(3), ":", sizeStr)) {
				value.erase(value.end() - (sizeStr.size() + 1), value.end());
				std::istringstream iss(sizeStr);
				iss >> logfile.maxdata;
				m_mgr.Log(LOGLEVEL_DEBUG, "will use maxdata size : %u", logfile.maxdata);
			} else {
				continue ;
			}
			// read next arguments : ignore and trigger options
			while (!conf.eof()) {
				string			arg;
				std::getline(conf, buf);
				if (utils::parseStrGetNext(buf, "ignore ", arg)) {
					m_mgr.Log(LOGLEVEL_DEBUG, "will ignore : %s", arg.c_str());
					logfile.ignores.push_back(arg);
				} else if (utils::parseStrGetNext(buf, "trigger ", arg)) {
					m_mgr.Log(LOGLEVEL_DEBUG, "will trigger : %s", arg.c_str());
					logfile.triggers.push_back(arg);
				} else {
					skipNextLineFlag = true;
					break ;
				}
			}
			if (IsBackQuotedString(value)) {
				std::list<string>			list;
				std::list<string>::iterator	itr;

				GetLinesFromCommand(value, list);
				for (itr = list.begin(); itr != list.end(); ++itr) {
					logfile.path = (*itr);
					m_logfiles.push_back(logfile);
				}
			} else {
				logfile.path = value;
				m_logfiles.push_back(logfile);
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

bool		AgentFileSystem::GetFileAttributes(const string & path, stringstream & reportData, bool logfile) {
	HANDLE							handle;
	BY_HANDLE_FILE_INFORMATION		handle_file_info;
	__int64							fsize;   

	FILE *f = _fsopen(path.c_str(), "rt", _SH_DENYNO);
	if (f == NULL) {
		string		err;

		utils::GetLastErrorString(err);
		reportData << "ERROR: " << err << endl;
		return false;
	}
	if ((handle = (HANDLE)_get_osfhandle(f->_file)) == INVALID_HANDLE_VALUE) {
		string		err;

		utils::GetLastErrorString(err);
		reportData << "ERROR: " << err << endl;
		return false;
	}
	if (::GetFileInformationByHandle(handle, &handle_file_info) == 0) {
		string		err;

		utils::GetLastErrorString(err);
		reportData << "ERROR: " << err << endl;
		fclose(f);
		return false;
	}
	fsize = (handle_file_info.nFileSizeHigh * MAXDWORD) + handle_file_info.nFileSizeLow;
	// we get size information for logfile parsing
	if (logfile == true) {
		std::map<std::string, fs_logfile_seekdata_t>::iterator		res = m_seekdata.find(path);
		if (res == m_seekdata.end()) { 
			fs_logfile_seekdata_t		seekdata;
			SecureZeroMemory(&seekdata, sizeof(seekdata));
			seekdata.used = true;
			seekdata.size = fsize;
			m_seekdata.insert(std::pair<std::string, fs_logfile_seekdata_t> (path, seekdata));
		} else {
			// the seek data is used
			fs_logfile_seekdata_t & seekdata = (*res).second;
			seekdata.used = true;
			seekdata.size = fsize;
		}
	}
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
	fclose(f);
	return true;
}

bool	AgentFileSystem::ListFiles(const std::string & path, std::stringstream & report, __int64 & size) {
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
		return false;
	}
	return true;
}

bool		AgentFileSystem::ExecuteDirRule(const std::string & dir) {
	stringstream 		reportData;	
	__int64				size = 0;
	bool				ret;

	string title = "dir:" + dir;
	ret = ListFiles(dir, reportData, size);
	if (reportData.str().substr(0, 5) != "ERROR")
		reportData << format("%lu\t %s") % ((size < 1024 && size != 0) ? 1 : size / 1024) % dir << endl;
	reportData << endl;
	m_mgr.ClientData(title.c_str(), reportData.str().c_str());
	return ret;
}

bool		AgentFileSystem::GetDataFromLogFile(const fs_logfile_t & logfile, std::stringstream data) {
	
	return true;
}

bool		AgentFileSystem::ExecuteLogFileRule(const fs_logfile_t & logfile) {
	std::map<std::string, fs_logfile_seekdata_t>::iterator	res;
	stringstream						reportData;

	// first exec file rule
	fs_file_t		file;
	file.hashtype = NONE;
	file.path = logfile.path;
	file.logfile = true;
	if (ExecuteFileRule(file) == false) {
		// first step failed
		return false;
	}
	// get or create the seek data
	res = m_seekdata.find(logfile.path);
	if (res == m_seekdata.end()) { 
		// should never append
		return false;
	}
	FILE *f = _fsopen(logfile.path.c_str(), "rt", _SH_DENYNO);
	if (f == NULL) {
		return false;
	}
	// the seek data is used
	fs_logfile_seekdata_t & seekdata = (*res).second;
	seekdata.used = true;
	TCHAR			buf[LOGFILE_BUFFER];
	fpos_t			pos = 0;

	cout << "OLDEST Point " << seekdata.point[SEEKDATA_START_POINT] << endl;
	if (seekdata.point[SEEKDATA_START_POINT] > 0) {
		// get the save point
		cout << "Res " << fseek(f, (long)seekdata.point[SEEKDATA_START_POINT], 0) << endl;
	} else {
		// skip to end of file depending file size and logfile maxdata
		if (seekdata.size > logfile.maxdata) {
			fseek(f, (long)(seekdata.size - logfile.maxdata), 0);
		}
	}
	while ((fgets(buf, LOGFILE_BUFFER, f) != NULL)) {
		stringstream			tmp;

		tmp << buf;
		//while (tmp.getline()
		reportData << tmp;
	}
	if (fgetpos(f, &pos) == 0) {
		// failed
	}
	// skip oldest value to save current position
	for (DWORD count = SEEKDATA_START_POINT; count > 0; --count) {
		cout << seekdata.point[count] << "-" << seekdata.point[count - 1] << endl;
		seekdata.point[count] = seekdata.point[count - 1];
	}
	seekdata.point[0] = pos;
	fclose(f);

	// prepare the title of the section
	string title = "msgs:" + logfile.path;
	reportData << endl;

	string report = reportData.str();
	// clean the data sent
	if (report.size() > logfile.maxdata) {
		report.insert(0, "<...SKIPPED...>\n");
	}
	report.erase(std::remove(report.begin(), report.end(), '\r'), report.end());
	m_mgr.ClientData(title.c_str(), report.c_str());
	return true;
}

void					AgentFileSystem::LoadSeekData() {

}

void					AgentFileSystem::SaveSeekData() {
	string seekdataCfgPath = m_mgr.GetSetting("tmppath") + (string)"\\logfetch.status";
	std::map<std::string, fs_logfile_seekdata_t>::iterator	itr;
	
	cout << seekdataCfgPath << endl;
	Sleep(5000);
	FILE *f = fopen(seekdataCfgPath.c_str(), "wt");
	if (f == NULL) {
		string		err;

		utils::GetLastErrorString(err);
		cout << "Error: " << err << endl;
		return ;
	}
	for (itr = m_seekdata.begin(); itr != m_seekdata.end(); ++itr) {
		if ((*itr).second.used == true) {
			fprintf(f, "%s:%i:%i:%i:%i:%i:%i:%i\n", 
				(*itr).first.c_str(),
				(*itr).second.point[0],
				(*itr).second.point[1],
				(*itr).second.point[2],
				(*itr).second.point[3],
				(*itr).second.point[4],
				(*itr).second.point[5],
				(*itr).second.point[6]);
		}
	}
	fclose(f);
}


bool		AgentFileSystem::ExecuteFileRule(const fs_file_t & file) {
	stringstream 		reportData;	
	string				title;
	bool				ret;

	if (file.logfile == true) {
		title = "logfile:" + file.path;
	} else {
		title = "file:" + file.path;
	}
	if ((ret = this->GetFileAttributes(file.path, reportData, file.logfile)) && file.hashtype != NONE) {
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
			LPTSTR		tmp;
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
				tmp = digest_done(dig);
				digstr = tmp;
				if (tmp) free(tmp);
				reportData << digstr << endl;
			}
		}
	}
	reportData << endl;
	m_mgr.ClientData(title.c_str(), reportData.str().c_str());
	return ret;
}

void		AgentFileSystem::ExecuteRules() {
	for (std::list<fs_file_t>::iterator file_itr = m_files.begin(); file_itr != m_files.end(); ++file_itr) {
		ExecuteFileRule(*file_itr);
	}
	for (std::list<string>::iterator dir_itr = m_dirs.begin(); dir_itr != m_dirs.end(); ++ dir_itr) {
		ExecuteDirRule(*dir_itr);
	}
	// clean unused seekdata 
	// may be files are not monitored anymore
	std::map<std::string, fs_logfile_seekdata_t>::iterator	itr;
	for (itr = m_seekdata.begin(); itr != m_seekdata.end(); ++itr) {
		if ((*itr).second.used == false) {
			m_seekdata.erase(itr);
		}
	}
	for (std::list<fs_logfile_t>::iterator logfile_itr = m_logfiles.begin(); logfile_itr != m_logfiles.end(); ++logfile_itr) {
		ExecuteLogFileRule(*logfile_itr);
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
	if (m_mgr.IsCentralModeEnabled() == false) {
		m_mgr.Log(LOGLEVEL_ERROR, "filesystem agent only work with the BBWin centralized mode");
		return false;
	}
	LoadSeekData();
	return true;
}

AgentFileSystem::AgentFileSystem(IBBWinAgentManager & mgr) : m_mgr(mgr) {
}

AgentFileSystem::~AgentFileSystem() {
	SaveSeekData();
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
