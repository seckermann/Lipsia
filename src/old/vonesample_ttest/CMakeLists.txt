PROJECT(vonesample_ttest)

ADD_EXECUTABLE(vonesample_ttest vonesample_ttest.c)
TARGET_LINK_LIBRARIES(vonesample_ttest lipsia $ {VIA_LIBRARY})

SET_TARGET_PROPERTIES(vonesample_ttest PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vonesample_ttest
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
