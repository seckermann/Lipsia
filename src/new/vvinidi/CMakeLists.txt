PROJECT(vvinidi)
add_executable(vvinidi isisconv.cpp)

# target_link_libraries(vvinidi ${ISIS_LIB} ${ISIS_LIB_DEPENDS} ${Boost_LIBRARIES} dl /usr/lib/liboil-0.3.a)
target_link_libraries(vvinidi ${ISIS_LIB} ${ISIS_LIB_DEPENDS} ${Boost_LIBRARIES} dl )

SET_TARGET_PROPERTIES(vvinidi PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vvinidi
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)

