PROJECT(vcolorglm)

ADD_DEFINITIONS(-DHAVE_INLINE=1 -DGSL_RANGE_CHECK=0)

INCLUDE_DIRECTORIES(../lib_lipsia)

ADD_EXECUTABLE(vcolorglm vcolorglm.c gauss.c BlockIO.c)
TARGET_LINK_LIBRARIES(vcolorglm lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vcolorglm PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vcolorglm
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
