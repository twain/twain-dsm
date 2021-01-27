#!/bin/bash
#
# Build and release the TWAIN DSM on Ubuntu 6.06 or SuSE 10.1.
#
# To run this script do the following:
#	1) make sure you have the latest DSM code
#	2) check the *** Configuration *** data below
#	3) change to root with su or sudo
#	4) ./mkdsm.sh
#
# To clean the directory of temporary files:
#	./mkdsm.sh clean
#
# History:
#
# 2.0 mlm 27-Oct-2017
# Linux 64-bit and Mac OS X...
#
# 1.0 mlm 12-Sep-2009
# Initial version...
#

#
# *** Configuration ***
#
# Set this section with the version info and the reason
# for the build.  The rest will happen automatically.
# The script will detect if a version has already been
# configured, in which case it won't update any of the
# files like changelog or twaindsm.spec...
#
export DSMMAJOR=2
export DSMMINOR=4
export DSMBUILD=3
export DSMREASON="use _pAppId for DAT_IDENTITY/MSG_GET for Linux and Mac"

# Don't touch these lines...
export DSMBUILDER="good"
export DSMDIR=`printf "dsm_%02d%02d%02d" ${DSMMAJOR} ${DSMMINOR} ${DSMBUILD}`
export DSMDIRRPM=`printf "dsm_%02d%02d%02d" ${DSMMAJOR} ${DSMMINOR} ${DSMBUILD}`
export DSMEMAIL="TWAIN Working Group <twaindsm@twain.org>"

# Say hi...
/bin/echo "mkdsm v3.1 27-Jun-2021 [TWAIN DSM Build and Release Tool]"
/bin/echo ""

# Make sure we're root...
if [ $(/usr/bin/id -u) -ne 0 ] ;then
	/bin/echo "Please run this script as root (or use su or sudo)..."
	exit 1
fi

# Identify our machine...
UMACHINE=`uname -m`
if [ "${UMACHINE}" == "x86_64" ]; then
        export MACHINE="amd64"
        export MACHINERPM="x86_64"
elif [ "${UMACHINE}" == "mips64" ]; then
	if [ "${OSTARGET}" == "" ]; then
		OSTARGET=mips64el
	fi
	export OSTARGET
	if [ "${OSTARGET}" == "mips64el" ]; then
        	export MACHINE="mips64el"
        	export MACHINERPM="mips64el"
	else
        	export MACHINE="mipsel"
        	export MACHINERPM="mipsel"
	fi
else
        export MACHINE="i386"
        export MACHINERPM="i386"
fi

# Identify our distro...
if grep "Ubuntu 6.06" /etc/lsb-release &> /dev/null; then
	export BOLD="[1m"
	export NORM="[0m"
	export ECHO="/bin/echo -e"
	${ECHO} "Distro:      ${BOLD}Ubuntu 6.06${NORM}"
	export OSNAME="ubuntu"
	export OSDIR="ubuntu_0606"
	export OSDIRRPM="suse_1001"
	export DSMBASE="twaindsm_${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}"
	export DSMBASERPM="twaindsm-${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}"
	export DEBUILD="debuild"
elif grep "Ubuntu 8.04" /etc/lsb-release &> /dev/null; then
	export BOLD="[1m"
	export NORM="[0m"
	export ECHO="/bin/echo -e"
	${ECHO} "Distro:      ${BOLD}Ubuntu 8.04${NORM}"
	export OSNAME="ubuntu"
	export OSDIR="ubuntu_0804"
	export OSDIRRPM="suse_1101"
	export DSMBASE="twaindsm_${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}"
	export DSMBASERPM="twaindsm-${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}"
	export DEBUILD="debuild -d --no-tgz-check -us -uc"
