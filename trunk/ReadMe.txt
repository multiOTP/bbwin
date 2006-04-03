README file for BBWin
==============================================================

Summary
=======

BBWin is an open source implementation of a big brother hobbit windows client.


Configuration Needed 
====================

BBWin has been developed to be used on these platforms :

Windows NT 4.0 SP5 or superior
Windows 2000
Windows 2003
Windows XP

in uni or multiprocessor environments
(No 64 bits version for the moment)

BBWin is released under the GNU GPL, version 2
or later. See the licence.txt file for details.


Installation
============

Installation is really easy thanks to the native Windows Installer package available.
It permits you to deploy the BBWin software automatically on many servers if you 
are using active directory policies.


Running it
==========

BBWin is installed as a native service. You can start it from the 
Services Management console or via a command line window by
typing the command :

net start bbwin



BBWin Installation Tree
=======================

BBWin =>
	> bin : BBwin binaries, DLL agents, externals scripts
	> etc : configuration files for BBWin and externals
	> doc : documentation for BBWin and externals
	> sdk : if installed, C++ files to develop your owns agents
	> tmp : temp directory


Registry Information
====================

BBWin registers 3 keys in the registry for important paths :
(Installed automatically by the MSI package)

HKEY_LOCAL_MACHINE\SOFTWARE\BBWin\binPath : specify the directory where are located
agents dll, externals scripts and bbwin binaries. When BBWin started, this directory 
becomes its current directory.

HKEY_LOCAL_MACHINE\SOFTWARE\BBWin\etcPath : specify the directory for temp files.
It is currently used by the externals agent for example.

HKEY_LOCAL_MACHINE\SOFTWARE\BBWin\tmpPath : specify the directory for temp files.
It is currently used by the externals agent for example.

BBWin can't start if one of these keys are deleted.


Client configuration
====================

BBWin configuration is located in the etc directory of the BBWin installation tree.
BBWin configuration is a simple XML file named BBWin.cfg.
BBWin can't start if the BBWin file is not present or is invalid.

BBWin.cfg :
<?xml version="1.0" encoding="utf-8" ?>
<configuration>
<bbwin>
	<setting name="hostname" value="yourhostname" />
	<setting name="bbdisplay" value="yourfirstbbdisplay" />
	<!-- <setting name="bbdisplay" value="yoursecondbbdisplay:port" />-->
	<setting name="timer" value="5m" />
	<load name="externals" value="externals.dll"/>
	<load name="procs" value="procs.dll"/>
	<load name="uptime" value="uptime.dll" timer="10m" />
	<setting name="loglevel" value="3" />
	<setting name="logpath" value="C:\BBWin.log"/>
</bbwin>

Before starting BBWin for the first time, you should configure 2 settings:

- the hostname setting : insert the hostname of the machine as written 
in your hobbit os configuration file

- the bbdisplay setting : the format is bbdisplay:port.
If port is omitted, port 1984 is used.
bbdisplay can be an IP or a DNS entry.

You can specify several bbdisplay setting if you are reporting on
several hobbit servers.

If no hostname or no bbdisplay are configured, BBWin won't start.

Note :
For the moment, BBWin need to be restarted if the configuration file is changed.


Why BBWin can't start !!??
==========================

If BBWin can't start or doesn't seem to report to your bbdisplay.
You should first check your application event log, BBWin will report
errors in it so you will be able to find out what is wrong in 
your configuration.

Also, you can enable the BBWin logging report feature which will
trace important events in a simple text file.

2 settings are available:

- loglevel : 0 no log, 1 errors, 2 warnings, 3 information, 4 debug
loglevel 4 is the most verbose mode.
If not configured, default loglevel is 0.

- logpath : path file of the logfile
If not configured, default logpath is "C:\BBWin.log"


Agents
======

BBWin service is in charge to load and run code from the agents.
The agents are libraries in charge of the monitoring stuffs.
If BBWin load no agent, BBWin will do nothing forever :)

In the bbwin XML node, <load /> commands are used
to tell BBWin to load agents.
Full paths can be set for load command. Example :
<load name="myagent" value="C:\MyAgent.dll"/>

Global timer setting is used to set the time interval between agents execution.
timer setting will be inherited to all agents if not overwritten in the load command.
If not set, timer is 5 minutes.

timer format : if no unit is specified, timer is understood as seconds.
However, you can specify timer like this : 
90s => 90 seconds
5m => 5 minutes
1h => 1 hour
2d => 2 days
4w => 4 weeks

Minimal timer is 5 seconds.

* Cpu
-----

Cpu is an agent used to monitor your cpu usage.

Cpu configuration namespace :

<cpu>
	<!-- If true, the agent  will always report with green status -->
	<setting name="alwaysgreen" value="false" />
	<setting name="default" warnlevel="90" paniclevel="95" delay="3" />
</cpu>

Delay is used as a count value before setting the yellow status 
and red status. It helps to refine alerts detection.

Default delay is 3
Default Warning alert is 90%
Default Panic alert is 95%


