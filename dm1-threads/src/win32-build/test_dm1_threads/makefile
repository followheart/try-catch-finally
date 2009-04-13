# Microsoft Developer Studio Generated NMAKE File, Based on test_dm1_threads.dsp
!IF "$(CFG)" == ""
CFG=test_dm1_threads - Win32 Debug
!MESSAGE No configuration specified. Defaulting to test_dm1_threads - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "test_dm1_threads - Win32 Release" && "$(CFG)" != "test_dm1_threads - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "test_dm1_threads.mak" CFG="test_dm1_threads - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "test_dm1_threads - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "test_dm1_threads - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "test_dm1_threads - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\test_dm1_threads.exe"

!ELSE 

ALL : "dm1_threads - Win32 Release" "$(OUTDIR)\test_dm1_threads.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"dm1_threads - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\test_dm1.obj"
	-@erase "$(INTDIR)\test_dm1_event.obj"
	-@erase "$(INTDIR)\test_dm1_latch.obj"
	-@erase "$(INTDIR)\test_dm1_monitor.obj"
	-@erase "$(INTDIR)\test_dm1_monitor2.obj"
	-@erase "$(INTDIR)\test_dm1_monitor3.obj"
	-@erase "$(INTDIR)\test_dm1_monitor4.obj"
	-@erase "$(INTDIR)\test_dm1_thread.obj"
	-@erase "$(INTDIR)\test_dm1_threadpool.obj"
	-@erase "$(INTDIR)\vc70.idb"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\test_dm1_threads.exe"
	-@erase "$(INTDIR)\test_dm1_threads.pch"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../../win32api/include" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\test_dm1_threads.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\test_dm1_threads.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
#LINK32_FLAGS=kernel32.lib user32.lib dm1_threads.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\test_dm1_threads.pdb" /machine:I386 /out:"$(OUTDIR)\test_dm1_threads.exe" /libpath:"../../lib" 
LINK32_FLAGS=kernel32.lib dm1_threads.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\test_dm1_threads.pdb" /machine:I386 /out:"$(OUTDIR)\test_dm1_threads.exe" /libpath:"../../lib" 
LINK32_OBJS= \
	"$(INTDIR)\test_dm1.obj" \
	"$(INTDIR)\test_dm1_event.obj" \
	"$(INTDIR)\test_dm1_latch.obj" \
	"$(INTDIR)\test_dm1_monitor.obj" \
	"$(INTDIR)\test_dm1_monitor2.obj" \
	"$(INTDIR)\test_dm1_monitor3.obj" \
	"$(INTDIR)\test_dm1_monitor4.obj" \
	"$(INTDIR)\test_dm1_thread.obj" \
	"$(INTDIR)\test_dm1_threadpool.obj" \
	"..\..\lib\dm1_threads.lib"

"$(OUTDIR)\test_dm1_threads.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "test_dm1_threads - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\test_dm1_threads.exe"

!ELSE 

ALL : "dm1_threads - Win32 Debug" "$(OUTDIR)\test_dm1_threads.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"dm1_threads - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\test_dm1.obj"
	-@erase "$(INTDIR)\test_dm1_event.obj"
	-@erase "$(INTDIR)\test_dm1_latch.obj"
	-@erase "$(INTDIR)\test_dm1_monitor.obj"
	-@erase "$(INTDIR)\test_dm1_monitor2.obj"
	-@erase "$(INTDIR)\test_dm1_monitor3.obj"
	-@erase "$(INTDIR)\test_dm1_monitor4.obj"
	-@erase "$(INTDIR)\test_dm1_thread.obj"
	-@erase "$(INTDIR)\test_dm1_threadpool.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vc70.idb"
	-@erase "$(INTDIR)\vc70.pdb"
	-@erase "$(OUTDIR)\test_dm1_threads.exe"
	-@erase "$(OUTDIR)\test_dm1_threads.ilk"
	-@erase "$(OUTDIR)\test_dm1_threads.pdb"
	-@erase "$(INTDIR)\test_dm1_threads.pch"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Gm /GX /ZI /Od /I "../../win32api/include" /I "../../include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\test_dm1_threads.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\test_dm1_threads.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
#LINK32_FLAGS=kernel32.lib user32.lib dm1_threads.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\test_dm1_threads.pdb" /debug /machine:I386 /out:"$(OUTDIR)\test_dm1_threads.exe" /pdbtype:sept /libpath:"../../lib" 
LINK32_FLAGS=kernel32.lib dm1_threads.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\test_dm1_threads.pdb" /debug /machine:I386 /out:"$(OUTDIR)\test_dm1_threads.exe" /pdbtype:sept /libpath:"../../lib" 
LINK32_OBJS= \
	"$(INTDIR)\test_dm1.obj" \
	"$(INTDIR)\test_dm1_event.obj" \
	"$(INTDIR)\test_dm1_latch.obj" \
	"$(INTDIR)\test_dm1_monitor.obj" \
	"$(INTDIR)\test_dm1_monitor2.obj" \
	"$(INTDIR)\test_dm1_monitor3.obj" \
	"$(INTDIR)\test_dm1_monitor4.obj" \
	"$(INTDIR)\test_dm1_thread.obj" \
	"$(INTDIR)\test_dm1_threadpool.obj" \
	"..\..\lib\dm1_threads.lib"

"$(OUTDIR)\test_dm1_threads.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("makefile.dep")
!INCLUDE "makefile.dep"
!ELSE 
!MESSAGE Warning: cannot find "makefile.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "test_dm1_threads - Win32 Release" || "$(CFG)" == "test_dm1_threads - Win32 Debug"
SOURCE=..\..\test\test_dm1.cpp

"$(INTDIR)\test_dm1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\test\test_dm1_event.cpp

"$(INTDIR)\test_dm1_event.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\test\test_dm1_latch.cpp

"$(INTDIR)\test_dm1_latch.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\test\test_dm1_monitor.cpp

"$(INTDIR)\test_dm1_monitor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\test\test_dm1_monitor2.cpp

"$(INTDIR)\test_dm1_monitor2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\test\test_dm1_monitor3.cpp

"$(INTDIR)\test_dm1_monitor3.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\test\test_dm1_monitor4.cpp

"$(INTDIR)\test_dm1_monitor4.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\test\test_dm1_thread.cpp

"$(INTDIR)\test_dm1_thread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\test\test_dm1_threadpool.cpp

"$(INTDIR)\test_dm1_threadpool.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!IF  "$(CFG)" == "test_dm1_threads - Win32 Release"

"dm1_threads - Win32 Release" : 
   cd "..\dm1_threads"
   $(MAKE) /$(MAKEFLAGS) CFG="dm1_threads - Win32 Release" 
   cd "..\test_dm1_threads"

"dm1_threads - Win32 ReleaseCLEAN" : 
   cd "..\dm1_threads"
   $(MAKE) /$(MAKEFLAGS) CFG="dm1_threads - Win32 Release" RECURSE=1 CLEAN 
   cd "..\test_dm1_threads"

!ELSEIF  "$(CFG)" == "test_dm1_threads - Win32 Debug"

"dm1_threads - Win32 Debug" : 
   cd "..\dm1_threads"
   $(MAKE) /$(MAKEFLAGS) CFG="dm1_threads - Win32 Debug" 
   cd "..\test_dm1_threads"

"dm1_threads - Win32 DebugCLEAN" : 
   cd "..\dm1_threads"
   $(MAKE) /$(MAKEFLAGS) CFG="dm1_threads - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\test_dm1_threads"

!ENDIF 


!ENDIF 

