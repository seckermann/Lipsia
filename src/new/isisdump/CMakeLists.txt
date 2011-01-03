PROJECT(vdump)
add_executable(vdump isisdump.cpp)

# target_link_libraries(vdump ${ISIS_LIB} ${ISIS_LIB_DEPENDS} ${Boost_LIBRARIES} dl /usr/lib/liboil-0.3.a)
target_link_libraries(vdump ${ISIS_LIB} ${ISIS_LIB_DEPENDS} ${Boost_LIBRARIES} dl )

SET_TARGET_PROPERTIES(vdump PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vdump
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)