elif grep "Ubuntu 10.04" /etc/lsb-release &> /dev/null; then
	export BOLD="[1m"
	export NORM="[0m"
	export ECHO="/bin/echo -e"
	${ECHO} "Distro:      ${BOLD}Ubuntu 10.04${NORM}"
	export OSNAME="ubuntu"
	export OSDIR="ubuntu_1004"
	export OSDIRRPM="suse_1201"
	export DSMBASE="twaindsm_${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}"
	export DSMBASERPM="twaindsm-${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}"
	export DEBUILD="debuild -d --no-tgz-check -us -uc"
elif grep "NeoKylin" /etc/system-release &> /dev/null; then
	export BOLD="[1m"
	export NORM="[0m"
	export ECHO="/bin/echo -e"
	${ECHO} "Distro:      ${BOLD}NeoKylin 7.2 (${OSTARGET})${NORM}"
	export OSNAME="neokylin"
	export OSDIR="neokylin_0702"
	export DSMBASE="twaindsm-${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}"
elif grep "Kylin V10" /etc/os-release &> /dev/null; then
	export BOLD="[1m"
	export NORM="[0m"
	export ECHO="/bin/echo -e"
	${ECHO} "Distro:      ${BOLD}Kylin 10 (${OSTARGET})${NORM}"
	export OSNAME="kylin"
	export OSDIR="kylin_10"
	export DSMBASE="twaindsm-${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}"
	export DEBUILD="debuild -d --no-tgz-check -us -uc"
elif grep "SUSE LINUX 10.1" /etc/SuSE-release &> /dev/null; then
	export BOLD="[1m"
	export NORM="[0m"
	export ECHO="/bin/echo -e"
	${ECHO} "Distro:      ${BOLD}SuSE 10.1${NORM}"
	export OSNAME="suse"
	export OSDIR="suse_1001"
	export DSMBASE="twaindsm-${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}"
elif uname -a | grep "Darwin Kernel Version 11" &> /dev/null; then
	export BOLD="[1m"
	export NORM="[0m"
	export ECHO="/bin/echo"
	${ECHO} "Distro:      ${BOLD}Mac OS X 10.7${NORM}"
	export OSNAME="macosx"
	export OSDIR="macosx_1007"
	export DSMBASE="twaindsm-${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}"
else
	export ECHO=/bin/echo
	${ECHO} "  mkdsm.sh failed (unsupported distro)..."
	${ECHO} ""
	${ECHO} "mkdsm failed..."
	exit 1
fi

# Make sure all our scripts are executeable...
if ! chmod u+rx mkdsm_*.sh ;then
	${ECHO} "  unable to set execute on our scripts..."
	${ECHO} ""
	${ECHO} "mkdsm failed..."
	exit 1
fi

# Figure out what we're using to change from CRLF to LF...
export DOS2UNIX=dos2unix
if ! which dos2unix &> /dev/null; then
        export DOS2UNIX=fromdos
        if ! which fromdos &> /dev/null; then
                export DOS2UNIX="perl -pi -e 's/\r\n|\n|\r/\n/g'"
                if ! which perl &> /dev/null; then
                        echo "  Please install 'dos2unix' or 'tofrodos'..."
                        exit 1
                fi
        fi
fi

# Repair files in our topmost folder, we need this because
# git likes to 'fix' our files for us...
if [ "${OSNAME}" == "ubuntu" ] ;then
        ${DOS2UNIX} -p mkdsm_*.sh
elif [ "${OSNAME}" == "kylin" ] ;then
        ${DOS2UNIX} -q -k -o mkdsm_*.sh
elif [ "${OSNAME}" == "debian" ] ;then
        ${DOS2UNIX} -p mkdsm_*.sh
elif [ "$OSNAME" == "neokylin" ] ;then
        ${DOS2UNIX} -q -k -o mkdsm_*.sh
elif [ "$OSNAME" == "suse" ] ;then
        ${DOS2UNIX} -q -k -o mkdsm_*.sh
elif [ "$OSNAME" == "macosx" ] ;then
        ${DOS2UNIX} mkdsm_*.sh
else
        ${ECHO} "unsupported os: ${OSNAME}"
        exit 1
fi

