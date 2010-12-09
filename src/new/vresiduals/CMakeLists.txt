PROJECT(vresiduals)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

ADD_EXECUTABLE(vresiduals vresiduals.c BlockIO.c GlobalMean.c gauss.c gsl_dutils.c gsl_futils.c gsl_utils.h)
TARGET_LINK_LIBRARIES(vresiduals ${GSL_GSL_LIBRARY} ${GSL_CBLAS_LIBRARY} ${VIAIO_LIBRARY} ${LIPSIA_LIBRARY})

SET_TARGET_PROPERTIES(vresiduals PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vresiduals
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
