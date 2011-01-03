PROJECT(vdifftimecourse)

ADD_EXECUTABLE(vdifftimecourse vdifftimecourse.c)
TARGET_LINK_LIBRARIES(vdifftimecourse lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vdifftimecourse PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vdifftimecourse
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
