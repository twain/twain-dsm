#!/bin/bash
#
# Scrub the directory tree...
#

# Say hi...
echo ""
echo "Scrubbing the TWAIN_DSM directory tree..."
if [ "$DSMBUILDER" != "good" ] ;then
        echo "  please run ./mkdsm.sh"
        exit 1
fi


#
# Stuff that everybody does...
#

# Remove .svn from every level of the directory tree...
echo "  ...removing .svn directories"
rm -rf `find . -name .svn`
if ls -R | grep .svn; then
	echo ".svn files still remain, please remove them manually..."
	exit 1
fi

# Remove everything but script files, pub and TWAIN_DSM from this directory...
echo "  ...cleaning the linuxdsm directory"
if [ "$1" == "keeprelease" ]; then
	rm -rf `ls | grep -v .sh | grep -v pub | grep -v signer | grep -v TWAIN_DSM | grep -v dsm_02`
else
	rm -rf `ls | grep -v .sh | grep -v pub | grep -v signer | grep -v TWAIN_DSM`
fi

# Remove everything but .sln files and .vcproj files from visual_studio...
echo "  ...cleaning the visual_studio directory"
pushd TWAIN_DSM/visual_studio &> /dev/null
rm -rf `ls | grep -v .sln | grep -v .vcproj`
popd &> /dev/null

# Remove everything but .cpp, .h and the specific files we call out...
echo "  ...cleaning the src directory"
pushd TWAIN_DSM/src &> /dev/null
rm -rf `ls | grep -v .cpp | grep -v .h | grep -v .rc | grep -v .def | grep -v .doc | grep -v CMakeLists.txt`
rm -f CMakeCache.txt
popd &> /dev/null

# Remove twaindsm from TWAIN_DSM/debian
echo "  ...cleaning the debian directory"
pushd TWAIN_DSM/debian &> /dev/null
rm -rf twaindsm
popd &> /dev/null


#
# Stuff that Ubuntu does...
#
if [ "$OSNAME" == "ubuntu" ] ;then

        # Cleaning /usr/local/lib/libtwaindsm.so*...
        echo "  ...cleaning /usr/local/lib/libtwaindsm.so*"
        rm -rf /usr/local/lib/libtwaindsm.so*

        # Cleaning /usr/src/rpm/BUILD...
        echo "  ...cleaning /usr/src/rpm/BUILD"
        rm -rf /usr/src/rpm/BUILD/twaindsm*

        # Cleaning /usr/src/rpm/SOURCES...
        echo "  ...cleaning /usr/src/rpm/SOURCES"
        rm -rf /usr/src/rpm/SOURCES/twaindsm*

        # Cleaning /usr/src/rpm/RPMS...
        echo "  ...cleaning /usr/src/rpm/RPMS"
        rm -rf /usr/src/rpm/RPMS/twaindsm*

        # Cleaning /usr/src/rpm/SRPMS...
        echo "  ...cleaning /usr/src/rpm/SRPMS"
        rm -rf /usr/src/rpm/SRPMS/twaindsm*

fi


#
# Stuff that SuSE does...
#
if [ "$OSNAME" == "suse" ] ;then

	# Backup /usr/local/lib/twain...
	echo "  ...backing up and cleaning /usr/local/lib/twain"
	if [ -e /usr/local/lib/twain ]; then
		mv /usr/local/lib/twain /usr/local/lib/twain.backup
		mkdir /usr/local/lib/twain
	fi

	# Cleaning /usr/local/lib/libtwaindsm.so*...
	echo "  ...cleaning /usr/local/lib/libtwaindsm.so*"
	rm -rf /usr/local/lib/libtwaindsm.so*

	# Cleaning /usr/src/packages/BUILD...
	echo "  ...cleaning /usr/src/packages/BUILD"
	rm -rf /usr/src/packages/BUILD/twaindsm*

	# Cleaning /usr/src/packages/SOURCES...
	echo "  ...cleaning /usr/src/packages/SOURCES"
	rm -rf /usr/src/packages/SOURCES/twaindsm*

	# Cleaning /usr/src/packages/RPMS...
	echo "  ...cleaning /usr/src/packages/RPMS"
	rm -rf /usr/src/packages/RPMS/twaindsm*

	# Cleaning /usr/src/packages/SRPMS...
	echo "  ...cleaning /usr/src/packages/SRPMS"
	rm -rf /usr/src/packages/SRPMS/twaindsm*

fi


# Bye-bye
exit 0
