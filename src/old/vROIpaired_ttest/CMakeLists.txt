PROJECT(vROIpaired_ttest)

ADD_EXECUTABLE(vROIpaired_ttest vROIpaired_ttest.c)
TARGET_LINK_LIBRARIES(vROIpaired_ttest lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vROIpaired_ttest PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vROIpaired_ttest
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
