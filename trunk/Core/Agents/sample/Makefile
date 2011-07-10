#this file is part of BBWin
#Copyright (C)2006 Etienne GRIGNON  ( etienne.grignon@gmail.com )
#
#This program is free software; you can redistribute it and/or
#modify it under the terms of the GNU General Public License
#as published by the Free Software Foundation; either
#version 2 of the License, or (at your option) any later version.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program; if not, write to the Free Software
#Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

!include <Win32.Mak>

all: sample.dll

cflags = $(cflags) /EHsc /I ../..

.cpp.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsmt) $**		
	
## sample dependencies

sample_objs = 	sample.obj

sample.res: sample.rc
	$(rc) $(rcflags) $(rcvars) /fo sample.res sample.rc
	
sample.dll: $(sample_objs) sample.res
    $(link) /DLL $(dllldebug) $(sample_objs) sample.res -out:sample.dll
# Check if this is part of the build or part of the SDK and place the sample.dll accordingly
	@if not exist .\IBBWinAgentManager.h nmake copy_build
	@if exist .\IBBWinAgentManager.h nmake copy_distro

clean:
	@erase /Q /S $(sample_objs) sample.lib sample.exp vc100.pdb sample.dll

copy_build:
	@echo Copying compiled DLL to Release...
	@if not exist ..\..\..\Release\ mkdir ..\..\..\Release
	@copy /Y sample.dll ..\..\..\Release

copy_distro:
	@echo Copying compiled DLL to .\Compiled...
	@if not exist .\Compiled\ mkdir .\Compiled
	@copy /Y sample.dll .\Compiled
