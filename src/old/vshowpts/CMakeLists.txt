PROJECT(vshowpts)

ADD_EXECUTABLE(vshowpts vshowpts.c)
TARGET_LINK_LIBRARIES(vshowpts lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vshowpts PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vshowpts
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
