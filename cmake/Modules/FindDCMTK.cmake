# - FindDCMTK.cmake
# 
# Author: Thomas Proeger
#
# Find the library files for the libdcmtk1-dev package.
# 
# This module exports the following variables:
# 
# DCMTK_INCLUDE_DIR - the root directory of the header files. Usually: /usr/include/dcmtk
# 
# the DCMTK libraries
# 
# DCMTK_DCMDATA_LIBRARY
# DCMTK_DCMDSIG_LIBRARY
# DCMTK_DCMIMAGE_LIBRARY
# DCMTK_DCMIMGLE_LIBRARY
# DCMTK_DCMJPEG_LIBRARY
# DCMTK_DCMNET_LIBRARY
# DCMTK_DCMPSTAT_LIBRARY
# DCMTK_DCMQRDB_LIBRARY
# DCMTK_DCMSR_LIBRARY
# DCMTK_DCMTLS_LIBRARY
# DCMTK_DCMWLM_LIBRARY
# DCMTK_IJG12_LIBRARY
# DCMTK_IJG16_LIBRARY
# DCMTK_IJG8_LIBRARY
# DCMTK_OFSTD_LIBRARY
#
# DCMTK_FOUND      - set to TRUE if and only if ALL other files are found.


########################################
# include files
########################################
FIND_PATH(DCMTK_INCLUDE_DIR
    NAMES config/cfunix.h
    DOC "The root directory of the dcmtk header files"
    PATH_SUFFIXES dcmtk
    )

########################################
# library files
########################################

# dcmdata
FIND_LIBRARY(DCMTK_DCMDATA_LIBRARY
    NAMES dcmdata
    DOC "The 'libdcmdata' library"
    )

# dcmdsig
FIND_LIBRARY(DCMTK_DCMDSIG_LIBRARY
    NAMES dcmdsig
    DOC "The 'libdcmdsig' library"
    )

# dcmimage
FIND_LIBRARY(DCMTK_DCMIMAGE_LIBRARY
    NAMES dcmimage
    DOC "The 'libdcmimage' library"
    )

# dcmimgle
FIND_LIBRARY(DCMTK_DCMIMGLE_LIBRARY
    NAMES dcmimgle
    DOC "The 'libdcmimgle' library"
    )

# dcmjpeg
FIND_LIBRARY(DCMTK_DCMJPEG_LIBRARY
    NAMES dcmjpeg
    DOC "The 'libdcmjpeg' library"
    )

# dcmnet
FIND_LIBRARY(DCMTK_DCMNET_LIBRARY
    NAMES dcmnet
    DOC "The 'libdcmpstat' library"
    )

# dcmpstat
FIND_LIBRARY(DCMTK_DCMPSTAT_LIBRARY
    NAMES dcmpstat
    DOC "The 'libdcmpstat' library"
    )

# dcmqrdb
FIND_LIBRARY(DCMTK_DCMQRDB_LIBRARY
    NAMES dcmqrdb
    DOC "The 'libdcmqrdb' library"
    )

# dcmsr
FIND_LIBRARY(DCMTK_DCMSR_LIBRARY
    NAMES dcmsr
    DOC "The 'libdcmsr' library"
    )
    
# tls
FIND_LIBRARY(DCMTK_DCMTLS_LIBRARY
    NAMES dcmtls
    DOC "The 'libdcmtls' library"
    )

# dcmwlm
FIND_LIBRARY(DCMTK_DCMWLM_LIBRARY
    NAMES dcmwlm
    DOC "The 'libdcmwlm' library"
    )

# ijg12
FIND_LIBRARY(DCMTK_IJG12_LIBRARY
    NAMES ijg12
    DOC "The 'libijg12' library"
    )

# ijg16
FIND_LIBRARY(DCMTK_IJG16_LIBRARY
    NAMES ijg16
    DOC "The 'libijg16' library"
    )

# ijg8
FIND_LIBRARY(DCMTK_IJG8_LIBRARY
    NAMES ijg8
    DOC "The 'libijg8' library"
    )
    
# ofstd
FIND_LIBRARY(DCMTK_OFSTD_LIBRARY
    NAMES ofstd
    DOC "The 'libofstd' library"
    )

# handle the QUIETLY and REQUIRED arguments and set PNG_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(DCMTK
    "Cannot find package DCMTK. Did you install the package 'libdcmtk1-dev'?"
     DCMTK_INCLUDE_DIR
     DCMTK_DCMDATA_LIBRARY
     DCMTK_DCMDSIG_LIBRARY
     DCMTK_DCMIMAGE_LIBRARY
     DCMTK_DCMIMGLE_LIBRARY
     DCMTK_DCMJPEG_LIBRARY
     DCMTK_DCMNET_LIBRARY
     DCMTK_DCMPSTAT_LIBRARY
     DCMTK_DCMQRDB_LIBRARY
     DCMTK_DCMSR_LIBRARY
     DCMTK_DCMTLS_LIBRARY
     DCMTK_DCMWLM_LIBRARY
     DCMTK_IJG12_LIBRARY
     DCMTK_IJG16_LIBRARY
     DCMTK_IJG8_LIBRARY
     DCMTK_OFSTD_LIBRARY
     )

# these variables are only visible in 'advanced mode' 
MARK_AS_ADVANCED(DCMTK
     DCMTK_INCLUDE_DIR
     DCMTK_DCMDATA_LIBRARY
     DCMTK_DCMDSIG_LIBRARY
     DCMTK_DCMIMAGE_LIBRARY
     DCMTK_DCMIMGLE_LIBRARY
     DCMTK_DCMJPEG_LIBRARY
     DCMTK_DCMNET_LIBRARY
     DCMTK_DCMPSTAT_LIBRARY
     DCMTK_DCMQRDB_LIBRARY
     DCMTK_DCMSR_LIBRARY
     DCMTK_DCMTLS_LIBRARY
     DCMTK_DCMWLM_LIBRARY
     DCMTK_IJG12_LIBRARY
     DCMTK_IJG16_LIBRARY
     DCMTK_IJG8_LIBRARY
     DCMTK_OFSTD_LIBRARY
     )
