PROJECT(vdemon)

ADD_EXECUTABLE(vdemon maxwell.C vdemon.C)
TARGET_LINK_LIBRARIES(vdemon lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vdemon PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vdemon
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
