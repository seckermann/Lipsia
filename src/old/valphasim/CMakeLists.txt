PROJECT(valphasim)

ADD_EXECUTABLE(valphasim valphasim.c GaussFilter.c)
TARGET_LINK_LIBRARIES(valphasim lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(valphasim
                      PROPERTIES COMPILE_FLAGS -ansi
                      LINK_FLAGS -Wl)

INSTALL(TARGETS valphasim
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
