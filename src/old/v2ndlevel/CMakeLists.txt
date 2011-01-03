PROJECT(V2NDLEVEL)

INCLUDE_DIRECTORIES(../lib_lipsia)

ADD_EXECUTABLE(v2ndlevel v2ndlevel.c Read2ndLevelDesign.c)
TARGET_LINK_LIBRARIES(v2ndlevel lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(v2ndlevel
                      PROPERTIES COMPILE_FLAGS -ansi
                      LINK_FLAGS -Wl)

INSTALL(TARGETS v2ndlevel
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
