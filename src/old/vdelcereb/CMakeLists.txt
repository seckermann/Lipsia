PROJECT(vdelcereb)

ADD_EXECUTABLE(vdelcereb vdelcereb.c)
TARGET_LINK_LIBRARIES(vdelcereb lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vdelcereb PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vdelcereb
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
