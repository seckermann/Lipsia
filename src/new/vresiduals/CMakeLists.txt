PROJECT(vresiduals)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

ADD_EXECUTABLE(vresiduals vresiduals.c BlockIO.c GlobalMean.c gauss.c gsl_dutils.c gsl_futils.c gsl_utils.h)
TARGET_LINK_LIBRARIES(vresiduals ${VIA_LIBRARY} ${LIPSIA_LIBRARY})

SET_TARGET_PROPERTIES(vresiduals PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vresiduals
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
