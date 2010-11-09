PROJECT(vbrainmask)

ADD_EXECUTABLE(vbrainmask vbrainmask.c)
TARGET_LINK_LIBRARIES(vbrainmask lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vbrainmask PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vbrainmask
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
