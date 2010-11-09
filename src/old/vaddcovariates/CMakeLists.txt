PROJECT(vaddcovariates)

ADD_EXECUTABLE(vaddcovariates vaddcovariates.c)
TARGET_LINK_LIBRARIES(vaddcovariates lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vaddcovariates
    PROPERTIES COMPILE_FLAGS -ansi
    LINK_FLAGS -Wl)

INSTALL(TARGETS vaddcovariates
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
