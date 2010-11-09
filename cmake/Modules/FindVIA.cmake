# - FindVIA.cmake
# 
# Author: Thomas Proeger
#
# Find the header files and libraries for the libvia and libvia-dev packages
#
# This find package exports the following variables:
# VIA_INCLUDE_DIR         - directory for via.h viadata.h
# VIAIO_INCLUDE_DIR       - directory for VImage.h, option.h, etc. pp
# VIA_LIBRARY             - libvia
# VIAIO_LIBRARY           - libviaio
# VX_LIBRARY              - libvx
# VIA_FOUND               - TRUE if and only if ALL other variables have correct values.
#

# INCLUDE directories
FIND_PATH(VIA_INCLUDE_DIR
    NAMES via.h
    PATH_SUFFIXES via
    DOC "The include directory containing via.h"
    )
    
FIND_PATH(VIAIO_INCLUDE_DIR
    NAMES VImage.h
    PATH_SUFFIXES viaio
    DOC "The include directory containing VImage.h and option.h"
    )

# LIBRARY files
FIND_LIBRARY(VIA_LIBRARY
    NAMES via libvia
    DOC "The library file libvia.so"
)

FIND_LIBRARY(VIAIO_LIBRARY
    NAMES viaio libviaio
    DOC "The library file libviaio.so"
)

FIND_LIBRARY(VX_LIBRARY
    NAMES vx libvx
    DOC "The library file libvx.so"
)

# handle the QUIETLY and REQUIRED arguments and set PNG_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(VIA
    "Cannot find package VIA. Did you install the package 'libvia-dev'?"
    VIA_INCLUDE_DIR
    VIAIO_INCLUDE_DIR
    VIA_LIBRARY
    VIAIO_LIBRARY
#    VX_LIBRARY
)

# these variables are only visible in 'advanced mode' 
MARK_AS_ADVANCED(VIA_INCLUDE_DIR
    VIAIO_INCLUDE_DIR
    VIA_LIBRARY
    VIAIO_LIBRARY
    VX_LIBRARY
    )
