PROJECT(vpaired_ttest)

ADD_EXECUTABLE(vpaired_ttest vpaired_ttest.c)
TARGET_LINK_LIBRARIES(vpaired_ttest lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vpaired_ttest PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vpaired_ttest
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
