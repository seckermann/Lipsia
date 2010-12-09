PROJECT(vnormalize)

ADD_EXECUTABLE(vnormalize Normalize.c vnormalize.c)
TARGET_LINK_LIBRARIES(vnormalize lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vnormalize PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vnormalize
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