* Disk
------

Disk is an agent used to monitor your disk usage.

Disk configuration namespace :

<disk>
	<!-- If true, this setting will always report with green status -->
	<setting name="alwaysgreen" value="false" />
	<!-- Level can be given by % or size unit mb, gb, tb -->
	<setting name="default" warnlevel="85%" paniclevel="95%" />
	<setting name="C" warnlevel="70%" paniclevel="400mb" />
	<setting name="E" ignore="true" />
	<!-- If true, the agent will check remote drives -->
	<setting name="remote" value="false" />
	<!-- If true, the agent will that cd/dvdrom drives are empty -->
	<setting name="cdrom" value="false" />
</disk>

Default Warning alert is 90%
Default Panic alert is 95%


* Externals
-----------

Externals is an agent used to run your own BB scripts and send reports files to
the bbdisplay configured in BBWin.

Extenals configuration namespace :

...
<load name="externals" value="externals.dll"/>
...
<externals>
	<setting name="timer" value="3m" />
	<setting name="logstimer" value="60s" />
	<load value="cscript mybbscript.vbs" />
	<load value="memory.exe" />
	<load value="cscript wlbs.vbs" timer="15m" />
	<load value="cluster.exe" timer="90s" />
</externals>

Several settings are available to configure the externals agent behavior :

timer is used the time interval between scripts execution.
If omitted, the timer setting is inherited from the agent load timer value or from
the timer value.


* Migrate Externals from BBNt :

There are 2 ways to migrate your existing BB scripts made for bbnt to BBWin.

- Change the BBNt externals registry key to the BBWin tmp directory,
then just run the scripts from BBWin.
- The second way consists to modify all your scripts to use the tmpPath BBWin 
registry key in spite instead of the BBnt externals registry key.

Note : the current directory for the bbwin service process is the BBWin bin path.
Full paths can be set for externals load command.


* Memory
--------

Memory is an agent use to monitor your memory usage.

Memory configuration namespace :

<memory>
	<!-- If true, this setting will always report with green status -->
	<setting name="alwaysgreen" value="false" />
	<setting name="physical" warnlevel="78" paniclevel="98" />
	<setting name="page" warnlevel="70" paniclevel="90" />
	<setting name="virtual" warnlevel="78" paniclevel="90" />
</memory>

Default physical warning 100%
Default physical panic 101%
Default page warning 80%
Default page panic 90%
Default virtual warning 90%
Default virtual panic 99%


* Procs
-------

Procs is an agent to check running processes.
You have to specify the rules in the bbwin.cfg file

Example:
<procs>
	<!-- The rule bellow check that no drwtsn are running -->
	<setting name="drwtsn" rule="<1" alarmcolor="red" />
	<!-- The rule bellow check that Msn Messenger is always running :) -->
	<setting name="drwtsn" rule="msnmsgr.exe" rule="=1"/>
</procs>

Possible rules are =
=num
<=num
>=num
>num
<num

Default color is yellow.
If no rules are specified, green status is still sent.


* Sample
--------

Sample is a demonstrative agent installed with the BBWin Agent SDK.
Test name and Message can be over written via the bbwin.cfg file

<sample>
	<setting name="testname" value="newcolumntest" />
	<setting name="message" value="Hello World !" />
</sample>


* Stats
-------

Stats is an agent only used for trends purpose. It sends reports like netstat report.

No configuration needed.


* Uptime
--------

Uptime is an agent which monitor the uptime of a server.
It will warn 30 minutes if a reboot has been done. 

Default time is 30 minutes but it can be over written by the bbwin.cfg file :

Example : It will warn 1 h after the last reboot
<setting name="delay" value="1h" />

Default alarm color is yellow. It can be over written by the bbwin.cfg file :
<setting name="alarmcolor" value="red" />



BBWinCmd : the bb unix command replacement
=========================================

BBWinCmd is a command line tool which allow you to send messages to the hobbit server 
as the bb command under unix.
BBWinCmd is installed by default in the BBWin binary directory.

Type BBWinCmd with no argument to get help.


Externals
=========


BBWin delivers some nice externals :


Cluster.exe
-----------

Monitor mscs clusters. It checks resources states and current node changes.
No configuration file used.


Fsmon
-----

Fsmon is used to monitor filesystem entries as directories (be sure that a directory contains 
less than 10 files for example).
See the configuration file WLBS.cfg in etc BBWin directory.


WLBS.vbs
--------

Monitor WLBS (NLB) cluster. It checks node status depending the default status.
See the configuration file WLBS.cfg in etc BBWin directory.


Tools
=====


MigrateBBntToBBWin.vbs
----------------------

This tool helps you to migrate your current bbnt configuration to the BBWin configuration file format.

Usage :

cscript MigrateBBntToBBWin.vbs OutPutBBWin.cfg


=============

Project Sourceforge Page :
http://sourceforge.net/projects/bbwin

Author : Etienne Grignon  (etienne.grignon@gmail.com)
3 April 2006

