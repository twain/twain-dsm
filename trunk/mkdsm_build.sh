#!/bin/bash
#
# Build the DSM...
#

# Say hi...
echo ""
echo "Build the DSM..."
if [ "$DSMBUILDER" != "good" ] ;then
        echo "  please run ./mkdsm.sh"
        exit 1
fi


#
# Running for ubuntu...
#
if [ "$OSNAME" == "ubuntu" ] ;then

	# Run cmake...
	echo "  ...running cmake"
	pushd TWAIN_DSM/src &> /dev/null
	if ! cmake . ;then
		echo "  cmake failed..."
		exit 1
	fi
	echo "  ...cmake finished successfully"
	popd &> /dev/null

	# Run debuild...
	echo ""
	echo "  ...running debuild"
	pushd TWAIN_DSM &> /dev/null
	if ! ${DEBUILD} ;then
		echo "  debuild failed..."
		exit 1
	fi
	popd &> /dev/null
	echo "  ...debuild finished successfully"

	# Copy the files to the final directory format...
	echo "  ...copy files to ${DSMDIR}/${OSDIR}"
	mkdir -p ${DSMDIR}/${OSDIR}
	if ! cp ${DSMBASE}-1.dsc ${DSMDIR}/${OSDIR}/ ;then
		echo "  dsc failed..."
		exit 1
	fi
	if ! cp ${DSMBASE}-1_${MACHINE}.changes ${DSMDIR}/${OSDIR}/ ;then
		echo "  changes failed..."
		exit 1
	fi
	if ! cp ${DSMBASE}-1_${MACHINE}.deb ${DSMDIR}/${OSDIR}/ ;then
		echo "  deb failed..."
		exit 1
	fi

	# Copy the files to the tarball...
	echo "  ...copy files to ${DSMDIR}/tarball"
	mkdir -p ${DSMDIR}/tarball
	if ! cp ${DSMBASE}-1.tar.gz ${DSMDIR}/tarball/${DSMBASE}.orig.tar.gz ;then
		echo "  tar.gz failed..."
		exit 1
	fi
	if ! touch ${DSMBASE}-1.diff ;then
		echo "  touch failed..."
		exit 1
	fi
	if ! gzip ${DSMBASE}-1.diff ;then
		echo "  gzip failed..."
		exit 1
	fi
	if ! cp ${DSMBASE}-1.diff.gz ${DSMDIR}/tarball ;then
		echo "  diff failed..."
		exit 1
	fi

	# Test the package...
	echo ""
	echo "  ...test the package"
	pushd ${DSMDIR}/${OSDIR} &> /dev/null
	if ! dpkg --simulate -i ${DSMBASE}-1_${MACHINE}.deb ;then
		echo "  dpkg test failed..."
		exit 1
	fi
	popd &> /dev/null
	echo "  ...test finished successfully"

	# If we have rpmbuild, then do that too...
	if [ -f /usr/bin/rpmbuild ]; then

		# Cleanup...
		echo ""
		echo "  ...cleanupe"
		./mkdsm_clean.sh keeprelease		

	        # Copy TWAIN_DSM to the target directory...
        	echo "  ...copy TWAIN_DSM to twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD"
        	cp -R TWAIN_DSM twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD

        	# Make the source code bundle...
        	echo "  ...create twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD.tar.gz"
        	tar czvf twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD.tar.gz twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD
        	echo "  ...create finished successfully"

        	# Copy source to /usr/src/rpm/SOURCES...
        	echo "  ...copy source to /usr/src/rpm/SOURCES"
        	cp twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD.tar.gz /usr/src/rpm/SOURCES/

        	# Run rpmbuild...
        	echo "  ...running rpmbuild"
        	pushd twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD &>/dev/null
        	if ! rpmbuild -ba twaindsm.spec ;then
                	echo "  rpmbuild failed..."
                	exit 1
        	fi
        	echo "  ...rpmbuild finished successfully"
        	popd &> /dev/null

        	# Copy the files to the final directory format...
        	echo "  ...copy files to ${DSMDIRRPM}/${OSDIRRPM}"
        	mkdir -p ${DSMDIRRPM}/${OSDIRRPM}
        	if ! cp ${DSMBASERPM}.tar.gz ${DSMDIRRPM}/${OSDIRRPM}/${DSMBASERPM}.orig.tar.gz ;then
                	echo "  cp failed..."
                	exit 1
        	fi
        	if ! cp /usr/src/rpm/RPMS/${MACHINERPM}/${DSMBASERPM}-1.${MACHINERPM}.rpm ${DSMDIRRPM}/${OSDIRRPM}/ ;then
                	echo "  cp failed..."
                	exit 1
        	fi
        	if ! cp /usr/src/rpm/SRPMS/${DSMBASERPM}-1.src.rpm ${DSMDIRRPM}/${OSDIRRPM}/ ;then
                	echo "  cp failed..."
                	exit 1
        	fi

        	# Test the package...
        	echo ""
        	echo "  ...test the package"
		rm -rf /tmp/${DSMBASERPM}-1.${MACHINERPM}.rpm
		rm -rf /tmp/${DSMBASE}-1_${MACHINE}.deb
        	pushd ${DSMDIRRPM}/${OSDIRRPM} &> /dev/null
		if ! cp ${DSMBASERPM}-1.${MACHINERPM}.rpm /tmp/ ;then
                	echo "  cp failed..."
                	exit 1
        	fi
        	pushd /tmp &> /dev/null
		if ! alien --to-deb --scripts -k ${DSMBASERPM}-1.${MACHINERPM}.rpm ;then
                	echo "  alien failed..."
                	exit 1
        	fi
		if ! dpkg --simulate -i /tmp/${DSMBASE}-1_${MACHINE}.deb ;then
			xxx=1
			###echo "  dpkg test failed..."
			###exit 1
		fi
        	popd &> /dev/null
        	popd &> /dev/null
		rm -rf /tmp/${DSMBASERPM}-1.${MACHINERPM}.rpm
		rm -rf /tmp/${DSMBASE}-1_${MACHINE}.deb
        	echo "  ...test finished successfully"

	fi


