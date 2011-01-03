PROJECT(vbrainpeel)

ADD_EXECUTABLE(vbrainpeel vbrainpeel.c VBrainpeel.c LocalContrast.c)
TARGET_LINK_LIBRARIES(vbrainpeel lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vbrainpeel PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vbrainpeel
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
