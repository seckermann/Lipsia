PROJECT(vconvcoord)

ADD_EXECUTABLE(vconvcoord vconvcoord.c)
TARGET_LINK_LIBRARIES(vconvcoord lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vconvcoord PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vconvcoord
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
