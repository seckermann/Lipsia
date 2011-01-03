PROJECT(vmovcorrection2d)

ADD_EXECUTABLE(vmovcorrection2d
               FreqFilter1.c MovCorr2d.c ApplyTransform2d.c vmovcorrection2d.c)
TARGET_LINK_LIBRARIES(vmovcorrection2d lipsia ${VIA_LIBRARY} ${FFTW3_FFTW3_LIBRARY})

SET_TARGET_PROPERTIES(vmovcorrection2d PROPERTIES
                      COMPILE_FLAGS -ansi
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vmovcorrection2d
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
