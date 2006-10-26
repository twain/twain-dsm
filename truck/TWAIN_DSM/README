May 14, 2005 <fredh@jflinc.com>
===============================
The TWAIN DSM is a shared object named libtwaindsm.so that gets installed
into the /usr/local/lib directory.

The DSM looks for data sources in the /usr/local/lib/twain directory. 
Datasources are also shared objects, but they have a .ds extension. 
ex: datasource.ds

The DSM walks the /usr/local/lib/twain directory and dlopens each .ds 
file it finds.

The DSM looks for the environment variable, TWAIN_LOG, for the location of the
log file to write to. If the environment variable is not set, then no log is
kept. Here are some examples of setting the environment variable from a bash
shell:

To send to a file:
export TWAIN_LOG=/tmp/twain.log

To send to the console:
export TWAIN_LOG=/dev/stdout


To create an application, simply compile against twain.h and dsm.h and open the 
libtwaindsm.so library using dlopen. You will want to dlsym the following
functions:
DSM_Entry
DSM_Alloc
DSM_Free
DSM_LockMemory
DSM_UnlockMemory 

To create a data source, simply compile against twain.h and dsm.h then provide the 
mandatory DS_Entry(...) function. Data sources will also need to dlopen 
the libtwaindsm.so and dlsym the 4 memory functions, DSM_Alloc, DSM_Free, 
etc... The data source is a shared object that should be installed with 
a .ds extension into the /usr/local/lib/twain dir.

The source code is documented using the Doxygen documentation system.

There is a file named doc/fhs-2.3.pdf included in this distribution. It is the
current filesytem hierarchy standard (FHS). The FHS was adhered to for Linux,
but not for Solaris.

Please refer to the TWAIN spec from http://www.TWAIN.org for further details
on TWAIN.

QMake is used to generate the makefiles. You can get a copy of this free in any
QT Open Source distribution.  Please see http://www.trolltech.com for more info.

Installation
------------
./linux.sh (if using linux) -or- ./solaris.sh (if runnig solaris)
make
make install (this requires root)

Useful Links
------------
TWAIN.org website:
http://www.TWAIN.org

QT (makers of qmake):
http://www.trolltech.com

Doxygen:
http://www.stack.nl/~dimitri/doxygen/index.html

Sun:
http://www.sun.com

SunFreeware:
http://www.sunfreeware.com/index.html
