# - FindGSL.cmake
#
# Author: Thomas Proeger
# 
# Find the header files and libraries from the libgsl0-dev package 
# 
# GSL_INCLUDE_DIR    - the root GSL include directory.
# GSL_GSL_LIBRARY    - the library file libgsl.so
# GSL_CBLAS_LIBRARY  - the library file libgslcblas.so
# GSL_FOUND          - TRUE if and only if ALL other variables have correct
#                      values.
#

# the header files
FIND_PATH(BLAS_INCLUDE_DIR
    NAMES cblas.h 
	PATH_SUFFIXES blas
    DOC "The path to the blas header files"
    )

# the library files
FIND_LIBRARY(BLAS_LIBRARY
    NAMES blas
    DOC "The library file libgsl"
    )
    
# the library files

# # handle the QUIETLY and REQUIRED arguments and set PNG_FOUND to TRUE if 
# # all listed variables are TRUE
# INCLUDE(FindPackageHandleStandardArgs)
# FIND_PACKAGE_HANDLE_STANDARD_ARGS(GSL
#     "Cannot find package GSL. Did you install 'libgsl0-dev'?"
#      GSL_INCLUDE_DIR 
#      GSL_GSL_LIBRARY
#      GSL_CBLAS_LIBRARY
#     )

# these variables are only visible in 'advanced mode' 
MARK_AS_ADVANCED(BLAS_INCLUDE_DIR 
     BLAS_LIBRARY
     )

