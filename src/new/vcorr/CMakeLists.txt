PROJECT(vcorr)

ADD_EXECUTABLE(vcorr vcorr.c ROI.c MI.c KendallDist.c Correlation.c)
TARGET_LINK_LIBRARIES(vcorr ${VIAIO_LIBRARY} ${LIPSIA_LIBRARY}
                      ${GSL_GSL_LIBRARY} ${GSL_CBLAS_LIBRARY})

SET_TARGET_PROPERTIES(vcorr PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vcorr
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
