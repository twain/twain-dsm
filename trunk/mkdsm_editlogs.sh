#!/bin/sh
#
#  Update the appropriate change logs.  We do all logs for all OS's,
#  since the system will detect if a change has already been made...
#

# Say hi...
echo ""
echo "Updating the change log files..."
if [ "$DSMBUILDER" != "good" ] ;then
        echo "  please run ./mkdsm.sh"
        exit 1
fi


# Only update debian/changelog when the version isn't found in the file...
echo "  ...checking debian/changelog"
pushd TWAIN_DSM/debian &> /dev/null
if ! grep "${DMSMAJOR}.${DSMMINOR}.${DSMBUILD}-1" changelog &> /dev/null ;then
	echo "  ...updating debian/changelog"
	DSMDATE=`date +"%a, %e %b %Y %T %z"`
	rm -rf tmp1.tmp
	echo "twaindsm (${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}-1) unstable; urgency=low" > tmp1.tmp
	echo "" >> tmp1.tmp
	echo "   * ${DSMREASON}" >> tmp1.tmp
	echo "" >> tmp1.tmp
	echo " -- ${DSMEMAIL}  ${DSMDATE}" >> tmp1.tmp
	echo "" >> tmp1.tmp
	cat changelog >> tmp1.tmp
	cp tmp1.tmp changelog
	rm -rf tmp1.tmp
fi
popd &> /dev/null


# Only update twaindsm.spec when the version's don't match.  Unfortunately,
# we don't have a version history to help us, so it has to be an exact
# match to keep us from appending...
echo "  ...checking twaindsm.spec"
pushd TWAIN_DSM &> /dev/null
if ! grep "Version: ${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}" twaindsm.spec &> /dev/null ;then
	echo "  ...updating twaindsm.spec"
	DSMDATE=`date +"%a %b %e %Y"`
	rm -rf tmp1.tmp tmp2.tmp tmp3.tmp
	cp twaindsm.spec tmp1.tmp
	cat tmp1.tmp | sed "s/Version:.*/Version: ${DSMMAJOR}.${DSMMINOR}.${DSMBUILD}/" > tmp2.tmp
	cat tmp2.tmp | sed "/%changelog/a* ${DSMDATE} ${DSMEMAIL}\n- ${DSMREASON}" > tmp3.tmp
	cp tmp3.tmp twaindsm.spec
	rm -rf tmp1.tmp tmp2.tmp tmp3.tmp
fi
popd &> /dev/null


# Update CMakeLists.txt with the new version...
echo "  ...updating CMakeLists.txt"
pushd TWAIN_DSM/src &> /dev/null
rm -rf tmp1.tmp tmp2.tmp tmp3.tmp tmp4.tmp
cp CMakeLists.txt tmp1.tmp
cat tmp1.tmp | sed "s/SET(\${PROJECT_NAME}_MAJOR_VERSION.*/SET(\${PROJECT_NAME}_MAJOR_VERSION ${DSMMAJOR})/" > tmp2.tmp
cat tmp2.tmp | sed "s/SET(\${PROJECT_NAME}_MINOR_VERSION.*/SET(\${PROJECT_NAME}_MINOR_VERSION ${DSMMINOR})/" > tmp3.tmp
cat tmp3.tmp | sed "s/SET(\${PROJECT_NAME}_PATCH_LEVEL.*/SET(\${PROJECT_NAME}_PATCH_LEVEL ${DSMBUILD})/" > tmp4.tmp
cp tmp4.tmp CMakeLists.txt
rm -rf tmp1.tmp tmp2.tmp tmp3.tmp tmp4.tmp
popd &> /dev/null


# Bye-bye
exit 0
