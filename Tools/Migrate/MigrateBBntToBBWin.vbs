'***********************************************************************************
'* this file is part of BBWin
'*Copyright (C)2006 Etienne GRIGNON  ( etienne.grignon@gmail.com )
'*
'*This program is free software; you can redistribute it and/or
'*modify it under the terms of the GNU General Public License
'*as published by the Free Software Foundation; either
'*version 2 of the License, or (at your option) any later version.
'*
'*This program is distributed in the hope that it will be useful,
'*but WITHOUT ANY WARRANTY; without even the implied warranty of
'*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'*GNU General Public License for more details.
'*
'*You should have received a copy of the GNU General Public License
'*along with this program; if not, write to the Free Software
'*Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
'*
'*
'* File:		MigrateBBntToBBWin.VBS
'* Created:		March 17, 2006
'* Last Modified:	May 10, 2006
'* Version:		0.5
'*
'* BBWin Version	0.9
'*
'* Main Function:  	Migrate BBnt registry to create the BBWin configuration file
'* 
'* Minimum System Requirement : Windows NT4, Scripting Installed
'*
'* Dependencies : no particular dependencies
'* 
'*
'* History
'* 2006/03/17      0.2          First try
'* 2006/04/18      0.3          Svcs migration part added, Memory Virtual changed by Memory Page
'* 2006/04/26     0.4           add bbpager support
'* 2006/05/10      0.5           add msgs support and create a registry file for the hostname setting
'*
'***********************************************************************************

' use explicit declaration
OPTION EXPLICIT


'********************************************************************
'* Globals
'********************************************************************
Dim 	shello ' Shell object to get registry value
Dim 	fso ' file system object to open the file

Dim		C_BBntRegistry 

C_BBntRegistry = "HKLM\SOFTWARE\Quest Software\BigBrother\bbnt\"

Set 	shello = WScript.CreateObject("WScript.Shell")
Set 	fso = CreateObject("Scripting.FileSystemObject")



'********************************************************************
'*
'* Sub Usage()
'* Purpose: Print usage of the PurgeFilesObject
'* Input:
'* Output:  
'*
'********************************************************************
Private Sub		Usage()
	WScript.Echo ""
	WScript.Echo "MigrateBBntToBBWin.vbs USAGE : Migrate the BBnt registry configuration to the a BBWin xml file configuration."
	WScript.Echo ""
	WScript.Echo "cscript MigrateBBntToBBWin.vbs OutPutBBWin.cfg"
	WScript.Echo ""
End Sub


