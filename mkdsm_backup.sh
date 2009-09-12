#!/bin/sh
#
# Back the directory up...
#

# Say hi...
echo ""
echo "Backup TWAIN_DSM..."
if [ "$DSMBUILDER" != "good" ] ;then
	echo "  please run ./mkdsm.sh"
	exit 1
fi

# Make sure we can do this...
echo "  ...checking directories"
if [ ! -e TWAIN_DSM ]; then
	echo "  TWAIN_DSM not found..."
	exit 1
fi
if [ -e TWAIN_DSM.backup ]; then
	rm -rf TWAIN_DSM.backup
	###echo "  TWAIN_DSM.backup already exists, please remove it manually..."
	###exit 1
fi

# Backup TWAIN_DSM
echo "  ...copying TWAIN_DSM to TWAIN_DSM.backup"
cp -R TWAIN_DSM TWAIN_DSM.backup
if [ ! -e TWAIN_DSM.backup ]; then
	echo "  Backing up TWAIN_DSM didn't work, aborting..."
	exit 1
fi

# Bye-bye...
exit 0
