PROJECT(vspectral)

ADD_EXECUTABLE(vspectral vspectral.c)
TARGET_LINK_LIBRARIES(vspectral lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vspectral PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vspectral
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