'********************************************************************
'*
'* Sub buildBBWinConfiguration()
'* Purpose: buildBBWinConfiguration entry point
'* Input: confFile 		configuration file object
'* Output:  
'********************************************************************
Sub			buildBBWinConfiguration(byref confFile)
	Dim 	val
	Dim		disp
	Dim		port
	
	confFile.WriteLine("<bbwin>")
	'* Old Configuration when hostname was configured inside the BBWin.cfg
	'*val = shello.RegRead(C_BBntRegistry & "AliasName\")
	'*confFile.WriteLine("<setting name=""hostname"" value=""" & val & """ />")
	val = shello.RegRead(C_BBntRegistry & "Timer\")
	confFile.WriteLine("<setting name=""timer"" value=""" & val & """ />")
	disp = shello.RegRead(C_BBntRegistry & "BBDISPLAY\")
	port = shello.RegRead(C_BBntRegistry & "IPport\")
	Dim BBDispArray
	BBDispArray = Split(disp, ";")
	For Each disp in BBDispArray
		confFile.WriteLine("<setting name=""bbdisplay"" value=""" & disp & ":" & port & """ />")
	Next
	val = shello.RegRead(C_BBntRegistry & "SendPageAlerts\")
	If val = "Y" Then
		val = "true"
	Else
		val = "false"
	End If
	confFile.WriteLine("<setting name=""usepager"" value=""" & val & """ />")
	disp = shello.RegRead(C_BBntRegistry & "BBPAGE\")
	Dim BBPageArray
	BBPageArray = Split(disp, ";")
	For Each disp in BBPageArray
		confFile.WriteLine("<setting name=""bbpager"" value=""" & disp & ":" & port & """ />")
	Next
	val = shello.RegRead(C_BBntRegistry & "PageLevels\")
	confFile.WriteLine("<setting name=""pagerlevels"" value=""" & val & """ />")
	confFile.WriteLine("<load name=""cpu"" value=""cpu.dll"" />")
	confFile.WriteLine("<load name=""disk"" value=""disk.dll"" />")
	confFile.WriteLine("<load name=""externals"" value=""externals.dll"" />")
	confFile.WriteLine("<load name=""procs"" value=""procs.dll"" />")
	confFile.WriteLine("<load name=""memory"" value=""memory.dll"" />")
	confFile.WriteLine("<load name=""msgs"" value=""msgs.dll"" />")
	confFile.WriteLine("<load name=""stats"" value=""stats.dll"" />")
	confFile.WriteLine("<load name=""svcs"" value=""svcs.dll"" />")
	confFile.WriteLine("<load name=""uptime"" value=""uptime.dll"" />")
	val = shello.RegRead(C_BBntRegistry & "Activatelog\")
	val = "0"
	If val = "Y" Then
		val = "3"
	End If
	confFile.WriteLine("<setting name=""loglevel"" value=""" & val & """ />")
	confFile.WriteLine("<setting name=""logpath"" value=""C:\BBWin.log"" />")
	confFile.WriteLine("</bbwin>")
End Sub

'********************************************************************
'*
'* Sub buildExternalsConfiguration()
'* Purpose: buildExternalsConfiguration entry point
'* Input: confFile 		configuration file object
'* Output:  
'********************************************************************
Sub			buildExternalsConfiguration(byref confFile)
	Dim 	val
	Dim 	regEx ' regexp object 
	Dim 	match ' match results
	Dim		timer
	
	Set regEx = New RegExp
   	regEx.Pattern = "^\s*(.*)\s+/INT=(\S+)\s*$"
	regEx.IgnoreCase = True
	regEx.Global = True
	confFile.WriteLine("<externals>")
	val = shello.RegRead(C_BBntRegistry & "PluginTimer\")
	confFile.WriteLine("<setting name=""logstimer"" value=""" & val & """ />")
	val = shello.RegRead(C_BBntRegistry & "Externals\")
	Dim ExtArray
	ExtArray = split(val, ";")
	For Each val In ExtArray
		timer = "0"
		If regEx.test(val) = True Then
			Set match = regEx.Execute(val)
			timer = match(0).SubMatches(1)
			val = match(0).SubMatches(0)
		End If
		val = LCase(val)
		If InStr(val, ".vbs") And InStr(val, "cscript") = 0 Then
			val = "cscript " & val
		End If
		If timer = "0" Then
			confFile.WriteLine("<load value=""" & val & """ />")
		Else
			confFile.WriteLine("<load value=""" & val & """ timer=""" & timer & """ />")
		End If
	Next 
	confFile.WriteLine("</externals>")
End Sub


'********************************************************************
'*
'* Sub buildUpTimeConfiguration()
'* Purpose: buildUpTimeConfiguration entry point
'* Input: confFile 		configuration file object
'* Output:  
'********************************************************************
Sub			buildUpTimeConfiguration(byref confFile)
	Dim		val
	
	confFile.WriteLine("<uptime>")
	val = shello.RegRead(C_BBntRegistry & "PluginTimer\")
	confFile.WriteLine("<setting name=""delay"" value=""" & val & "m"" />")
	val = shello.RegRead(C_BBntRegistry & "CPUrebootcolor\")
	confFile.WriteLine("<setting name=""alarmcolor"" value=""" & LCase(val) & """ />")
	confFile.WriteLine("</uptime>")
End Sub


'********************************************************************
'*
'* Sub buildProcsConfiguration()
'* Purpose: buildProcsConfiguration entry point
'* Input: confFile 		configuration file object
'* Output:  
'********************************************************************
Sub			buildProcsConfiguration(byref confFile)
	Dim		val
	Dim 	regEx ' regexp object 
	Dim 	match ' match results
	Dim		color
	
	confFile.WriteLine("<procs>")
	val = shello.RegRead(C_BBntRegistry & "Procs\")
	Dim ProcsArray 
	ProcsArray = split(val, " ")
	Set regEx = New RegExp
   	regEx.Pattern = "^\s*(.*):(.*):(.*)\s*$"
	regEx.IgnoreCase = True
	regEx.Global = True
	For each val In ProcsArray
		If regEx.test(val) = True Then
			color = "yellow"
			Set match = regEx.Execute(val)
			If match(0).SubMatches(1) = "Y" Then
				color = "red"
			End If
			confFile.WriteLine("<setting name=""" & match(0).SubMatches(0) & """ rule=""=" & _
							match(0).SubMatches(2) & """ alarmcolor=""" & color & """ />")
		End If
	Next
	confFile.WriteLine("</procs>")
End Sub


'********************************************************************
'*
'* Sub buildSvcsConfiguration()
'* Purpose: buildSvcsConfiguration entry point
'* Input: confFile 		configuration file object
'* Output:  
'********************************************************************
Sub			buildSvcsConfiguration(byref confFile)
	Dim		val
	Dim 	regEx ' regexp object 
	Dim 	match ' match results
	Dim		color, state, reset
	
	confFile.WriteLine("<svcs>")
	val = shello.RegRead(C_BBntRegistry & "Services\")
	Dim SvcsArray 
	SvcsArray = split(val, ";")
	Set regEx = New RegExp
   	regEx.Pattern = "^\s*(.*):(.*):(.*):(.*)\s*$"
	regEx.IgnoreCase = True
	regEx.Global = True
	confFile.WriteLine("<setting name=""alwaysgreen"" value=""false"" />")
	confFile.WriteLine("<setting name=""alarmcolor"" value=""yellow"" />")
	confFile.WriteLine("<setting name=""autoreset"" value=""false"" />")
	For each val In SvcsArray
		If regEx.test(val) = True Then
			color = "yellow"
			state = "started"
			reset = "false"
			Set match = regEx.Execute(val)
			If match(0).SubMatches(1) = "R" Then
				color = "red"
			End If
			If match(0).SubMatches(2) = "S" Then
				state = "stopped"
			End If
			If match(0).SubMatches(3) = "Y" Then
				reset = "true"
			End If
			confFile.WriteLine("<setting name=""" & match(0).SubMatches(0) & """ value=""=" & _
							state & """ autoreset=""" & reset & """ alarmcolor=""" & color & """ />")
		End If
	Next
	confFile.WriteLine("</svcs>")
End Sub

'********************************************************************
'*
'* Sub buildCpuConfiguration()
'* Purpose: buildCpuConfiguration entry point
'* Input: confFile 		configuration file object
'* Output:  
'********************************************************************
Sub			buildCpuConfiguration(byref confFile)
	Dim		val
	Dim 	regEx ' regexp object 
	Dim 	match ' match results
	
	confFile.WriteLine("<cpu>")
	val = shello.RegRead(C_BBntRegistry & "CPUalwaysGreen\")
	Set regEx = New RegExp
   	regEx.Pattern = "^.*CPU:([0-9]*):([0-9]*).*$"
	regEx.IgnoreCase = True
	regEx.Global = True
	If val = "Y" Then
		confFile.WriteLine("<setting name=""alwaysgreen"" value=""true"" />")
	End If
	val = shello.RegRead(C_BBntRegistry & "Defaults\")
	If regEx.Test(val) = true Then
		Set match = regEx.Execute(val)
		confFile.WriteLine("<setting name=""default"" warnlevel=""" & match(0).SubMatches(0) & "%"" " & _ 
							"paniclevel=""" & match(0).SubMatches(1) & "%"" />")
	End If
	confFile.WriteLine("</cpu>")
End Sub


'********************************************************************
'*
'* Sub buildDiskConfiguration()
'* Purpose: buildDiskConfiguration entry point
'* Input: confFile 		configuration file object
'* Output:  
'********************************************************************
Sub			buildDiskConfiguration(byref confFile)
	Dim		val
	Dim 	regEx ' regexp object 
	Dim 	match ' match results
	
	confFile.WriteLine("<disk>")
	val = shello.RegRead(C_BBntRegistry & "DISKalwaysGreen\")
	Set regEx = New RegExp
   	regEx.Pattern = "^.*DISK:([0-9]*):([0-9]*).*$"
	regEx.IgnoreCase = True
	regEx.Global = True
	If val = "Y" Then
		confFile.WriteLine("<setting name=""alwaysgreen"" value=""true"" />")
	End If
	val = shello.RegRead(C_BBntRegistry & "Defaults\")
	Set match = regEx.Execute(val)
	confFile.WriteLine("<setting name=""default"" warnlevel=""" & match(0).SubMatches(0) & "%"" " & _ 
						"paniclevel=""" & match(0).SubMatches(1) & "%"" />")
	val = shello.RegRead(C_BBntRegistry & "DiskList\")
	Dim DiskArray
	DiskArray = split(val, " ")
	regEx.Pattern = "^(.):([0-9]+):([0-9]+).*$"
	For Each val In DiskArray
		If regEx.Test(val) = true Then
			Set match = regEx.Execute(val)
			confFile.WriteLine("<setting name=""" & match(0).SubMatches(0) & """ warnlevel=""" & match(0).SubMatches(1) & "%"" " & _
								" paniclevel=""" & match(0).SubMatches(2) & "%"" />")
		End If
	Next
	confFile.WriteLine("</disk>")
End Sub


'********************************************************************
'*
'* Sub buildMemoryConfiguration()
'* Purpose: buildMemoryConfiguration entry point
'* Input: confFile 		configuration file object
'* Output:  
'********************************************************************
Sub			buildMemoryConfiguration(byref confFile)
	Dim		val
	Dim 	regEx ' regexp object 
	Dim 	match ' match results
	Dim 	title
	
	confFile.WriteLine("<memory>")
	val = shello.RegRead(C_BBntRegistry & "MemLevels\")
	Dim MemArray 
	MemArray = split(val, " ")
	Set regEx = New RegExp
   	regEx.Pattern = "^(.*):([0-9]*):([0-9]*)$"
	regEx.IgnoreCase = True
	regEx.Global = True
	For Each val In MemArray
		If regEx.Test(val) = true Then
			Set match = regEx.Execute(val)
			If match(0).SubMatches(0) = "COMMIT" Then
				title = "page"
			Else
				title = "physical"
			End If
			confFile.WriteLine("<setting name=""" & title & """ warnlevel=""" & match(0).SubMatches(1) & "%"" " & _ 
							"paniclevel=""" & match(0).SubMatches(2) & "%"" />")
		End If
	Next 
	confFile.WriteLine("</memory>")
End Sub


'********************************************************************
'*
'* Sub buildMsgsConfiguration()
'* Purpose: buildMsgsConfiguration entry point
'* Input: confFile 		configuration file object
'* Output:  
'********************************************************************
Sub			buildMsgsConfiguration(byref confFile)
	Dim		val
	Dim 	regEx ' regexp object 
	Dim 	match ' match results
	
	confFile.WriteLine("<msgs>")
	val = shello.RegRead(C_BBntRegistry & "MsgLevels\")
	Dim MsgsArray 
	MsgsArray = split(val, " ")
	Set regEx = New RegExp
   	regEx.Pattern = "^(.*):(.*):(.):([0-9]+)$"
	regEx.IgnoreCase = True
	regEx.Global = True
	confFile.WriteLine("<setting name=""summary"" value=""true"" />")
	For Each val In MsgsArray
		If regEx.Test(val) = true Then
			Dim 	color, evtype, delay
			
			Set match = regEx.Execute(val)
			If match(0).SubMatches(2) = "Y" Then
				color = "red"
			Else
				color = "yellow"
			End If
			evtype = LCase(match(0).SubMatches(1))
			If evtype = "fail_audit" Then
				evtype = "fail"
			End If
			If evtype = "success_audit" Then
				evtype = "success"
			End If
			confFile.WriteLine("<match logfile=""" & LCase(match(0).SubMatches(0)) & _
							""" " & "type=""" & evtype & """ alarmcolor=""" & color & """ " & " />")
		End If
	Next 
	confFile.WriteLine("")
	confFile.WriteLine("<!-- Ignore rules -->")
	val = shello.RegRead(C_BBntRegistry & "IgnoreMsgs\")
	MsgsArray = split(val, ";")
	For Each val In MsgsArray
		confFile.WriteLine("<ignore logfile=""Application"" value=""" & val & """/>")
		confFile.WriteLine("<ignore logfile=""Security"" value=""" & val & """/>")
		confFile.WriteLine("<ignore logfile=""System"" value=""" & val & """/>")
	Next 
	confFile.WriteLine("</msgs>")
End Sub

'********************************************************************
'*
'* Sub BuildRegistryFile()
'* Purpose: BuildRegistryFile entry point
'* Input: args 		arguments
'* Output:  
'********************************************************************
Sub 	BuildRegistryFile(filename)
	Dim		regFile, val
	
	On Error Resume Next
	Err.Number = 0
	Set regFile = fso.OpenTextFile(filename & ".reg", 2, true)
	If Err.Number <> 0 Then
		WScript.Echo "Can't create registry file " & args(0)
	End If
	On Error Goto 0
	regFile.WriteLine("REGEDIT4")
	regFile.WriteLine("")
	regFile.WriteLine("[HKEY_LOCAL_MACHINE\SOFTWARE\BBWin]")
	val = shello.RegRead(C_BBntRegistry & "AliasName\")
	regFile.WriteLine("""hostname""=""" & val & """")
	regFile.Close
End Sub


'********************************************************************
'*
'* Sub MigrateBBntToBBWin()
'* Purpose: MigrateBBntToBBWin entry point
'* Input: args 		arguments
'* Output:  
'********************************************************************
Sub 	MigrateBBntToBBWin(byref args)
	Dim		confFile
	
	On Error Resume Next
	Err.Number = 0
	Set confFile = fso.OpenTextFile(args(0), 2, true)
	If Err.Number <> 0 Then
		WScript.Echo "Can't create file " & args(0)
	End If
	On Error Goto 0
	confFile.WriteLine("<?xml version=""1.0"" encoding=""utf-8"" ?>")
	confFile.WriteLine("<configuration>")
	buildBBWinConfiguration(confFile)
	buildCpuConfiguration(confFile)
	buildDiskConfiguration(confFile)
	buildExternalsConfiguration(confFile)
	buildMemoryConfiguration(confFile)
	buildMsgsConfiguration(confFile)
	buildProcsConfiguration(confFile)
	buildSvcsConfiguration(confFile)
	buildUpTimeConfiguration(confFile)
	confFile.WriteLine("</configuration>")
	confFile.Close
	BuildRegistryFile(args(0))
End Sub

'********************************************************************
'*
'* Sub Main()
'* Purpose: Entry point for the main function
'* Input:   args      The Arguments object
'* Output:  
'*
'********************************************************************
Sub			Main(byref args)
	If InStr(LCase(WScript.Fullname), "cscript") = 0 Then
		WScript.Echo "Please use cscript in a cmd window to launch MigrateBBntToBBWin script"
		WScript.Quit(1)
	End If
	If args.Count < 1 Then
		Usage()
		WScript.Quit(1)
	End If 
	call MigrateBBntToBBWin(args)
End Sub


'********************************************************************
'* Main call
'********************************************************************

Main(WScript.Arguments)

Set shello = nothing
Set fso = nothing

'********************************************************************
'*                                                                  *
'*                           End of File                            *
'*                                                                  *
'********************************************************************
