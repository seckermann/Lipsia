

# address warnings by cmake
cmake_minimum_required(VERSION 2.6)

include(InstallRequiredSystemLibraries) #http://docdb.fnal.gov/ILC/sid/stanitz/ilcsoft-latest/cmake-2.4.8/Modules/InstallRequiredSystemLibraries.cmake

PROJECT(LIPSIA)

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


IF(APPLE)
	set(LIB_INSTALL_PATH lib)
else(APPLE)
	set(LIB_INSTALL_PATH lib/lipsia)
endif(APPLE)

# the name of the Lipsia library
SET(LIPSIA_LIBRARY "lipsia")

# set the lipsia version
SET(LIPSIA_VERSION_MAJOR "2")
SET(LIPSIA_VERSION_MINOR "2")
SET(LIPSIA_VERSION_PATCH "0")

ADD_DEFINITIONS(-D_LIPSIA_VERSION_MAJOR=${LIPSIA_VERSION_MAJOR})
ADD_DEFINITIONS(-D_LIPSIA_VERSION_MINOR=${${CMAKE_PROJECT_NAME}_VERSION_MINOR})
ADD_DEFINITIONS(-D_LIPSIA_VERSION_PATCH=${${CMAKE_PROJECT_NAME}_VERSION_PATCH})

SET(LIPSIA_SHARED_LIB_VERSION
	${LIPSIA_VERSION_MAJOR}.${LIPSIA_VERSION_MINOR}.${LIPSIA_VERSION_PATCH})

############################################################
# RCS revision number
############################################################
# The revision number is based on GIT commit informations.
# It will be stored in GIT_REVISION.
find_program(GIT_COMMAND git DOC "The rcs revision description information tool.")

if (NOT GIT_REVISION)
  if (GIT_COMMAND)
    execute_process(COMMAND ${GIT_COMMAND} describe --always OUTPUT_VARIABLE OUT WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX MATCH "[0-9]+-[^-]+$" GIT_REVISION "${OUT}") #@todo wont work with more complex revisions/modified sources
    set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${GIT_REVISION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE})
  else(GIT_COMMAND)
    message(STATUS "git not found")
  endif (GIT_COMMAND)
endif (NOT GIT_REVISION)

if (GIT_REVISION)
  message(STATUS "RCS Revision: ${GIT_REVISION}")
  add_definitions(-DLIPSIA_RCS_REVISION=${GIT_REVISION})
  set(LIPSIA_REV_STRING ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}.${${CMAKE_PROJECT_NAME}_VERSION_PATCH}-${GIT_REVISION})
else(GIT_REVISION)
  set(LIPSIA_REV_STRING ${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}.${${CMAKE_PROJECT_NAME}_VERSION_PATCH})
endif(GIT_REVISION)

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

OPTION(QT3_SUPPORT "Toggle Qt-3 support" OFF)

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

# look for GSL
FIND_PACKAGE(GSL REQUIRED)

# look for fftw3
FIND_PACKAGE(FFTW3 REQUIRED)

if(QT3_SUPPORT)
	FIND_PACKAGE(Qt3 REQUIRED)
endif(QT3_SUPPORT)

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

#the nifti library has to be at due to backward-compatibility reasons
FIND_PACKAGE(NIFTI REQUIRED)

# look for boost
FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem regex system date_time)

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
  ${NIFTI_INCLUDE_DIR}
	)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -msse2  -fopenmp -O3  ")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -Wno-deprecated -msse2  -ftemplate-depth-100 -fopenmp -O3 ")

INSTALL(DIRECTORY doc/html
  DESTINATION share/doc/lipsia
  PATTERN ".svn" EXCLUDE )

#install(PROGRAMS lipsia-help DESTINATION bin)

#installing the scripts
install(PROGRAMS src/scripts/vcreatemask DESTINATION bin)
#install(PROGRAMS src/scripts/vnormdata DESTINATION bin)
install(PROGRAMS src/scripts/vpreproc_gui DESTINATION bin)

#############################################################
# CPack config
#############################################################
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "lipsia")
set(CPACK_PACKAGE_VENDOR "Erik Tuerke")
set(CPACK_PACKAGE_CONTACT "tuerke@cbs.mpg.de")
set(CPACK_PACKAGE_NAME "Lipsia")
set(CPACK_BUNDLE_NAME ${CPACK_PACKAGE_NAME})
set(CPACK_COMPONENTS_ALL "RuntimeLibraries" "Development" "DataFiles")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING.txt")
set(CPACK_PACKAGE_VERSION_MAJOR ${LIPSIA_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${LIPSIA_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${LIPSIA_VERSION_PATCH})
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${LIPSIA_VERSION_MAJOR}.${LIPSIA_VERSION_MINOR}.${LIPSIA_VERSION_PATCH}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE})
include(CPack)

message(STATUS "cpack package name: ${CPACK_PACKAGE_FILE_NAME}")
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

INSTALL(FILES ${FILES} DESTINATION share/lipsia COMPONENT DataFiles)

# subprojects
ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/src)
ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/design)

MESSAGE(STATUS "isis include: ${ISIS_INCLUDE_DIRS}")
MESSAGE(STATUS "isis libs: ${ISIS_LIB}")
MESSAGE(STATUS "isis lib depends: ${ISIS_LIB_DEPENDS}")
MESSAGE(STATUS "installing lipsia libraries to ${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_PATH}")


#add uninstall target

CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake//cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake")



