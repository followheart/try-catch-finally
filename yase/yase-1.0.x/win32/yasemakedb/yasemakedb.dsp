# Microsoft Developer Studio Project File - Name="yasemakedb" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=yasemakedb - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "yasemakedb.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "yasemakedb.mak" CFG="yasemakedb - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "yasemakedb - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "yasemakedb - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "yasemakedb - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../libxml2-2.4.15/include" /I "../wget/src" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "HAVE_CONFIG_H" /D "YASEMAKEDB" /D "USE_WGET" /D "USE_LIBXML" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 gdi32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib wget.lib libxml2_a.lib wsock32.lib user32.lib advapi32.lib /nologo /subsystem:console /machine:I386 /libpath:"../../libxml2-2.4.15/win32/dsp/libxml2" /libpath:"../../wget-1.5.3/windows/wget"

!ELSEIF  "$(CFG)" == "yasemakedb - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W2 /GX /Zi /Od /Ob1 /Gf /Gy /I "../../libxml2-2.4.15/include" /I "../wget/src" /D "_DEBUG" /D "HAVE_CONFIG_H" /D "YASEMAKEDB" /D "USE_WGET" /D "USE_LIBXML" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "USE_DL_PREFIX" /D "DEBUG" /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wget.lib libxml2_a.lib wsock32.lib kernel32.lib user32.lib advapi32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"../../libxml2-2.4.15/win32/dsp/libxml2" /libpath:"../../wget-1.5.3/windows/wget"

!ENDIF 

# Begin Target

# Name "yasemakedb - Win32 Release"
# Name "yasemakedb - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\alloc.c
# End Source File
# Begin Source File

SOURCE=..\..\src\avl3a.c
# End Source File
# Begin Source File

SOURCE=..\..\src\avl3b.c
# End Source File
# Begin Source File

SOURCE=..\..\src\bitfile.c
# End Source File
# Begin Source File

SOURCE=..\..\src\blockfile.c
# End Source File
# Begin Source File

SOURCE=..\..\src\btree.c
# End Source File
# Begin Source File

SOURCE=..\..\src\compress.c
# End Source File
# Begin Source File

SOURCE=..\..\src\dlmalloc.c
# End Source File
# Begin Source File

SOURCE=..\..\src\docdb.c
# End Source File
# Begin Source File

SOURCE=..\..\src\getconfig.c
# End Source File
# Begin Source File

SOURCE=..\..\src\getopt.c
# End Source File
# Begin Source File

SOURCE=..\..\src\getopt1.c
# End Source File
# Begin Source File

SOURCE=..\..\src\getopt_init.c
# End Source File
# Begin Source File

SOURCE=..\..\src\getword.c
# End Source File
# Begin Source File

SOURCE=..\..\src\list.c
# End Source File
# Begin Source File

SOURCE=..\..\src\locator.c
# End Source File
# Begin Source File

SOURCE=..\..\src\makedb.c
# End Source File
# Begin Source File

SOURCE=..\..\src\param.c
# End Source File
# Begin Source File

SOURCE=..\..\src\stem.c
# End Source File
# Begin Source File

SOURCE=..\..\src\util.c
# End Source File
# Begin Source File

SOURCE=..\..\src\xmlparser.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ystdio.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\src\alloc.h
# End Source File
# Begin Source File

SOURCE=..\..\src\avl3.h
# End Source File
# Begin Source File

SOURCE=..\..\src\avl3int.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bitfile.h
# End Source File
# Begin Source File

SOURCE=..\..\src\blockfile.h
# End Source File
# Begin Source File

SOURCE=..\..\src\btree.h
# End Source File
# Begin Source File

SOURCE=..\..\src\compress.h
# End Source File
# Begin Source File

SOURCE=..\..\src\dlmalloc.h
# End Source File
# Begin Source File

SOURCE=..\..\src\docdb.h
# End Source File
# Begin Source File

SOURCE=..\..\src\getconfig.h
# End Source File
# Begin Source File

SOURCE=..\..\src\getopt.h
# End Source File
# Begin Source File

SOURCE=..\..\src\getword.h
# End Source File
# Begin Source File

SOURCE=..\..\src\list.h
# End Source File
# Begin Source File

SOURCE=..\..\src\locator.h
# End Source File
# Begin Source File

SOURCE=..\..\src\makedb.h
# End Source File
# Begin Source File

SOURCE=..\..\src\param.h
# End Source File
# Begin Source File

SOURCE=..\..\src\stem.h
# End Source File
# Begin Source File

SOURCE=..\..\src\util.h
# End Source File
# Begin Source File

SOURCE=..\..\src\version.h
# End Source File
# Begin Source File

SOURCE=..\..\src\wgetargs.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32config.h
# End Source File
# Begin Source File

SOURCE=..\..\src\xmlparser.h
# End Source File
# Begin Source File

SOURCE=..\..\src\yase.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ystdio.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
