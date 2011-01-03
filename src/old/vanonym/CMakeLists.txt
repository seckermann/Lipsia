PROJECT(vanonym)

ADD_EXECUTABLE(vanonym vanonym.c)
TARGET_LINK_LIBRARIES(vanonym lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vanonym PROPERTIES
# not ansi since uses c++ comments
#COMPILE_FLAGS -ansi
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vanonym
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
