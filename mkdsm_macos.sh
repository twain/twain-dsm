#!/bin/bash
#
# Build the DSM on macOS, and create a package file, which
# is wrapped in a disk image, and then compress it.  We'll
# put the finished result on the desktop...
#

# Say hi...
echo ""
echo "Build the DSM..."

# Init stuff...
VERSION=`printf "%02d.%02d.%02d.00" "${DSMMAJOR}" "${DSMMINOR}" "${DSMBUILD}"`
SIGNER=`cat signer`
SCRIPTDIR=$( cd $(dirname "${BASH_SOURCE}"); pwd )
BLDDIR="${SCRIPTDIR}/TWAIN_DSM/build/Release"
TMPDIR="/private/tmp/twaindsm"
PKGDIR="${TMPDIR}/pkg"
DSTDIR="${TMPDIR}/dst"
DMGDIR="${TMPDIR}/dmg"
DESKTOPDIR="${HOME}/Desktop"

# Get the folders ready...
rm -rf "${TMPDIR}" >/dev/null 2>&1
rm "${DESKTOPDIR}/twaindsm-${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}.dmg.gz" >/dev/null 2>&1
mkdir -p "${TMPDIR}"
mkdir -p "${PKGDIR}"
mkdir -p "${DSTDIR}"
mkdir -p "${DMGDIR}"

# Build the project, drop into it to do this...
echo "run xcodebuild..."
pushd "${SCRIPTDIR}/TWAIN_DSM" >/dev/null 2>&1
xcodebuild clean build | egrep '^(/.+:[0-9+:[0-9]+:.(error|warning):|fatal|===)'
popd >/dev/null 2>&1

# Create the package folder, and the rooted path to where
# we'll be installing it on the target machines, and then
# copy our stuff over...
echo "build the package folder..."
mkdir -p "${PKGDIR}/Library/Frameworks"
cp -R "${BLDDIR}/TWAINDSM.framework" "${PKGDIR}/Library/Frameworks"

# Fix the owner, group, and mode...
chown -R root "${PKGDIR}/Library"
chgrp -R wheel "${PKGDIR}/Library"
chmod -R 755 "${PKGDIR}/Library"

# Sign our binary...
echo "sign the binary..."
if [ "${SIGNER}" == "" ]; then
	echo "**************************************************"
	echo "* signer file not found, nothing will be signed. *"
	echo "* If you want to sign the DSM and the installer, *"
	echo "* then put the certificate name in a file  named *"
	echo "* signer. This  certificate will  have  to be in *"
	echo "* your keychain.                                 *"
	echo "**************************************************"
else
	echo "signer: ${SIGNER}"
	codesign -f -s "${SIGNER}" "${PKGDIR}/Library/Frameworks/TWAINDSM.Framework/Versions/A/TWAINDSM"
fi

# Build the package
if [ "${SIGNER}" == "" ]; then
	pkgbuild\
	   --identifier com.twain.dsm.pkg\
	   --version "${VERSION}"\
	   --root "${PKGDIR}"\
	   "${DSTDIR}/twaindsm-${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}.pkg"
else
	pkgbuild\
	   --identifier com.twain.dsm.pkg\
	   --version "${VERSION}"\
	   --sign "${SIGNER}"\
	   --root "${PKGDIR}"\
	   "${DSTDIR}/twaindsm-${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}.pkg"
fi
if [ "$?" != "0" ]; then
	echo "pkgbuild failed..."
	exit 1
fi

# Fix the owner and the group...
chown -R root "${PKGDIR}"
chgrp -R wheel "${PKGDIR}"

# Create the disk image
echo "creating the disk image..."
pushd "${TMPDIR}" >/dev/null 2>&1
hdiutil create 	-srcfolder "${DSTDIR}" 	-fs HFS+ -volname "twaindsm-${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}.pkg" "${DMGDIR}/tmp.dmg" >/dev/null 2>&1
if [ "$?" != "0" ]; then
	echo "hdiutil create failed"
	exit 1
fi
popd >/dev/null 2>&1

# This makes the disk read only, and moves it to the desktop...
echo "converting the disk image..."
hdiutil	convert -format UDRO -o "${DESKTOPDIR}/twaindsm-${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}.dmg" "${DMGDIR}/tmp.dmg" >/dev/null 2>&1
if [ "$?" != "0" ]; then
	echo "hdiutil convert failed..."
	exit 1
fi

chown root "${DESKTOPDIR}/twaindsm-${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}.dmg"
chgrp wheel "${DESKTOPDIR}/twaindsm-${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}.dmg"

# And this zips the disk image into the final form, which is "twaindsm.dmg.gz":
echo "Zip it up..."
pushd ${DESKTOPDIR} >/dev/null 2>&1
gzip "twaindsm-${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}.dmg"
if [ "$?" != "0" ]; then
	echo "gzip failed..."
	popd
	exit 1
fi
popd >/dev/null 2>&1

# Drop a copy of the DSM into the DMS folder for this version...
echo "Copy to ${DSMDIR}/macosx..."
mkdir -p "${DSMDIR}/macosx" >/dev/null 2>&1
cp "${DESKTOPDIR}/twaindsm-${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}.dmg.gz" "${DSMDIR}/macosx/"

#Cleanup...
rm -rf "${TMPDIR}"
