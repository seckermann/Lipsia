PROJECT(vnormvals)

ADD_EXECUTABLE(vnormvals vnormvals.c)
TARGET_LINK_LIBRARIES(vnormvals m $ {VIAIO_LIBRARY} $ {LISPIA_LIBRARY})

SET_TARGET_PROPERTIES(vnormvals PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vnormvals
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
