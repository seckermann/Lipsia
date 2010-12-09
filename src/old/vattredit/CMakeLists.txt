PROJECT(vattredit)

ADD_EXECUTABLE(vattredit vattredit.c)
TARGET_LINK_LIBRARIES(vattredit lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vattredit PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vattredit
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
