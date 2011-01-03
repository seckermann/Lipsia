PROJECT(vcacp)

ADD_EXECUTABLE(vcacp vcacp.c Rotate.c Shear.c)
# GaussConv.c Kernel.c ConvolveSep.c Convolve.c
TARGET_LINK_LIBRARIES(vcacp lipsia $ {VIA_LIBRARY})

# it is very IMPORTANT to provide a directory with double quotes.
SET_TARGET_PROPERTIES(vcacp PROPERTIES
                      COMPILE_DEFINITIONS TEMPLATE_FILE="${LIPSIA_INSTALL_RESOURCE_DIR}/vcacp.v"
                            LINK_FLAGS -Wl)

INSTALL(TARGETS vcacp
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)

