PROJECT(visotrop)

ADD_EXECUTABLE(visotrop visotrop.c)
TARGET_LINK_LIBRARIES(visotrop lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(visotrop PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS visotrop
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
