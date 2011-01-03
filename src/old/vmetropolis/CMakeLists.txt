PROJECT(vmetropolis)

ADD_EXECUTABLE(vmetropolis vmetropolis.c)
TARGET_LINK_LIBRARIES(vmetropolis lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vmetropolis PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vmetropolis
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
