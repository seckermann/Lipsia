PROJECT(vmulticomp)

ADD_EXECUTABLE(vmulticomp
    vmulticomp.c GaussFilter.c Multicomp2d.c Multicomp3d.c Symmetry.c)
TARGET_LINK_LIBRARIES(vmulticomp lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vmulticomp PROPERTIES
    COMPILE_FLAGS -ansi
    LINK_FLAGS -Wl)

INSTALL(TARGETS vmulticomp
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
