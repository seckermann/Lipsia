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

# hidden OPTION, used to fix the lipsia include bug. With
# an temporary VIA installation we have to provide an include path
# that alliows #include <viaio/*.h> and #include <via/*.h> directives.
IF(LIPSIA_INCLUDE_ROOT)
    INCLUDE_DIRECTORIES(${LIPSIA_INCLUDE_ROOT})
ENDIF(LIPSIA_INCLUDE_ROOT)

# the name of the Lipsia library
SET(LIPSIA_LIBRARY "lipsia")

# set the lipsia version
SET(LIPSIA_VERSION_MAJOR "2")
SET(LIPSIA_VERSION_MINOR "0")
SET(LIPSIA_VERSION_PATCH "0")

ADD_DEFINITIONS(-D_LIPSIA_VERSION_MAJOR=${LIPSIA_VERSION_MAJOR})
ADD_DEFINITIONS(-D_LIPSIA_VERSION_MINOR=${${CMAKE_PROJECT_NAME}_VERSION_MINOR})
ADD_DEFINITIONS(-D_LIPSIA_VERSION_PATCH=${${CMAKE_PROJECT_NAME}_VERSION_PATCH})

SET(LIPSIA_SHARED_LIB_VERSION
	${LIPSIA_VERSION_MAJOR}.${LIPSIA_VERSION_MINOR}.${LIPSIA_VERSION_PATCH})

############################################################
# SVN revision number
############################################################
# Set the revision number if this dir is a svn working copy.
# The revision number will be stored in SVN_REVISION.
FIND_FILE(SVNVERSION svnversion
    PATHS /usr/bin /usr/local/bin
    DOC "The subversion revision information tool.")

IF (NOT SVN_REVISION)
	IF (SVNVERSION)
		EXECUTE_PROCESS(COMMAND ${SVNVERSION}
			OUTPUT_VARIABLE OUT
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
		string(REGEX MATCH "exported" EXPORTED "${OUT}")
		IF (NOT EXPORTED)
			string(REGEX MATCH "^[0-9]+" SVN_REVISION "${OUT}") #@todo wont work with more complex revisions/modified sources
			SET(SVN_REVISION ${SVN_REVISION})
		ELSE (NOT EXPORTED)
			set(SVN_REVISION "exported")
		ENDIF (NOT EXPORTED)
	ENDIF (SVNVERSION)
ENDIF (NOT SVN_REVISION)

IF (SVN_REVISION)
  MESSAGE(STATUS "SVN Revision: ${SVN_REVISION}")
  ADD_DEFINITIONS(-D_LIPSIA_SVN_REVISION=${SVN_REVISION})
ENDIF(SVN_REVISION)


# configure install destinations
# SET(LIPSIA_INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin")
# move private libs into separate directory (only relevant for shared libs)
# SET(LIPSIA_INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/lipsia")
# target directory of the docs
# SET(LIPSIA_INSTALL_DOC_DIR "${CMAKE_INSTALL_PREFIX}/share/doc/lipsia")
# the shared resource files directory
# SET(LIPSIA_INSTALL_RESOURCE_DIR "${CMAKE_INSTALL_PREFIX}/share/lipsia")
# target directory of the header files
# SET(LIPSIA_INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include/lipsia")

#isis related settings
add_definitions(-D_ENABLE_LOG=1)
add_definitions(-D_ENABLE_DEBUG=0)

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

FIND_PACKAGE(ISIS REQUIRED)

# look for ZLIB
FIND_PACKAGE(ZLIB REQUIRED)

# look for itk
FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

FIND_PACKAGE(BLAS REQUIRED)
MESSAGE(STATUS ${BLAS_LIBRARY})

# look for boost
FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem regex system date_time)

###########################################################
# CPack config
###########################################################
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Aanalysis suite for MRI and fMRI data.")
SET(CPACK_PACKAGE_VENDOR "Max Planck Institute for Human Cognitive and Brain Sciences")
SET(CPACK_PACKAGE_CONTACT "tuerke@cbs.mpg.de")
SET(CPACK_PACKAGE_NAME "lipsia")
SET(CPACK_BUNDLE_NAME ${CPACK_PACKAGE_NAME})
#SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${${CMAKE_PROJECT_NAME}_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${${CMAKE_PROJECT_NAME}_VERSION_PATCH})
SET(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE})
#SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}")
SET(CPACK_RPM_POST_INSTALL_SCRIPT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/rpm_%post.in)
SET(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/rpm_%postun.in)
INCLUDE(CPack)

#
# Install global pieces
#

# add include directories
INCLUDE_DIRECTORIES(
  ${ISIS_INCLUDE_DIRS}
  ${VIA_INCLUDE_DIR}
  ${VIAIO_INCLUDE_DIR}
  ${GSL_INCLUDE_DIR}
  ${ISIS_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
	)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -msse2  -fopenmp -O3 -march=amdfam10 ")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -Wno-deprecated -msse2  -ftemplate-depth-100 -fopenmp -O3 -march=amdfam10 ")

INSTALL(DIRECTORY doc/html
  DESTINATION share/doc/lipsia
  PATTERN ".svn" EXCLUDE )

install(PROGRAMS lipsia-help DESTINATION bin)

#installing the scripts
install(PROGRAMS src/scripts/vpreproc_gui DESTINATION bin)
install(PROGRAMS src/scripts/vcreatemask DESTINATION bin)

# install ressource files
FILE(GLOB RES_FILES data/*)
# remove '.svn' element from filelist
FOREACH(ELEMENT ${RES_FILES})
    STRING(REGEX MATCH "\\.svn" FOUND ${ELEMENT})
    # add entry if it's not ".svn"
    IF(NOT FOUND)
        SET(FILES ${FILES} ${ELEMENT})
    ENDIF(NOT FOUND)
ENDFOREACH(ELEMENT ${RES_FILES})

INSTALL(FILES ${FILES} DESTINATION share/lipsia)

# subprojects
ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/src)
ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/design)

MESSAGE(STATUS "isis include: ${ISIS_INCLUDE_DIRS}")
MESSAGE(STATUS "isis libs: ${ISIS_LIB}")
MESSAGE(STATUS "isis lib depends: ${ISIS_LIB_DEPENDS}")
