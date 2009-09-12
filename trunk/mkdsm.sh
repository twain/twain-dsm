#!/bin/sh
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
export DSMMINOR=0
export DSMBUILD=7
export DSMREASON="TWAIN 2.1 features"

# Don't touch these lines...
export DSMBUILDER="good"
export DSMDIR=`printf "dsm_%02d%02d%02d" ${DSMMAJOR} ${DSMMINOR} ${DSMBUILD}`
export DSMEMAIL="TWAIN Working Group <twaindsm@twain.org>"

# Say hi...
echo "mkdsm v1.0 12-Sep-2009 [TWAIN DSM Build and Release Tool]"
echo ""

# Make sure we're root...
if [[ $(/usr/bin/id -u) -ne 0 ]] ;then
	echo "Please run this script as root (or use su or sudo)..."
	exit 1
fi

# Identify our distro...
if grep "Ubuntu 6.06" /etc/lsb-release &> /dev/null; then
	echo -e "Distro:      \033[1mUbuntu 6.06\033[0m"
	export OSNAME="ubuntu"
	export OSDIR="ubuntu_0606"
	export DSMBASE="twaindsm_${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}"
elif grep "SUSE LINUX 10.1" /etc/SuSE-release &> /dev/null; then
	echo -e "Distro:      \033[1mSuSE 10.1\033[0m"
	export OSNAME="suse"
	export OSDIR="suse_1001"
	export DSMBASE="twaindsm-${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}"
else
	echo "  mkdsm.sh failed (unsupported distro)..."
	echo ""
	echo "mkdsm failed..."
	exit 1
fi

# Make sure all our scripts are executeable...
if ! chmod u+rx mkdsm_*.sh ;then
	echo "  unable to set execute on our scripts..."
	echo ""
	echo "mkdsm failed..."
	exit 1
fi

# Clean...
if [ "$1" == "clean" ] ;then

	# Scrub the TWAIN_DSM directory tree
	if ! ./mkdsm_clean.sh; then
		echo "  mkdsm_clean.sh failed..."
		echo ""
		echo "mkdsm failed..."
		exit 1
	fi

	# All done...
	echo ""
	echo "mkdsm clean successful..."
	exit 0
fi

# Ask for confirmation...
ANSWER="N"
if [ -e TWAIN_DSM/twaindsm.spec ] ;then
	TMPVERSION=`grep "Version: " TWAIN_DSM/twaindsm.spec | sed 's/Version: //'`
	echo -e "Current DEB: \033[1m${TMPVERSION}\033[0m"
fi
if [ -e TWAIN_DSM/debian/changelog ] ;then
	TMPVERSION=`head -n1 TWAIN_DSM/debian/changelog | sed 's/twaindsm (//'| sed 's/-1.*//'`
	echo -e "Current RPM: \033[1m${TMPVERSION}\033[0m"
fi
echo -e "New Version: \033[1m${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}\033[0m"
echo -e "New Reason:  \033[1m${DSMREASON}\033[0m"
while ( read -t 0 ANSWER ) ; do echo &> /dev/null ; done
read -n 1 -p "Is this correct (y/N)? " ANSWER
if ! echo "${ANSWER}" | grep -i "Y" &> /dev/null ;then
	echo ""
	echo "mkdsm aborted..."
	exit 1
fi

#
# Okay, start the real work here...
#

# Backup the TWAIN_DSM directory...
if ! ./mkdsm_backup.sh ;then
	echo "  mkdsm_backup.sh failed..."
	echo ""
	echo "mkdsm failed..."
	exit 1
fi

# Validate that we have all the stuff we need to run...
if ! ./mkdsm_validate.sh ;then
	echo "  mkdsm_validate.sh failed..."
	echo ""
	echo "mkdsm failed..."
	exit 1
fi

# Scrub the TWAIN_DSM directory tree
if ! ./mkdsm_clean.sh ;then
	echo "  mkdsm_clean.sh failed..."
	echo ""
	echo "mkdsm failed..."
	exit 1
fi

# Get rid of those pesky CRLF's...
if ! ./mkdsm_rmcrlf.sh ;then
	echo "  mkdsm_rmcrlf.sh failed..."
	echo ""
	echo "mkdsm failed..."
	exit 1
fi

# Update the change logs...
if ! ./mkdsm_editlogs.sh ;then
	echo "  mkdsm_editlogs.sh failed..."
	echo ""
	echo "mkdsm failed..."
	exit 1
fi

# Build the DSM
if ! ./mkdsm_build.sh ;then
	echo "  mkdsm_build.sh failed..."
	echo ""
	echo "mkdsm failed..."
	exit 1
fi

# We made it...
echo ""
echo "mkdsm finished successfully..."

# All done...
exit 0
