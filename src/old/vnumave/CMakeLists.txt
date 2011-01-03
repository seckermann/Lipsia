PROJECT(vnumave)

ADD_EXECUTABLE(vnumave vnumave.c)
TARGET_LINK_LIBRARIES(vnumave lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vnumave PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vnumave
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
