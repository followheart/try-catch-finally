The library can be built using three different sets of tools:

1. Microsoft Visual C++ .Net 7.0
2. Microsoft Visual C++ 6.0
3. Microsoft .Net Framework SDK

The .Net Framework contains the Visual C++ .Net 7.0 command line compiler, 
and standard C header files. It does not contain Win32 header files, hence in order to 
build this library, I make use of the win32 header files supplied with MinGW compiler
(http://www.mingw.org).

To build with .Net Framework, you will need the following:

.Net Framework SDK 1.0 with Service Pack 2.

The build process is as follows:

cd win32-build\dm1_threads
nmake
cd ..\test_dm1_threads
nmake

To clean the distribution execute 'nmake clean'.

Eventually, I intend to supply make files to build the library using gcc. 

Notes:

Currently, only the 1st build option supports a configuration that uses the Win32-Pthreads
library. I use this primarily to test that the library works with PThreads.If you build with
Win32-pthreads, you will have to manually copy the pthreadVSE.dll file to somewhere in your
path.

