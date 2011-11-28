PROJECT(vpreprocess)

ADD_EXECUTABLE(vpreprocess vpreprocess.c MinVals.c SpatialFilter.c FreqFilter.c)
TARGET_LINK_LIBRARIES(vpreprocess lipsia ${VIA_LIBRARY} ${FFTW3_FFTW3_LIBRARY})

SET_TARGET_PROPERTIES(vpreprocess PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vpreprocess
        RUNTIME DESTINATION bin
                COMPONENT RuntimeLibraries)
