#!/bin/bash
#
# Remove CRLF (replacing with LF) in all of the
# specified files (or directories)...
#

# Say hi... 
${ECHO} ""
${ECHO} "Removing CRLF (replacing with just LF)..."
if [ "${DSMBUILDER}" != "good" ] ;then
        ${ECHO} "  please run ./mkdsm.sh"
        exit 1
fi

# We validated this, but on macOS, we're not getting
# the value of DOS2UNIX, soooooo... :(
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

# TWAIN_DSM
${ECHO} " ...fixing files in TWAIN_DSM"
pushd TWAIN_DSM &> /dev/null
if [ "${OSNAME}" == "ubuntu" ] ;then
	${DOS2UNIX} -p Doxyfile twaindsm.spec *.txt
elif [ "$OSNAME" == "suse" ] ;then
	${DOS2UNIX} -q -k -o Doxyfile twaindsm.spec *.txt 
elif [ "$OSNAME" == "macosx" ] ;then
	${DOS2UNIX} Doxyfile twaindsm.spec *.txt 
else
	${ECHO} "unsupported os: ${OSNAME}"
	exit 1
fi
popd &> /dev/null

# TWAIN_DSM/debian
${ECHO} " ...fixing files in TWAIN_DSM/debian"
pushd TWAIN_DSM/debian &> /dev/null
if [ "${OSNAME}" == "ubuntu" ] ;then
	${DOS2UNIX} -p *
elif [ "$OSNAME" == "suse" ] ;then
	${DOS2UNIX} -q -k -o *
elif [ "$OSNAME" == "macosx" ] ;then
	${DOS2UNIX} *
else
	${ECHO} "unsupported os: ${OSNAME}"
	exit 1
fi
popd &> /dev/null

# TWAIN_DSM/src
${ECHO} " ...fixing files in TWAIN_DSM/src"
pushd TWAIN_DSM/src &> /dev/null
if [ "${OSNAME}" == "ubuntu" ] ;then
	${DOS2UNIX} -p *
elif [ "$OSNAME" == "suse" ] ;then
	${DOS2UNIX} -q -k -o *
elif [ "$OSNAME" == "macosx" ] ;then
	${DOS2UNIX} *
else
	${ECHO} "unsupported os: ${OSNAME}"
	exit 1
fi
popd &> /dev/null

# Bye-bye
exit 0
