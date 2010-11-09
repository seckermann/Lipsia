PROJECT(vdifftimecourse)

ADD_EXECUTABLE(vdifftimecourse vdifftimecourse.c)
TARGET_LINK_LIBRARIES(vdifftimecourse lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vdifftimecourse PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vdifftimecourse
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
