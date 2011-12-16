#!/bin/bash
#
# Remove CRLF (replacing with LF) in all of the
# specified files (or directories)...
#

# Say hi... 
echo ""
echo "Removing CRLF (replacing with just LF)..."
if [ "$DSMBUILDER" != "good" ] ;then
        echo "  please run ./mkdsm.sh"
        exit 1
fi


# TWAIN_DSM
echo " ...fixing files in TWAIN_DSM"
pushd TWAIN_DSM &> /dev/null
if [ "$OSNAME" == "ubuntu" ] ;then
	$(DOS2UNIX) -p Doxyfile twaindsm.spec *.txt
else
	$(DOS2UNIX) -q -k -o Doxyfile twaindsm.spec *.txt 
fi
popd &> /dev/null

# TWAIN_DSM/debian
echo " ...fixing files in TWAIN_DSM/debian"
pushd TWAIN_DSM/debian &> /dev/null
if [ "$OSNAME" == "ubuntu" ] ;then
	$(DOS2UNIX) -p *
else
	$(DOS2UNIX) -q -k -o *
fi
popd &> /dev/null

# TWAIN_DSM/src
echo " ...fixing files in TWAIN_DSM/src"
pushd TWAIN_DSM/src &> /dev/null
if [ "$OSNAME" == "ubuntu" ] ;then
	$(DOS2UNIX) -p *
else
	$(DOS2UNIX) -q -k -o *
fi
popd &> /dev/null

# Bye-bye
exit 0
