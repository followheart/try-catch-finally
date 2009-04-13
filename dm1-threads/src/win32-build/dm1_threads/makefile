# Microsoft Developer Studio Generated NMAKE File, Based on dm1_threads.dsp
!IF "$(CFG)" == ""
CFG=dm1_threads - Win32 Debug
!MESSAGE No configuration specified. Defaulting to dm1_threads - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "dm1_threads - Win32 Release" && "$(CFG)" != "dm1_threads - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dm1_threads.mak" CFG="dm1_threads - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dm1_threads - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "dm1_threads - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "dm1_threads - Win32 Release"

OUTDIR=.\../../lib
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\../../lib/
# End Custom Macros

ALL : "$(OUTDIR)\dm1_threads.lib"


CLEAN :
	-@erase "$(INTDIR)\event.obj"
	-@erase "$(INTDIR)\latch.obj"
	-@erase "$(INTDIR)\list.obj"
	-@erase "$(INTDIR)\monitor.obj"
	-@erase "$(INTDIR)\thread.obj"
	-@erase "$(INTDIR)\threadpool.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc70.idb"
	-@erase "$(OUTDIR)\dm1_threads.lib"
	-@erase "$(INTDIR)\dm1_threads.pch"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "../../win32api/include" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\dm1_threads.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dm1_threads.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\dm1_threads.lib" 
LIB32_OBJS= \
	"$(INTDIR)\event.obj" \
	"$(INTDIR)\latch.obj" \
	"$(INTDIR)\list.obj" \
	"$(INTDIR)\monitor.obj" \
	"$(INTDIR)\thread.obj" \
	"$(INTDIR)\threadpool.obj"

"$(OUTDIR)\dm1_threads.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dm1_threads - Win32 Debug"

OUTDIR=.\../../lib
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\../../lib/
# End Custom Macros

ALL : "$(OUTDIR)\dm1_threads.lib"


CLEAN :
	-@erase "$(INTDIR)\event.obj"
	-@erase "$(INTDIR)\latch.obj"
	-@erase "$(INTDIR)\list.obj"
	-@erase "$(INTDIR)\monitor.obj"
	-@erase "$(INTDIR)\thread.obj"
	-@erase "$(INTDIR)\threadpool.obj"
	-@erase "$(INTDIR)\vc70.idb"
	-@erase "$(INTDIR)\vc70.pdb"
	-@erase "$(OUTDIR)\dm1_threads.lib"
	-@erase "$(INTDIR)\dm1_threads.pch"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Gm /GX /ZI /Od /I "../../win32api/include" /I "../../include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\dm1_threads.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dm1_threads.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\dm1_threads.lib" 
LIB32_OBJS= \
	"$(INTDIR)\event.obj" \
	"$(INTDIR)\latch.obj" \
	"$(INTDIR)\list.obj" \
	"$(INTDIR)\monitor.obj" \
	"$(INTDIR)\thread.obj" \
	"$(INTDIR)\threadpool.obj"

"$(OUTDIR)\dm1_threads.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("makefile.dep")
!INCLUDE "makefile.dep"
!ELSE 
!MESSAGE Warning: cannot find "dm1_threads.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "dm1_threads - Win32 Release" || "$(CFG)" == "dm1_threads - Win32 Debug"
SOURCE=..\..\src\threads\event.cpp

"$(INTDIR)\event.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\threads\latch.cpp

"$(INTDIR)\latch.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\threads\list.cpp

"$(INTDIR)\list.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\threads\monitor.cpp

"$(INTDIR)\monitor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\threads\thread.cpp

"$(INTDIR)\thread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\threads\threadpool.cpp

"$(INTDIR)\threadpool.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

