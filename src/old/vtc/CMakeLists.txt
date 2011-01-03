PROJECT(vtc)

INCLUDE_DIRECTORIES(../lib_lipsia)

ADD_EXECUTABLE(vtc vtc.c PCA.c Covariance.c)
TARGET_LINK_LIBRARIES(vtc lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vtc PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vtc
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
