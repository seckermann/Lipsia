PROJECT(vzmax)

ADD_EXECUTABLE(vzmax ZMax.c vzmax.c)
TARGET_LINK_LIBRARIES(vzmax lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vzmax PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vzmax
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
