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

#include <sstream>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

#define BBWIN_AGENT_EXPORTS

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/format.hpp"

using boost::format;
using namespace boost::posix_time;
using namespace boost::gregorian;

#include "disk.h"

static const char *bbcolors[] = { "green", "yellow", "red", NULL };


static const BBWinAgentInfo_t 		diskAgentInfo =
{
	BBWIN_AGENT_VERSION,				// bbwinVersion;
	0,              					// agentMajVersion;
	1,              					// agentMinVersion;
	"disk",    					// agentName;
	"disk agent : report disk usage",        // agentDescription;
};                

static const disk_unit_t		disk_unit_table[] = 
{
	{"mb", 1048576},
	{"gb", 1073741824},
	{"tb", 1099511627776},
	{"m", 1048576},
	{"g", 1073741824},
	{"t", 1099511627776},
	{NULL, 0},
};

static const disk_type_t		disk_type_table[] = 
{
	{"UNKOWN", DRIVE_UNKNOWN},
	{"FIXED", DRIVE_FIXED},
	{"REMOTE", DRIVE_REMOTE},
	{"CDROM", DRIVE_CDROM},
	{"RAMDISK", DRIVE_RAMDISK},
	{NULL, 0},
};

const BBWinAgentInfo_t & AgentDisk::About() {
	return diskAgentInfo;
}

static const char * GetDiskTypeStr(DWORD type) {
	DWORD		inc;

	for (inc = 0; disk_type_table[inc].name; ++inc) {
		if (disk_type_table[inc].type == type)
			return disk_type_table[inc].name;
	}
	return disk_type_table[0].name;
}

bool			AgentDisk::GetSizeValue(const string & level, __int64 & val) {
	size_t		res;

	for (DWORD inc = 0; disk_unit_table[inc].name; ++inc) {
		res = level.find(disk_unit_table[inc].name);
		if (res > 0 && res < level.size()) {
			val = m_mgr.GetNbr(level.c_str()) * disk_unit_table[inc].size;
			return true;
		}
	}
	return false;
}

bool			AgentDisk::GetDisksData() {
	TCHAR 			driveString[BUFFER_SIZE + 1];
	size_t			driveStringLen;
	LPTSTR			driveName;
	BOOL 			fResult;
	UINT  			errorMode;
	DWORD			pos, count, driveType;
	disk_t			*disk;
	
	memset(driveString, 0, (BUFFER_SIZE + 1) * sizeof(TCHAR));
	driveStringLen = GetLogicalDriveStrings(BUFFER_SIZE, (LPTSTR)driveString);
	if (driveStringLen == 0) {
		return false;
	}
	count = 0;
	errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
	for (pos = 0;pos < driveStringLen; ++count)  {
		driveName = &(driveString[pos]);
		driveType = GetDriveType(driveName);
		if (driveType != DRIVE_REMOVABLE) { // no floppy 
			
			try {
				disk = new disk_t;
			} catch (std::bad_alloc ex) {
				continue ;
			}
			if (disk == NULL)
				continue ;
			ZeroMemory(disk, sizeof(*disk));
			disk->type = driveType;
			strncpy(disk->letter, driveName, 1);
			disk->letter[2] = '\0';
			fResult = GetDiskFreeSpaceEx(driveName, (PULARGE_INTEGER)&(disk->i64FreeBytesToCaller), 
				(PULARGE_INTEGER)&(disk->i64TotalBytes), (PULARGE_INTEGER)&(disk->i64FreeBytes));
			if (fResult != 0) {
				GetVolumeInformation(driveName, disk->volumeName, MAXNAMELEN, &(disk->volumeSerialNumber), 
				 &(disk->maximumComponentLength), &(disk->fileSystemFlags), disk->fileSystemName, MAXNAMELEN);
			} 
			if (disk->i64TotalBytes > 0)
				disk->percent = (DWORD)(((disk->i64TotalBytes - disk->i64FreeBytes) * 100) / disk->i64TotalBytes);
			m_disks.push_back(disk);
		} 
		pos += strlen(driveName) + 1;
	}
	SetErrorMode(errorMode);
	return true;
}

DWORD	AgentDisk::ApplyRule(disk_t & disk) {
	map<std::string, disk_rule_t *>::iterator	itr;
	disk_rule_t		*rule;
	string			letter = disk.letter;
	DWORD			color;
	
	color = GREEN;
	itr = m_rules.find(letter);
	if (itr == m_rules.end()) {
		itr = m_rules.find(DEFAULT_RULE_NAME);
	}
	rule = (*itr).second;
	if (rule->ignore == true) {
		disk.ignore = true;
		return GREEN;
	}
	// check if cdrom or remote check is on. If specific rule, so check setting is understood as true
	if ((m_checkRemote == false && disk.type == DRIVE_REMOTE && (*itr).first == DEFAULT_RULE_NAME)
			|| (m_checkCdrom == false && disk.type == DRIVE_CDROM) && (*itr).first == DEFAULT_RULE_NAME) {
		disk.ignore = true;
		return GREEN;
	}
	// warning status
	if (rule->warnUseSize == true) {
		if (rule->warnSize > disk.i64FreeBytes)
			color = YELLOW;
	} else {
		if (rule->warnPercent < disk.percent)
			color = YELLOW;
	}
	// panic status
	if (rule->panicUseSize == true) {
		if (rule->panicSize > disk.i64FreeBytes)
			color = RED;
	} else {
		if (rule->panicPercent < disk.percent)
			color = RED;
	}
	if (disk.type == DRIVE_CDROM) {
		if (disk.i64TotalBytes > 0)
			color = YELLOW;
		else {
			disk.ignore = true;
			color = GREEN;
		}
	}
	return color;
}

