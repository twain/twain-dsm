#!/bin/bash

chmod u+x configure dos2unix

./dos2unix -v \
  configure* \
  NEWS\
  README\
  TODO\
  INSTALL\
  Doxyfile\
  COPYING\
  ChangeLog\
  `find -name Makefile.am`\
  `find -name '*.txt'`\
  `find -name '*.h'`\
  `find -name '*.cpp'`

# This script simply runs configure using a prefix that is FHS compliant
./configure --prefix=/usr
