PROJECT(vtrialaver)

INCLUDE_DIRECTORIES(../lib_lipsia)

ADD_EXECUTABLE(vtrialaver vtrialaverage.c)
TARGET_LINK_LIBRARIES(vtrialaver lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vtrialaver PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vtrialaver
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
