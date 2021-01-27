# this is just a basic CMakeLists.txt, for more information see the cmake manpage...

cmake_minimum_required(VERSION 2.4)

SET(OSTARGET "${OSTARGET}" CACHE STRING "woof")

IF(EXISTS "/bin/uname")
	EXEC_PROGRAM("/bin/uname" ARGS -p OUTPUT_VARIABLE CMAKE_SYSTEM_PROCESSOR)
ELSEIF("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
	SET(CMAKE_SYSTEM_PROCESSOR "x86_64" CACHE STRING "woof" FORCE)
ELSE()
	SET(CMAKE_SYSTEM_PROCESSOR "x86" CACHE STRING "woof" FORCE)
ENDIF(EXISTS "/bin/uname")

IF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "mips64")
	message(STATUS "Detected processor: ${CMAKE_SYSTEM_PROCESSOR} building ${OSTARGET}")
ELSE()
	message(STATUS "Detected processor: ${CMAKE_SYSTEM_PROCESSOR}")
ENDIF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "mips64")

# Setup the install prefix, if it's not already defined
IF(NOT CMAKE_INSTALL_PREFIX)
	SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT CMAKE_INSTALL_PREFIX)

# Setup the build prefix, if it's not already defined
IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF(NOT CMAKE_BUILD_TYPE)

#add definitions, compiler switches, etc.
IF(APPLE)
	SET(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
	ADD_DEFINITIONS(-Wall -Wextra -Werror -isysroot /Developer/SDKs/MacOSX10.6.sdk -mmacosx-version-min=10.6 -fexceptions -fPIC)
ELSEIF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "mips64")
	IF("${OSTARGET}" STREQUAL "mips64el")
		ADD_DEFINITIONS(-Wall -Wextra -Werror)
	ELSE()
		ADD_DEFINITIONS(-Wall -Wextra -Werror -march=from-abi -mabi=32)
	ENDIF()
ELSE()
	ADD_DEFINITIONS(-Wall -Wextra -Werror)
ENDIF(APPLE)

#let's not be sharing our symbols...
IF(APPLE)
	SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-ldl -Wl,-framework,CoreServices -Wl,-framework,Foundation" CACHE STRING "woof" FORCE)
ELSEIF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "mips64")
	IF("${OSTARGET}" STREQUAL "mips64el")
		SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic -Wl,--no-undefined -Wl,-ldl" CACHE STRING "woof" FORCE)
	ELSE()
		SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic -Wl,--no-undefined -Wl,-ldl -march=from-abi -mabi=32" CACHE STRING "woof" FORCE)
	ENDIF()
ELSE()
	SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-Bsymbolic -Wl,--no-undefined -Wl,-ldl" CACHE STRING "woof" FORCE)
ENDIF(APPLE)

#project name
PROJECT(twaindsm)

#project version
SET(${PROJECT_NAME}_MAJOR_VERSION 2)
SET(${PROJECT_NAME}_MINOR_VERSION 4)
SET(${PROJECT_NAME}_PATCH_LEVEL 3)

#build a shared library
ADD_LIBRARY(twaindsm SHARED dsm.cpp apps.cpp log.cpp)
target_link_libraries(twaindsm dl)

#
SET_TARGET_PROPERTIES(twaindsm PROPERTIES
					  VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_LEVEL}
					  SOVERSION ${${PROJECT_NAME}_MAJOR_VERSION})

#add an install target here
INSTALL(FILES twain.h DESTINATION include)
INSTALL(TARGETS twaindsm 
		LIBRARY DESTINATION lib
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

