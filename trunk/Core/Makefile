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

DIRS = \
    Agents\~   		\
	
!include <sdkpropbld.mak>
	
all: $(OUTDIR) $(OUTDIR)\BBWin.exe $(OUTDIR)\BBWinCmd.exe $(DIRS)

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

cflags = $(cflags) /EHsc 

mc = mc.exe

.cpp{$(OUTDIR)}.obj:
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $**


## BBWin dependencies

bbwinmessage_objs = 	BBWinMessages.h \
						BBWinMessages.rc\

bbwin_objs = 	$(OUTDIR)\BBWin.obj \
				$(OUTDIR)\BBWinService.obj \
				$(OUTDIR)\ou_thread.obj \
				$(OUTDIR)\BBWinHandler.obj \
				$(OUTDIR)\Logging.obj \
				$(OUTDIR)\Utils.obj \
				$(OUTDIR)\BBWinConfig.obj \
				$(OUTDIR)\BBWinNet.obj \
				$(OUTDIR)\BBWinAgentManager.obj \
				$(OUTDIR)\tinystr.obj \
				$(OUTDIR)\tinyxml.obj \
				$(OUTDIR)\tinyxmlerror.obj \
				$(OUTDIR)\tinyxmlparser.obj

$(bbwinmessage_objs): BBWinMessages.mc
			$(mc) -A BBWinMessages.mc			

$(OUTDIR)\BBWin.res: BBWin.rc
	$(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\BBWin.res BBWin.rc

$(OUTDIR)\BBWinMessages.res: $(bbwinmessage_objs)
	$(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\BBWinMessages.res BBWinMessages.rc
	
$(OUTDIR)\BBWin.exe: $(OUTDIR)\BBWinMessages.res $(OUTDIR)\BBWin.res $(bbwin_objs)
    $(link) $(ldebug) $(bbwin_objs) $(OUTDIR)\BBWin.res $(OUTDIR)\BBWinMessages.res \
    -out:$(OUTDIR)\BBWin.exe  $(conlibsmt) advapi32.lib User32.lib
	if not exist Release mkdir Release
	copy /Y $(OUTDIR)\BBWin.exe Release
	
## BBwinCmd dependencies

bbwincmd_objs =		$(OUTDIR)\BBWinCmd.obj \
					$(OUTDIR)\BBWinNet.obj \

$(OUTDIR)\BBWincmd.res: BBWincmd.rc
	$(rc) $(rcflags) $(rcvars) /fo $(OUTDIR)\BBWincmd.res BBWincmd.rc

$(OUTDIR)\BBWinCmd.exe: $(bbwincmd_objs) $(OUTDIR)\BBWincmd.res
    $(link) $(ldebug) $(conlflags) -out:$(OUTDIR)\BBWinCmd.exe $(bbwincmd_objs) $(OUTDIR)\BBWincmd.res $(conlibsmt)
	if not exist Release mkdir Release
	copy /Y $(OUTDIR)\BBWinCmd.exe Release

$(DIRS):
!include <sdkbld.mak>
	
clean:
		erase /Q /S BBWinMessages.h BBWinMessages.rc
        $(CLEANUP)
