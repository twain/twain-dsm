TWAIN Data Source Manager [DSM], 
compliant with version 2.2 of the TWAIN specification. 

[Windows] 
The TWAIN DSM is a shared library named TWAINDSM.DLL.  There is a 32bit and a 
64bit version of this file.  TWAINDSM.DLL is installed in the Windows System 
directory (normally C:\Windows\System32).  If installing the 32bit file on a 
64bit system, make sure it ends up in the WOW64 System directory (normally 
C:\Windows\SysWow64) 

The DSM looks for data sources in C:\Windows\twain_32 or C:\Windows\twain_64. 
Data sources are also shared objects, but they have a .ds extension. 
ex: datasource.ds 

The DSM walks the twain_[32|64] directory and LoadLibrary each .ds 
file it finds, then uses GetProcAddress to locate the DS_Entry function. 
  
The DSM looks for the environment variable, TWAINDSM_LOG, for the location of 
the log file to write to. If the environment variable is not set, then no log 
is kept. Here is an example of setting the environment variable from a 
command prompt: 

  To send to a file: 
  set TWAINDSM_LOG=/tmp/twain.log 
  
The source code is documented using the Doxygen documentation system. 
  
Please refer to the TWAIN spec from http://www.TWAIN.org for further details 
on TWAIN. 

CMake is used to generate the makefiles. You can get a copy of this free 
from http://www.cmake.org. Or use the provided Visual Studio project file. 
To prevent hook code from causing an exception on some systems with DEP 
turned on, link the 32bit version with the /NXCOMPAT:NO flag.

Use the TWAINDSM merge module with installations of TWAIN applications and 
data sources to distribute the TWAINDSM.dll. The 64bit merge module, 
TWAINDSM64.msm, contains both the 32bit and 64bit versions of the DSM.

[Linux] 
The TWAIN DSM is a shared object named libtwaindsm.so that is installed 
in /usr/local/lib. 

The DSM looks for data sources in /usr/local/lib/twain.  Data sources are 
also shared objects, but they have a .ds extension.  ex: datasource.ds 

The DSM walks the /usr/local/lib/twain directory and dlopens each .ds 
file it finds, then uses dlsym to locate the DS_Entry function. 

The DSM looks for the environment variable, TWAINDSM_LOG, for the location of 
the log file to write to. If the environment variable is not set, then no log 
is kept. Here are some examples of setting the environment variable from a 
bash shell: 

  To send to a file: 
  export TWAINDSM_LOG=/tmp/twain.log 
  
  To send to the console: 
  export TWAINDSM_LOG=/dev/stdout 

The source code is documented using the Doxygen documentation system. 

There is a file named doc/fhs-2.3.pdf included in this distribution. It is the 
current filesytem hierarchy standard (FHS). The FHS was adhered to for Linux, 
but not for Solaris. 

Please refer to the TWAIN spec from http://www.TWAIN.org for further details 
on TWAIN. 

CMake is used to generate the makefiles. You can get a copy of this free 
from http://www.cmake.org. 


Useful Links 
------------ 
TWAIN.org website: 
http://www.TWAIN.org 

CMake: 
http://www.cmake.org 

Doxygen: 
http://www.stack.nl/~dimitri/doxygen/index.html 


- original: fredh@jflinc.com, May 14, 2005 
- updated:  jimw@jflinc.com, Dec 10, 2007 
- updated:  mlm, Sep 12, 2008
- updated:  pbp, Mar 14, 2012
