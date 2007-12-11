TWAIN Data Source Manager [DSM], 
compliant with version 2.0 of the TWAIN specification.

[Windows]
The TWAIN DSM is a shared library named TWAINDSM.dll that gets installed 
into the system directory.  The 32bit version gets installed into the
/Windows/System32 directory and the 64bit version gets installed into the 
/Windows/System64 directory. 

The DSM looks for data sources in the /Windows/twain_32 or 
/Windows/twain_64 directory.  Data sources are also shared objects, but 
they have a .ds extension. ex: datasource.ds

The DSM walks the twain_[32|64] directory and LoadLibrary each .ds 
file it finds, then uses GetProcAddress to locate the DS_Entry function.

The DSM looks for the environment variable, TWAINDSM_LOG, for the location of 
the log file to write to. If the environment variable is not set, then no log 
is kept. Here are some examples of setting the environment variable from a 
command prompt:

To send to a file:
set TWAIN_LOG=/tmp/twain.log

To create an application, simply compile against twain.h and open the 
TWAINdsm.dll library using LoadLibrary. 

To create a data source, simply compile against twain.h then provide the 
mandatory DS_Entry(...) function.  The data source is a shared library that 
is installed with a .ds extension into the /Windows/twain_32 or 
/Windows/twain_64 directory.

The source code is documented using the Doxygen documentation system.

Please refer to the TWAIN spec from http://www.TWAIN.org for further details
on TWAIN.

CMake is used to generate the makefiles. You can get a copy of this free from http://www.cmake.org. 
Or use the provided Visual Studio project file.

Use the TWAINDSM merge module with installations of TWAIN applications and 
data sources to distribute the TWAINDSM.dll.

[Linux]
The TWAIN DSM is a shared object named libtwaindsm.so that gets installed
into the /usr/local/lib directory.

The DSM looks for data sources in the /usr/local/lib/twain directory. 
Data sources are also shared objects, but they have a .ds extension. 
ex: datasource.ds

The DSM walks the /usr/local/lib/twain directory and dlopens each .ds 
file it finds, then uses dlsym to locate the DS_Entry function.

The DSM looks for the environment variable, TWAINDSM_LOG, for the location of 
the log file to write to. If the environment variable is not set, then no log 
is kept. Here are some examples of setting the environment variable from a 
bash shell:

To send to a file:
export TWAIN_LOG=/tmp/twain.log

To send to the console:
export TWAIN_LOG=/dev/stdout


To create an application, simply compile against twain.h and open the 
libtwaindsm.so library using dlopen. 

To create a data source, simply compile against twain.h then provide the 
mandatory DS_Entry(...) function.  The data source is a shared object that 
is installed with a .ds extension into the /usr/local/lib/twain dir.

The source code is documented using the Doxygen documentation system.

There is a file named doc/fhs-2.3.pdf included in this distribution. It is the
current filesytem hierarchy standard (FHS). The FHS was adhered to for Linux,
but not for Solaris.

Please refer to the TWAIN spec from http://www.TWAIN.org for further details
on TWAIN.

CMake is used to generate the makefiles. You can get a copy of this free from http://www.cmake.org.


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
