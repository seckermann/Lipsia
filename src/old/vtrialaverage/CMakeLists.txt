PROJECT(vtrialaverage)

INCLUDE_DIRECTORIES(../lib_lipsia)

ADD_EXECUTABLE(vtrialaverage vtrialaverage.c)
TARGET_LINK_LIBRARIES(vtrialaverage lipsia ${VIA_LIBRARY})

SET_TARGET_PROPERTIES(vtrialaverage PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vtrialaverage
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)
