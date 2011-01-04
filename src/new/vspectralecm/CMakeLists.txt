PROJECT(vspectralecm)

ADD_EXECUTABLE(vspectralecm vspectralecm.c)
TARGET_LINK_LIBRARIES(vspectralecm lipsia ${VIAIO_LIBRARY} ${LIPSIA_LIBRARY})

SET_TARGET_PROPERTIES(vspectralecm PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vspectralecm
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
