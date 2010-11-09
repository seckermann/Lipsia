PROJECT(vtwosample_ttest)

ADD_EXECUTABLE(vtwosample_ttest vtwosample_ttest.c Gaussianize.c)
TARGET_LINK_LIBRARIES(vtwosample_ttest lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vtwosample_ttest PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vtwosample_ttest
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
