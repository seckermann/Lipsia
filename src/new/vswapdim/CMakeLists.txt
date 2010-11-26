PROJECT(vswapdim)
add_executable(vswapdim isisswap.cpp)

target_link_libraries(vswapdim ${ISIS_LIB} ${ISIS_LIB_DEPENDS} ${Boost_LIBRARIES} dl /usr/lib/liboil-0.3.a)

SET_TARGET_PROPERTIES(vswapdim PROPERTIES
	    LINK_FLAGS -Wl)

INSTALL(TARGETS vswapdim
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
					
