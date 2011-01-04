PROJECT(vcovariates)


ADD_EXECUTABLE(vcovariates vcovariates.c)
TARGET_LINK_LIBRARIES(vcovariates lipsia ${VIAIO_LIBRARY} ${GSL_CBLAS_LIBRARY} ${LIPSIA_LIBRARY})

SET_TARGET_PROPERTIES(vcovariates PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vcovariates
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
