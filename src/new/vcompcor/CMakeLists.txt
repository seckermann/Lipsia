PROJECT(vcompcor)

ADD_EXECUTABLE(vcompcor vcompcor.c PCA.c MinVals.c HighpassFilter.c FreqFilter.c)
TARGET_LINK_LIBRARIES(vcompcor lipsia ${VIA_LIBRARY} ${FFTW3_FFTW3_LIBRARY})

SET_TARGET_PROPERTIES(vcompcor PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vcompcor
        RUNTIME DESTINATION bin
                COMPONENT RuntimeLibraries)
