PROJECT(vmaskave)

ADD_EXECUTABLE(vmaskave vmaskave.c)
TARGET_LINK_LIBRARIES(vmaskave lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vmaskave PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vmaskave
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
