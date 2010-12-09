PROJECT(vwhiteglm)

INCLUDE_DIRECTORIES(../lib_lipsia)

ADD_DEFINITIONS(-D_GNU_SOURCE -DHAVE_INLINE=1 -DGSL_RANGE_CHECK=0)

ADD_EXECUTABLE(vwhiteglm
    BlockIO.c whitecov2.c dfs.c prewhite.c vwhiteglm.c whitereg.c via2mat.c
    whitecov.c)
TARGET_LINK_LIBRARIES(vwhiteglm lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vwhiteglm PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vwhiteglm
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
