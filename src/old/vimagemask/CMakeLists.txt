PROJECT(vimagemask)

ADD_EXECUTABLE(vimagemask vimagemask.c)
TARGET_LINK_LIBRARIES(vimagemask lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vimagemask PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vimagemask
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
