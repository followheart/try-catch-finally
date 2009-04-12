# Microsoft Developer Studio Project File - Name="wget" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=wget - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wget.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wget.mak" CFG="wget - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wget - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "wget - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wget - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "wget - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD CPP /nologo /MD /Ze /W2 /Gm- /Gi- /GX /Zi /Ot /Ob1 /Gf /Gy /I "..\\" /I "..\..\src" /D "_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "WINDOWS" /D "HAVE_CONFIG_H" /D SYSTEM_WGETRC=\"wgetrc\" /FD /GZ  /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "wget - Win32 Release"
# Name "wget - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\alloca.c
# End Source File
# Begin Source File

SOURCE=..\..\src\cmpt.c
# End Source File
# Begin Source File

SOURCE=..\..\src\connect.c
# End Source File
# Begin Source File

SOURCE=..\..\src\fnmatch.c
# End Source File
# Begin Source File

SOURCE="..\..\src\ftp-basic.c"
# End Source File
# Begin Source File

SOURCE="..\..\src\ftp-ls.c"
# End Source File
# Begin Source File

SOURCE="..\..\src\ftp-opie.c"
# End Source File
# Begin Source File

SOURCE=..\..\src\ftp.c
# End Source File
# Begin Source File

SOURCE=..\..\src\getopt.c
# End Source File
# Begin Source File

SOURCE=..\..\src\headers.c
# End Source File
# Begin Source File

SOURCE=..\..\src\host.c
# End Source File
# Begin Source File

SOURCE=..\..\src\html.c
# End Source File
# Begin Source File

SOURCE=..\..\src\http.c
# End Source File
# Begin Source File

SOURCE=..\..\src\init.c
# End Source File
# Begin Source File

SOURCE=..\..\src\libmain.c
# End Source File
# Begin Source File

SOURCE=..\..\src\log.c
# End Source File
# Begin Source File

SOURCE=..\..\src\md5.c
# End Source File
# Begin Source File

SOURCE=..\..\src\mswindows.c
# End Source File
# Begin Source File

SOURCE=..\..\src\netrc.c
# End Source File
# Begin Source File

SOURCE=..\..\src\rbuf.c
# End Source File
# Begin Source File

SOURCE=..\..\src\recur.c
# End Source File
# Begin Source File

SOURCE=..\..\src\retr.c
# End Source File
# Begin Source File

SOURCE=..\..\src\url.c
# End Source File
# Begin Source File

SOURCE=..\..\src\utils.c
# End Source File
# Begin Source File

SOURCE=..\..\src\version.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\src\config.h
# End Source File
# Begin Source File

SOURCE=..\..\src\connect.h
# End Source File
# Begin Source File

SOURCE=..\..\src\fnmatch.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ftp.h
# End Source File
# Begin Source File

SOURCE=..\..\src\getopt.h
# End Source File
# Begin Source File

SOURCE=..\..\src\headers.h
# End Source File
# Begin Source File

SOURCE=..\..\src\host.h
# End Source File
# Begin Source File

SOURCE=..\..\src\html.h
# End Source File
# Begin Source File

SOURCE=..\..\src\init.h
# End Source File
# Begin Source File

SOURCE=..\..\src\md5.h
# End Source File
# Begin Source File

SOURCE=..\..\src\mswindows.h
# End Source File
# Begin Source File

SOURCE=..\..\src\netrc.h
# End Source File
# Begin Source File

SOURCE=..\..\src\options.h
# End Source File
# Begin Source File

SOURCE=..\..\src\rbuf.h
# End Source File
# Begin Source File

SOURCE=..\..\src\recur.h
# End Source File
# Begin Source File

SOURCE=..\..\src\retr.h
# End Source File
# Begin Source File

SOURCE=..\..\src\sysdep.h
# End Source File
# Begin Source File

SOURCE=..\..\src\url.h
# End Source File
# Begin Source File

SOURCE=..\..\src\utils.h
# End Source File
# Begin Source File

SOURCE=..\..\src\wget.h
# End Source File
# End Group
# End Target
# End Project
