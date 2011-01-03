PROJECT(vgetcontrast)

INCLUDE_DIRECTORIES(../lib_lipsia)

ADD_EXECUTABLE(vgetcontrast vgetcontrast.c)
TARGET_LINK_LIBRARIES(vgetcontrast lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vgetcontrast PROPERTIES
                      COMPILE_FLAGS -ansi
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vgetcontrast
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
