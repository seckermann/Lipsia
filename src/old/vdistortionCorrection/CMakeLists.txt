PROJECT(vdistortionCorrection)

ADD_EXECUTABLE(vdistortionCorrection
    DistortionCorrection.c  QuickMorph2d.c  vdistortionCorrection.c)
TARGET_LINK_LIBRARIES(vdistortionCorrection lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vdistortionCorrection PROPERTIES
    COMPILE_FLAGS -ansi
    LINK_FLAGS -Wl)

INSTALL(TARGETS vdistortionCorrection
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
