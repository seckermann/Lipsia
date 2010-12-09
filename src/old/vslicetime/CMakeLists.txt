PROJECT(vslicetime)

ADD_EXECUTABLE(vslicetime vslicetime.c)
TARGET_LINK_LIBRARIES(vslicetime lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vslicetime PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vslicetime
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
