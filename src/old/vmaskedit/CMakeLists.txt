PROJECT(vmaskedit)

ADD_EXECUTABLE(vmaskedit vmaskedit.c)
TARGET_LINK_LIBRARIES(vmaskedit lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vmaskedit PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vmaskedit
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
