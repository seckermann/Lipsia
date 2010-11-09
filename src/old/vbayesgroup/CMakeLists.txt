PROJECT(vbayesgroup)

ADD_EXECUTABLE(vbayesgroup vbayesgroup.c)
TARGET_LINK_LIBRARIES(vbayesgroup lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vbayesgroup PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vbayesgroup
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
