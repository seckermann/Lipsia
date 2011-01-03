PROJECT(vgetcovariates)

ADD_EXECUTABLE(vgetcovariates vgetcovariates.c vregr.c vorth.c vconv.c)
TARGET_LINK_LIBRARIES(vgetcovariates lipsia ${VIA_LIBRARY} ${FFTW3_FFTW3_LIBRARY})

SET_TARGET_PROPERTIES(vgetcovariates PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vgetcovariates
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
