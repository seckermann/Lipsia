PROJECT(vbayes)

ADD_EXECUTABLE(vbayes vbayes.c)
TARGET_LINK_LIBRARIES(vbayes lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vbayes PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vbayes
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
