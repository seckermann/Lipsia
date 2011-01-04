PROJECT(vspectralcluster)

# F2C - Fortran2C output for the lapac library
FIND_LIBRARY(F2C_LIBRARY "f2c")
FIND_PATH(F2C_INCLUDES "f2c.h")
INCLUDE_DIRECTORIES(${F2C_INCLUDES})

# find lapack library
FIND_LIBRARY(LAPACK_LIBRARY lapack)
MESSAGE(STATUS "f2c: ${F2C_LIBRARY}")
ADD_EXECUTABLE(vspectralcluster Eigen.c KMeans.c LaplacianMatrix.c vspectralcluster.c)
TARGET_LINK_LIBRARIES(vspectralcluster lipsia ${LAPACK_LIBRARY} ${VIAIO_LIBRARY} ${GSL_CBLAS_LIBRARY} ${GSL_GSL_LIBRARY} ${LIPSIA_LIBRARY} )

SET_TARGET_PROPERTIES(vspectralcluster PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vspectralcluster
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