# Clean...
if [ "$1" == "clean" ] ;then

	# Scrub the TWAIN_DSM directory tree
	if ! ./mkdsm_clean.sh $2; then
		${ECHO} "  mkdsm_clean.sh failed..."
		${ECHO} ""
		${ECHO} "mkdsm failed..."
		exit 1
	fi

	# All done...
	${ECHO} ""
	${ECHO} "mkdsm clean successful..."
	exit 0
fi

# Ask for confirmation...
ANSWER="N"
if [ "${OSNAME}" == "macosx" ]; then
	if [ -e TWAIN_DSM/Info.plist ] ;then
		TMPVERSION=`/usr/libexec/PlistBuddy -c 'Print CFBundleVersion' 'TWAIN_DSM/Info.plist'`
		${ECHO} "Current Mac: ${BOLD}${TMPVERSION}${NORM}"
	fi
else
	if [ -e TWAIN_DSM/twaindsm.spec ] ;then
		TMPVERSION=`grep "Version: " TWAIN_DSM/twaindsm.spec | sed 's/Version: //'`
		${ECHO} "Current DEB: ${BOLD}${TMPVERSION}${NORM}"
	fi
	if [ -e TWAIN_DSM/debian/changelog ] ;then
		TMPVERSION=`head -n1 TWAIN_DSM/debian/changelog | sed 's/twaindsm (//'| sed 's/-1.*//'`
		${ECHO} "Current RPM: ${BOLD}${TMPVERSION}${NORM}"
	fi
fi
${ECHO} "New Version: ${BOLD}${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}${NORM}"
${ECHO} "New Reason:  ${BOLD}${DSMREASON}${NORM}"
while ( read -t 0 ANSWER ) ; do /bin/echo &> /dev/null ; done
read -n 1 -p "Is this correct (y/N)? " ANSWER
if ! ${ECHO} "${ANSWER}" | grep -i "Y" &> /dev/null ;then
	${ECHO}
	${ECHO} "mkdsm aborted..."
	exit 1
fi

#
# Okay, start the real work here...
#

# Backup the TWAIN_DSM directory...
if ! ./mkdsm_backup.sh ;then
	${ECHO} "  mkdsm_backup.sh failed..."
	${ECHO} ""
	${ECHO} "mkdsm failed..."
	exit 1
fi

# Validate that we have all the stuff we need to run...
if ! ./mkdsm_validate.sh ;then
	${ECHO} "  mkdsm_validate.sh failed..."
	${ECHO} ""
	${ECHO} "mkdsm failed..."
	exit 1
fi

# Scrub the TWAIN_DSM directory tree
if ! ./mkdsm_clean.sh ;then
	${ECHO} "  mkdsm_clean.sh failed..."
	${ECHO} ""
	${ECHO} "mkdsm failed..."
	exit 1
fi

# Get rid of those pesky CRLF's...
if ! ./mkdsm_rmcrlf.sh ;then
	${ECHO} "  mkdsm_rmcrlf.sh failed..."
	${ECHO} ""
	${ECHO} "mkdsm failed..."
	exit 1
fi

# Update the change logs...
if ! ./mkdsm_editlogs.sh ;then
	${ECHO} "  mkdsm_editlogs.sh failed..."
	${ECHO} ""
	${ECHO} "mkdsm failed..."
	exit 1
fi

# Build the DSM for macos...
if [ "${OSNAME}" == "macosx" ]; then
	if ! ./mkdsm_macos.sh ;then
		${ECHO} "  mkdsm_macos.sh failed..."
		${ECHO} ""
		${ECHO} "mkdsm failed..."
		exit 1
	fi

# Build the DSM for linux...
elif ! ./mkdsm_build.sh ;then
	${ECHO} "  mkdsm_build.sh failed..."
	${ECHO} ""
	${ECHO} "mkdsm failed..."
	exit 1
fi

# We made it...
${ECHO} ""
${ECHO} "mkdsm finished successfully..."

# All done...
exit 0
