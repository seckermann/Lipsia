PROJECT(vextractparam)

ADD_EXECUTABLE(vextractparam vextractparam.c)
TARGET_LINK_LIBRARIES(vextractparam lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vextractparam PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vextractparam
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
