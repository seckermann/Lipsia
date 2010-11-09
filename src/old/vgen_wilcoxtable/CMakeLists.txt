PROJECT(vgen_wilcoxtable)

ADD_EXECUTABLE(vgen_wilcoxtable vgen_wilcoxtable.c)
TARGET_LINK_LIBRARIES(vgen_wilcoxtable lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vgen_wilcoxtable PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vgen_wilcoxtable
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
