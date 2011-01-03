PROJECT(vtimestep)

ADD_EXECUTABLE(vtimestep vtimestep.c)
TARGET_LINK_LIBRARIES(vtimestep lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vtimestep PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vtimestep
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
