PROJECT(vrmglobal)

INCLUDE_DIRECTORIES(../../old/lib_lipsia)

ADD_EXECUTABLE(vrmglobal vrmglobal.c GlobalMean.c gauss.c)
TARGET_LINK_LIBRARIES(vrmglobal lipsia ${VIAIO_LIBRARY} ${GSL_CBLAS_LIBRARY} ${LIPSIA_LIBRARY})

SET_TARGET_PROPERTIES(vrmglobal PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vrmglobal
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
