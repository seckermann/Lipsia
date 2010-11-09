PROJECT(vdomulticomp)

ADD_EXECUTABLE(vdomulticomp vdomulticomp.c DoMulticomp2d.c DoMulticomp3d.c)
TARGET_LINK_LIBRARIES(vdomulticomp lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vdomulticomp PROPERTIES
    COMPILE_FLAGS -ansi
    LINK_FLAGS -Wl)

INSTALL(TARGETS vdomulticomp
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
