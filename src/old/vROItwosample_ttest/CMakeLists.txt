PROJECT(vROItwosample_ttest)

ADD_EXECUTABLE(vROItwosample_ttest vROItwosample_ttest.c)
TARGET_LINK_LIBRARIES(vROItwosample_ttest lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vROItwosample_ttest PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vROItwosample_ttest
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
