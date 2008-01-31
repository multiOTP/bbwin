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

#ifndef		__FILESYSTEM_H__
#define		__FILESYSTEM_H__

#include "IBBWinAgent.h"

#define MAX_TIME_BACKQUOTED_COMMAND		8000
#define LOG_MAXDATA						10240
#define MAX_SEEKDATA_POINT				6

enum hash_type_t { NONE, MD5, SHA1 };

// Struct used to save seek points 
typedef struct				fs_logfile_seekdata_s {
	DWORD					point[MAX_SEEKDATA_POINT];
	bool					used;	// tell if the logfile is still monitored 
									//and that seek data are still necessary
}							fs_logfile_seekdata_t;

// Struct used for log file monitoring
typedef struct				fs_logfile_s {
	std::string				path;
	DWORD					maxdata;
	std::list<std::string>	ignores;
	std::list<std::string>	triggers;
}							fs_logfile_t;

// Struct used for linecount monitoring
typedef struct				fs_linecount_s {
	std::string				path;
	std::list<std::string>	keywords;
}							fs_linecount_t;


// Struct used for file monitoring
typedef struct			fs_file_s {
	std::string			path;
	enum hash_type_t	hashtype;
	bool				logfile; // true if it is a logfile
}						fs_file_t;

class AgentFileSystem : public IBBWinAgent
{
	private :
		std::list<fs_file_t>							m_files;
		std::list<std::string>							m_dirs;
		std::list<fs_logfile_t>							m_logfiles;
		std::map<std::string, fs_logfile_seekdata_t>	m_seekdata;

	private :
		IBBWinAgentManager 		& m_mgr;
		std::string				m_testName;
		bool					InitCentralMode();
		void					ExecuteRules();

		void					ExecuteFileRule(const fs_file_t & file);
		bool					GetFileAttributes(const std::string & path, std::stringstream & reportData);
		bool					GetTimeString(const FILETIME & ftime, std::string & output);
		void					ExecuteDirRule(const std::string & dir);
		void					ListFiles(const std::string & path, std::stringstream & report, __int64 & size);
		void					GetLinesFromCommand(const std::string & command, std::list<std::string> & list);
		void					ExecuteLogFileRule(const fs_logfile_t & logfile);
		bool					GetDataFromLogFile(const fs_logfile_t & logfile, std::stringstream data);

	public :
		AgentFileSystem(IBBWinAgentManager & mgr) ;
		bool Init();
		void Run();
};


#endif 	// !__FILESYSTEM_H__

