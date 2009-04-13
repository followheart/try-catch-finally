# Microsoft Developer Studio Project File - Name="yasequery" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=yasequery - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "yasequery.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "yasequery.mak" CFG="yasequery - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "yasequery - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "yasequery - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "yasequery - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "yasequery - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "yasequery - Win32 Release"
# Name "yasequery - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\alloc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\avl3a.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\avl3b.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\bitset.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\blockfile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\boolsearch.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\btree.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\cbitfile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\collection.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\docdb.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\globals.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\htmloutput.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\list.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\postfile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\properties.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\query.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\rankedsearch.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\search.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\stem.c
# End Source File
# Begin Source File

SOURCE=..\..\src\tokenizer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\util.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\yasequery.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\ystdio.cpp
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

SOURCE=..\..\src\bitset.h
# End Source File
# Begin Source File

SOURCE=..\..\src\blockfile.h
# End Source File
# Begin Source File

SOURCE=..\..\src\boolsearch.h
# End Source File
# Begin Source File

SOURCE=..\..\src\btree.h
# End Source File
# Begin Source File

SOURCE=..\..\src\cbitfile.h
# End Source File
# Begin Source File

SOURCE=..\..\src\collection.h
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

SOURCE=..\..\src\locator.h
# End Source File
# Begin Source File

SOURCE=..\..\src\postfile.h
# End Source File
# Begin Source File

SOURCE=..\..\src\properties.h
# End Source File
# Begin Source File

SOURCE=..\..\src\query.h
# End Source File
# Begin Source File

SOURCE=..\..\src\rankedsearch.h
# End Source File
# Begin Source File

SOURCE=..\..\src\search.h
# End Source File
# Begin Source File

SOURCE=..\..\src\stem.h
# End Source File
# Begin Source File

SOURCE=..\..\src\tokenizer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\util.h
# End Source File
# Begin Source File

SOURCE=..\..\src\wgetargs.h
# End Source File
# Begin Source File

SOURCE=..\..\src\win32config.h
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
