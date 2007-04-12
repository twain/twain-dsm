#this is just a basic CMakeLists.txt, for more information see the cmake manpage

#add definitions, compiler switches, etc.
ADD_DEFINITIONS(-Wall -Werror -g)

#build a shared library
ADD_LIBRARY(twaindsm SHARED dsm.cpp apps.cpp log.cpp)

#for testing the shared library you probably need some test app too
#ADD_EXECUTABLE(twaindsmtest twaindsmtest.cpp)

#need to link to some other libraries ? just add them here
#TARGET_LINK_LIBRARIES(twaindsmtest twaindsm)

#add an install target here
#INSTALL_FILES(...)
#INSTALL_PROGRAMS(...)
#INSTALL_TARGET(...)

