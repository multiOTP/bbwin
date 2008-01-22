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


enum hash_type_t { NONE, MD5, SHA1 };

// Struct used for file monitoring
typedef struct			fs_file_s {
	std::string			path;
	enum hash_type_t	hashtype;
}						fs_file_t;

class AgentFileSystem : public IBBWinAgent
{
	private :
		std::list<fs_file_t>				m_files;
		std::list<std::string>				m_dirs;

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

	public :
		AgentFileSystem(IBBWinAgentManager & mgr) ;
		bool Init();
		void Run();
};


#endif 	// !__FILESYSTEM_H__

