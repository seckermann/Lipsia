PROJECT(LIPSIA)

# address warnings by cmake
cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0002 NEW)
endif(COMMAND cmake_policy)

IF(NOT UNIX)
  MESSAGE(FATAL_ERROR
    "The lipsia project only supports unix platforms, at the moment."
    )
ENDIF(NOT UNIX)

# add path for new modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
	"${CMAKE_SOURCE_DIR}/cmake/Modules")

# set the lipsia version
SET(LIPSIA_VERSION_MAJOR "2")
SET(LIPSIA_VERSION_MINOR "0")
SET(LIPSIA_VERSION_PATCH "0")

SET(LIPSIA_SHARED_LIB_VERSION
	${LIPSIA_VERSION_MAJOR}.${LIPSIA_VERSION_MINOR}.${LIPSIA_VERSION_PATCH})

# configure install destinations
SET(LIPSIA_INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin")
# move private libs into separate directory (only relevant for shared libs)
SET(LIPSIA_INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/lipsia")
# target directory of the docs
SET(LIPSIA_INSTALL_DOC_DIR "${CMAKE_INSTALL_PREFIX}/share/doc/lipsia")
# the shared resource files directory
SET(LIPSIA_INSTALL_RESOURCE_DIR "${CMAKE_INSTALL_PREFIX}/share/lipsia")
# target directory of the header files
SET(LIPSIA_INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include/lipsia")

#isis related settings
add_definitions(-D_ENABLE_LOG=1)
add_definitions(-D_ENABLE_DEBUG=0)

# add path for io-plugins
file (GLOB IO_PLUGIN_FILES plugins/*.so)

# add option to build shared libraries. The default is OFF.
# Building the three local libs as private shared libs, would reduce
# the memory footprint of LIPSIA especially on multi-user machines
# (even size of binaries would shrink by 30%).
# However, that does not seem to be possible since libvlh relies on symbols
# (e.g. 'pr') that are shared by all applications using the lib, which
# probably break the desired behavior. Maybe this could be changed?
#OPTION(BUILD_SHARED_LIBS "Toggle building shared libraries")

# look for VIA
FIND_PACKAGE(VIA REQUIRED)

# look for Qt3
FIND_PACKAGE(Qt3 REQUIRED)

# look for GSL
FIND_PACKAGE(GSL REQUIRED)

# look for fftw3
FIND_PACKAGE(FFTW3 REQUIRED)

# look for OpenGL
FIND_PACKAGE(OpenGL REQUIRED)

# look for ZLIB
FIND_PACKAGE(ZLIB REQUIRED)

# look for itk
FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

# look for boost
FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem regex system date_time)

# CPack support
# INCLUDE(CPack)

#
# Install global pieces
#

# add include directories
INCLUDE_DIRECTORIES(${VIA_INCLUDE_DIR}
  ${VIAIO_INCLUDE_DIR}
  ${GSL_INCLUDE_DIR}
  ${ISIS_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
	)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -msse2  -fopenmp -O3 -march=amdfam10 ")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -Wno-deprecated -msse2  -ftemplate-depth-100 -fopenmp -O3 -march=amdfam10 ")

INSTALL(DIRECTORY doc/html
  DESTINATION ${LIPSIA_INSTALL_DOC_DIR}
  PATTERN ".svn" EXCLUDE )

install(PROGRAMS lipsia-help DESTINATION ${LIPSIA_INSTALL_BIN_DIR})

# subprojects
ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/src)