void	AgentDisk::ApplyRules() {
	std::list<disk_t *>::iterator 	itr;
	
	m_pageColor = GREEN;
	for (itr = m_disks.begin(); itr != m_disks.end(); ++itr) {	
		disk_t	& disk = *(*itr);
		disk.color = GREEN;
		if (disk.ignore == false) {
			disk.color = ApplyRule(disk);
			if (m_pageColor < disk.color)
				m_pageColor = disk.color;
		}
	}
}

void		AgentDisk::FreeDisksData() {
	std::list<disk_t *>::iterator 	itr;
	
	for (itr = m_disks.begin(); itr != m_disks.end(); ++itr) {
		delete (*itr);
	}
	m_disks.clear();
}

//
// used to get only 2 digits
// not very efficient for the moment
//
static	void		formatNumber(__int64 & avag) {
	while (avag > 100) {
		avag /= 10;
	}
}

void		AgentDisk::GenerateSummary(const disk_t & disk, stringstream & summary) {
	__int64		size;
	__int64		avag;
	string		unit;
	
	unit = "b";
	size = disk.i64TotalBytes;
	avag = 0;
	if ((disk.i64TotalBytes / 1048576) > 0) {
		size = disk.i64TotalBytes / 1048576;
		avag = (disk.i64TotalBytes - (1048576 * size)) / 1024;
		unit = "mb";
	}
	if ((disk.i64TotalBytes / 1073741824) > 0) {
		size = disk.i64TotalBytes / 1073741824;
		avag = (disk.i64TotalBytes - (1073741824 * size)) / 1048576;
		unit = "gb";
	}
	if ((disk.i64TotalBytes / 1099511627776) > 0) {
		size = disk.i64TotalBytes / 1099511627776;
		avag = (disk.i64TotalBytes - (1099511627776 * size)) / 1073741824;
		unit = "tb";
	}	
	formatNumber(avag);
	summary << format("%f.%02u%s") % size % avag % unit;
	summary << "\\";
	unit = "b";
	size = disk.i64FreeBytes;
	avag = 0;
	if ((disk.i64FreeBytes / 1048576) > 0) {
		size = disk.i64FreeBytes / 1048576;
		avag = (disk.i64FreeBytes - (1048576 * size)) / 1024;
		unit = "mb";
	}
	if ((disk.i64FreeBytes / 1073741824) > 0) {
		size = disk.i64FreeBytes / 1073741824;
		avag = (disk.i64FreeBytes - (1073741824 * size)) / 1048576;
		unit = "gb";
	}
	if ((disk.i64FreeBytes / 1099511627776) > 0) {
		size = disk.i64FreeBytes / 1099511627776;
		avag = (disk.i64FreeBytes - (1099511627776 * size)) / 1073741824;
		unit = "tb";
	}
	formatNumber(avag);
	summary << format("%f.%02u%s") % size % avag % unit;
}

void		AgentDisk::SendStatusReport() {
	stringstream 					reportData;	
	std::list<disk_t *>::iterator 	itr;
	disk_t							*disk;
	
    ptime now = second_clock::local_time();
	reportData << to_simple_string(now) << " [" << m_mgr.GetSetting("hostname") << "] " << endl;
	reportData << "\n" << endl;
	reportData << format("Filesystem     1K-blocks     Used       Avail    Capacity    Mounted      Summary(Total\\Avail)") << endl;
	for (itr = m_disks.begin(); itr != m_disks.end(); ++itr) {
		stringstream					summary;
		
		disk = (*itr);
		if (disk->ignore)
			continue ;
		GenerateSummary(*disk, summary);
	    reportData << format("%s             %10.0f %10.0f %10.0f   %3d%%       /%s/%s      %s") %
				disk->letter % (disk->i64TotalBytes / 1024) % ((disk->i64TotalBytes - disk->i64FreeBytes) / 1024) % 
				(disk->i64FreeBytes / 1024) % disk->percent % GetDiskTypeStr(disk->type) % 
				disk->letter % summary.str();
		reportData << " &" << bbcolors[disk->color];
		reportData << endl;
	}
	m_mgr.Status("disk", bbcolors[m_pageColor], reportData.str().c_str());
}

void 		AgentDisk::Run() {
	GetDisksData();
	ApplyRules();
	if (m_alwaysgreen == true) 
		m_pageColor = GREEN;
	SendStatusReport();
	FreeDisksData();
}