#
# Running for SuSE...
#
elif [ "$OSNAME" == "suse" ] ;then

	# Copy TWAIN_DSM to the target directory...
	echo "  ...copy TWAIN_DSM to twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD"
	cp -R TWAIN_DSM twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD

	# Make the source code bundle...
	echo "  ...create twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD.tar.gz"
	tar czvf twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD.tar.gz twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD
	echo "  ...create finished successfully"

	# Copy source to /usr/src/packages/SOURCES...
	echo "  ...copy source to /usr/src/packages/SOURCES"
	cp twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD.tar.gz /usr/src/packages/SOURCES

	# Run rpmbuild...
	echo "  ...running rpmbuild"
	pushd twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD &>/dev/null
	if ! rpmbuild -ba twaindsm.spec ;then
		echo "  rpmbuild failed..."
		exit 1
	fi
	echo "  ...rpmbuild finished successfully"
	popd &> /dev/null

	# Copy the files to the final directory format...
	echo "  ...copy files to ${DSMDIR}/${OSDIR}"
	mkdir -p ${DSMDIR}/${OSDIR}
	if ! cp ${DSMBASE}.tar.gz ${DSMDIR}/${OSDIR}/${DSMBASE}.orig.tar.gz ;then
		echo "  cp failed..."
		exit 1
	fi
	if ! cp /usr/src/packages/RPMS/${MACHINE}/${DSMBASE}-1.${MACHINE}.rpm ${DSMDIR}/${OSDIR}/ ;then
		echo "  cp failed..."
		exit 1
	fi
	if ! cp /usr/src/packages/SRPMS/${DSMBASE}-1.src.rpm ${DSMDIR}/${OSDIR}/ ;then
		echo "  cp failed..."
		exit 1
	fi

	# Test the package...
	echo ""
	echo "  ...test the package"
	pushd ${DSMDIR}/${OSDIR} &> /dev/null
	if ! rpm --test --install --force ${DSMBASE}-1.${MACHINE}.rpm ;then
		echo "  rpm test failed..."
		exit 1
	fi
	popd &> /dev/null
	echo "  ...test finished successfully"


#
# Running for Mac OS X...
#
elif [ "$OSNAME" == "macosx" ] ;then

	# Run cmake...
	echo "  ...running cmake"
	pushd TWAIN_DSM/src &> /dev/null
	if ! cmake . ;then
		echo "  cmake failed..."
		exit 1
	fi
	echo "  ...cmake finished successfully"
	popd &> /dev/null

if true; then

	# Copy TWAIN_DSM to the target directory...
	echo "  ...copy TWAIN_DSM to twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD"
	cp -R TWAIN_DSM twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD

	# Make the source code bundle...
	echo "  ...create twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD.tar.gz"
	tar czvf twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD.tar.gz twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD
	echo "  ...create finished successfully"

	# Copy source to /usr/src/packages/SOURCES...
	echo "  ...copy source to /usr/src/packages/SOURCES"
	cp twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD.tar.gz /usr/src/packages/SOURCES

	# Run rpmbuild...
	echo "  ...running rpmbuild"
	pushd twaindsm-$DSMMAJOR.$DSMMINOR.$DSMBUILD &>/dev/null
	if ! rpmbuild -ba twaindsm.spec ;then
		echo "  rpmbuild failed..."
		exit 1
	fi
	echo "  ...rpmbuild finished successfully"
	popd &> /dev/null

	# Copy the files to the final directory format...
	echo "  ...copy files to ${DSMDIR}/${OSDIR}"
	mkdir -p ${DSMDIR}/${OSDIR}
	if ! cp ${DSMBASE}.tar.gz ${DSMDIR}/${OSDIR}/${DSMBASE}.orig.tar.gz ;then
		echo "  cp failed..."
		exit 1
	fi
	if ! cp /usr/src/packages/RPMS/${MACHINE}/${DSMBASE}-1.${MACHINE}.rpm ${DSMDIR}/${OSDIR}/ ;then
		echo "  cp failed..."
		exit 1
	fi
	if ! cp /usr/src/packages/SRPMS/${DSMBASE}-1.src.rpm ${DSMDIR}/${OSDIR}/ ;then
		echo "  cp failed..."
		exit 1
	fi

	# Test the package...
	echo ""
	echo "  ...test the package"
	pushd ${DSMDIR}/${OSDIR} &> /dev/null
	if ! rpm --test --install --force ${DSMBASE}-1.${MACHINE}.rpm ;then
		echo "  rpm test failed..."
		exit 1
	fi
	popd &> /dev/null
	echo "  ...test finished successfully"
else
	echo
fi

#
# Ruh-roh
#
else
	echo "  unrecognized distro... $OSNAME"
	exit 1
fi

# Bye-bye
exit 0
