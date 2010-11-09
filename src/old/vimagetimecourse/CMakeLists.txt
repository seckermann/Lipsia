PROJECT(vimagetimecourse)

ADD_EXECUTABLE(vimagetimecourse vimagetimecourse.c)
TARGET_LINK_LIBRARIES(vimagetimecourse lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vimagetimecourse PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vimagetimecourse
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
