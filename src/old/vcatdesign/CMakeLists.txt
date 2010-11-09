PROJECT(vcatdesign)

ADD_EXECUTABLE(vcatdesign vcatdesign.c)
TARGET_LINK_LIBRARIES(vcatdesign lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vcatdesign PROPERTIES
    COMPILE_FLAGS -ansi
    LINK_FLAGS -Wl)

INSTALL(TARGETS vcatdesign
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
