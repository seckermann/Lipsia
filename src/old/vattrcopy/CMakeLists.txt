PROJECT(vattrcopy)

ADD_EXECUTABLE(vattrcopy vattrcopy.c)
TARGET_LINK_LIBRARIES(vattrcopy lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vattrcopy PROPERTIES
                      COMPILE_FLAGS -ansi
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vattrcopy
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
