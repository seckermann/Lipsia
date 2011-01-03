PROJECT(vsendserv)

ADD_EXECUTABLE(vsendserv vsendserv.C)
TARGET_LINK_LIBRARIES(vsendserv lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vsendserv PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vsendserv
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
