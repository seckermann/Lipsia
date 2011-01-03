PROJECT(vmovcorrection)

ADD_EXECUTABLE(vmovcorrection
               FreqFilter1.c MovCorr.c ApplyTransform3d.c vmovcorrection.c RotationMatrix.c SpatialFilter.c)
TARGET_LINK_LIBRARIES(vmovcorrection lipsia ${VIA_LIBRARY} ${FFTW3_FFTW3_LIBRARY})

SET_TARGET_PROPERTIES(vmovcorrection PROPERTIES
                      COMPILE_FLAGS -ansi
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vmovcorrection
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
