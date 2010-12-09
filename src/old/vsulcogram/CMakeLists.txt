PROJECT(vsulcogram)

ADD_EXECUTABLE(vsulcogram CleanImage.c vsulcogram.c)
TARGET_LINK_LIBRARIES(vsulcogram lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vsulcogram PROPERTIES
    LINK_FLAGS -Wl)

INSTALL(TARGETS vsulcogram
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
