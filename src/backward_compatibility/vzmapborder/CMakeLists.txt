PROJECT(vzmapborder)

ADD_EXECUTABLE(vzmapborder vzmapborder.c)
TARGET_LINK_LIBRARIES(vzmapborder lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vzmapborder PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vzmapborder
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
