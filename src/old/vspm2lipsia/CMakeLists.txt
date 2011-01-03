PROJECT(vspm2lipsia)

ADD_EXECUTABLE(vspm2lipsia vspm2lipsia.c)
TARGET_LINK_LIBRARIES(vspm2lipsia lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vspm2lipsia PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vspm2lipsia
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
