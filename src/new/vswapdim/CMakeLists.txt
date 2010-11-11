PROJECT(vswapdim)
add_executable(vswapdim isisswap.cpp)

target_link_libraries(vswapdim ${ISIS_DIR}/lib/libisis_core.a ${Boost_LIBRARIES} dl /usr/lib/liboil-0.3.a)

SET_TARGET_PROPERTIES(vswapdim PROPERTIES
	    LINK_FLAGS -Wl)

INSTALL(TARGETS vswapdim
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
					
