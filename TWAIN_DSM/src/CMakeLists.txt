#this is just a basic CMakeLists.txt, for more information see the cmake manpage

# Setup the install prefix, if it's not already defined
IF(NOT CMAKE_INSTALL_PREFIX)
	SET(CMAKE_INSTAL_PREFIX "/usr/local")
ENDIF(NOT CMAKE_INSTALL_PREFIX)

# Setup the build prefix, if it's not already defined
IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF(NOT CMAKE_BUILD_TYPE)

#add definitions, compiler switches, etc.
ADD_DEFINITIONS(-Wall -Wextra -Werror)

#project name
PROJECT(twaindsm)

#project version
SET(${PROJECT_NAME}_MAJOR_VERSION 2)
SET(${PROJECT_NAME}_MINOR_VERSION 0)
SET(${PROJECT_NAME}_PATCH_LEVEL 4)

#build a shared library
ADD_LIBRARY(twaindsm SHARED dsm.cpp apps.cpp log.cpp)

SET_TARGET_PROPERTIES(twaindsm PROPERTIES
					  VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_LEVEL}
					  SOVERSION ${${PROJECT_NAME}_MAJOR_VERSION})

#add an install target here
INSTALL(FILES twain.h DESTINATION include)
INSTALL(TARGETS twaindsm 
		LIBRARY DESTINATION lib
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

