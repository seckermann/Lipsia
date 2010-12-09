PROJECT(vmapscale)

ADD_EXECUTABLE(vmapscale vmapscale.c)
TARGET_LINK_LIBRARIES(vmapscale lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vmapscale PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vmapscale
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
