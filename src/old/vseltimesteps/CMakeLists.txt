PROJECT(vseltimesteps)

ADD_EXECUTABLE(vseltimesteps vseltimesteps.c)
TARGET_LINK_LIBRARIES(vseltimesteps lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vseltimesteps PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vseltimesteps
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
