PROJECT(vimagetimecourse)

ADD_EXECUTABLE(vimagetimecourse vimagetimecourse.c)
TARGET_LINK_LIBRARIES(vimagetimecourse lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vimagetimecourse PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vimagetimecourse
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
