PROJECT(vpaired_wilcoxtest)

ADD_EXECUTABLE(vpaired_wilcoxtest vpaired_wilcoxtest.c)
TARGET_LINK_LIBRARIES(vpaired_wilcoxtest lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vpaired_wilcoxtest PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vpaired_wilcoxtest
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
