PROJECT(vwilcoxon)

ADD_EXECUTABLE(vwilcoxon vwilcoxon.c)
TARGET_LINK_LIBRARIES(vwilcoxon lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vwilcoxon PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vwilcoxon
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
