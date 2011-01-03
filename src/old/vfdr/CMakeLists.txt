PROJECT(vfdr)

ADD_EXECUTABLE(vfdr vfdr.c)
TARGET_LINK_LIBRARIES(vfdr lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vfdr PROPERTIES
                      COMPILE_FLAGS -ansi
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vfdr
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