AgentDisk::AgentDisk(IBBWinAgentManager & mgr) : m_mgr(mgr) {
	m_checkRemote = false;
	m_checkCdrom = false;
	m_alwaysgreen = false;
}

void		AgentDisk::BuildRule(disk_rule_t & rule, const string & warnlevel, const string & paniclevel) {
	map<std::string, disk_rule_t *>::iterator	itr;

	itr = m_rules.find(DEFAULT_RULE_NAME);
	if (itr == m_rules.end()) {
		rule.warnUseSize = false;
		rule.panicUseSize = false;
		rule.warnPercent = m_mgr.GetNbr(DEF_DISK_WARN);
		rule.panicPercent = m_mgr.GetNbr(DEF_DISK_PANIC);
	} else {
		disk_rule_t		* defRule = (*itr).second;
		rule.warnUseSize = defRule->warnUseSize;
		rule.panicUseSize = defRule->panicUseSize;
		rule.warnPercent = defRule->warnPercent;
		rule.panicPercent = defRule->panicPercent;
	}
	if (warnlevel.length() > 0) {
		if (GetSizeValue(warnlevel, rule.warnSize))
			rule.warnUseSize = true; 
		else {
			rule.warnPercent = m_mgr.GetNbr(warnlevel.c_str());	
			rule.warnUseSize = false; 
		}
	}
	if (paniclevel.length() > 0) {
		if (GetSizeValue(paniclevel, rule.panicSize))
			rule.panicUseSize = true;
		else {
			rule.panicPercent = m_mgr.GetNbr(paniclevel.c_str());
			rule.panicUseSize = false;
		}
	}
	if (rule.warnUseSize == false && rule.panicUseSize == false && rule.panicPercent < rule.warnPercent)
		m_mgr.ReportEventWarn("panic percentage must be higher than warning percentage");
	if (rule.warnUseSize == true && rule.panicUseSize == true && rule.panicSize > rule.warnSize)
		m_mgr.ReportEventWarn("warning free space must be higher than panic free space");
	if (rule.warnUseSize == true && rule.warnSize < 0)
		m_mgr.ReportEventWarn("invalid warning free space value");
	if (rule.panicUseSize == true && rule.panicSize < 0)
		m_mgr.ReportEventWarn("invalid panic free space value");
}

void		AgentDisk::AddRule(const string & label, const string & warnlevel, const string & paniclevel,
								const string & ignore) {
	disk_rule_t		*rule;
	
	try {
		rule = new disk_rule_t;
	} catch (std::bad_alloc ex) {
		return ;
	}
	if (rule == NULL)
		return ;
	ZeroMemory(rule, sizeof(*rule));
	rule->ignore = false;
	if (ignore == "true")
		rule->ignore = true;
	BuildRule(*rule, warnlevel, paniclevel);
	if (m_rules.find(label) == m_rules.end()) {
		m_rules.insert(pair<std::string, disk_rule_t *>(label, rule));
	} else {
		string mess;
		mess = "duplicate rule " + label;
		m_mgr.ReportEventWarn(mess.c_str());
		delete rule;
	}
}

bool		AgentDisk::Init() {
	bbwinagentconfig_t		* conf = m_mgr.LoadConfiguration(m_mgr.GetAgentName());
	
	if (conf == NULL)
		return false;
	bbwinconfig_range_t * range = m_mgr.GetConfigurationRange(conf, "setting");
	if (range == NULL)
		return false;
	for ( ; range->first != range->second; ++range->first) {
		string		name, value;

		name = m_mgr.GetConfigurationRangeValue(range, "name");
		value = m_mgr.GetConfigurationRangeValue(range, "value");
		if (name == "alwaysgreen") {
			if (value == "true")
				m_alwaysgreen = true;
		} else if (name == "remote") {
			if (value == "true")
				m_checkRemote = true;
		} else if (name == "cdrom") {
			if (value == "true")
				m_checkCdrom = true;
		} else {
			AddRule(name, 
					m_mgr.GetConfigurationRangeValue(range, "warnlevel"), 
					m_mgr.GetConfigurationRangeValue(range, "paniclevel"), 
					m_mgr.GetConfigurationRangeValue(range, "ignore"));	
		}
	}	
	// if no default, create the default rule
	if (m_rules.find(DEFAULT_RULE_NAME) == m_rules.end()) {
		AddRule(DEFAULT_RULE_NAME, DEF_DISK_WARN, DEF_DISK_PANIC, "false");
	}
	m_mgr.FreeConfigurationRange(range);
	m_mgr.FreeConfiguration(conf);
	return true;
}

AgentDisk::~AgentDisk() {
	map<std::string, disk_rule_t *>::iterator		itr;
	
	for (itr = m_rules.begin(); itr != m_rules.end(); ++itr)
		delete itr->second;
	m_rules.clear();
}

BBWIN_AGENTDECL IBBWinAgent * CreateBBWinAgent(IBBWinAgentManager & mgr)
{
	return new AgentDisk(mgr);
}

BBWIN_AGENTDECL void		 DestroyBBWinAgent(IBBWinAgent * agent)
{
	delete agent;
}
