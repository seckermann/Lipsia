PROJECT(vblobsize)

ADD_EXECUTABLE(vblobsize BlobSize.c vblobsize.c)
TARGET_LINK_LIBRARIES(vblobsize lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vblobsize PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vblobsize
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
