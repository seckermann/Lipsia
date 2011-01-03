PROJECT(vswapdim)
add_executable(vswapdim isisswap.cpp)

# target_link_libraries(vswapdim ${ISIS_LIB} ${ISIS_LIB_DEPENDS} dl /usr/lib/liboil-0.3.a)
target_link_libraries(vswapdim $ {ISIS_LIB} $ {ISIS_LIB_DEPENDS} dl )

SET_TARGET_PROPERTIES(vswapdim PROPERTIES
                      LINK_FLAGS -Wl)

INSTALL(TARGETS vswapdim
        RUNTIME DESTINATION bin
        COMPONENT RuntimeLibraries)

