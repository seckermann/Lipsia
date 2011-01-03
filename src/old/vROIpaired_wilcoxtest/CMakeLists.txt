PROJECT(vROIpaired_wilcoxtest)

ADD_EXECUTABLE(vROIpaired_wilcoxtest vROIpaired_wilcoxtest.c)
TARGET_LINK_LIBRARIES(vROIpaired_wilcoxtest lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vROIpaired_wilcoxtest PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vROIpaired_wilcoxtest
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
