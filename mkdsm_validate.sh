#!/bin/bash
#
# Validate that we have all the stuff we need...
#

# Say hi...
echo ""
echo "Validating the environment..."
if [ "$DSMBUILDER" != "good" ] ;then
        echo "  please run ./mkdsm.sh"
        exit 1
fi


#
# Stuff that everybody needs...
#

# cmake
echo "  ...checking for cmake (linux only)"
if [ "$OSNAME" != "macosx" ] ;then
	if ! which cmake &> /dev/null; then
		echo "  Please install 'cmake'..."
		exit 1
	fi
fi

# dos2unix/fromdos
echo "  ...checking for dos2unix/dos2unix"
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
echo "  ...using ${DOS2UNIX}"

# g++
echo "  ...checking for g++"
if ! which g++ &> /dev/null; then
	echo "  Please install 'g++'..."
	exit 1
fi

# make
echo "  ...checking for make"
if ! which make &> /dev/null; then
	echo "  Please install 'build-essentials'..."
	exit 1
fi


#
# Stuff that Ubuntu and Kylin needs...
#
if [ "$OSNAME" == "ubuntu" ] || [ "$OSNAME" == "kylin" ]; then

	# debhelper
	echo "  ...checking for debhelper"
	if [ ! -e /usr/share/debhelper ]; then
		echo "  Please install 'debhelper'..."
		exit 1
	fi

	# debuild
	echo "  ...checking for debuild"
	if ! which debuild &> /dev/null; then
		echo "  Please install 'devscripts'..."
		exit 1
	fi

	# gpg
	echo "  ...checking for gpg"
	if ! which gpg &> /dev/null; then
		echo "  Please install 'gpg'..."
		exit 1
	fi


#
# Stuff that Debian needs...
#
elif [ "$OSNAME" == "debian" ]; then

	# debhelper
	echo "  ...checking for debhelper"
	if [ ! -e /usr/share/debhelper ]; then
		echo "  Please install 'debhelper'..."
		exit 1
	fi

	# debuild
	echo "  ...checking for debuild"
	if ! which debuild &> /dev/null; then
		echo "  Please install 'devscripts'..."
		exit 1
	fi

	# gpg
	echo "  ...checking for gpg"
	if ! which gpg &> /dev/null; then
		echo "  Please install 'gpg'..."
		exit 1
	fi


#
# Stuff that NeoKylin needs...
#
elif [ "$OSNAME" == "neokylin" ]; then

	# rpmbuild
	echo "  ...checking for rpmbuild"
	if ! which rpmbuild &> /dev/null; then
		echo "  Please install 'rpmbuild'..."
		exit 1
	fi


#
# Stuff that SuSE needs...
#
elif [ "$OSNAME" == "suse" ]; then

	# rpmbuild
	echo "  ...checking for rpmbuild"
	if ! which rpmbuild &> /dev/null; then
		echo "  Please install 'rpmbuild'..."
		exit 1
	fi


#
# Stuff that Mac OS X needs...
#
elif [ "$OSNAME" == "macosx" ]; then
	# xcodebuild
	echo "  ...checking for xcodebuild"
	if ! which xcodebuild &> /dev/null; then
		echo "  Please install 'xcode'..."
		exit 1
	fi


#
# Ruh-roh...
#
else
	echo "  unsupported os..." $OSNAME
	exit 1
fi


# Bye-bye...
exit 0

