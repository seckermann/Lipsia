PROJECT(vdump)
add_executable(vdump isisdump.cpp)

target_link_libraries(vdump ${ISIS_LIB} ${ISIS_LIB_DEPENDS} ${Boost_LIBRARIES} dl /usr/lib/liboil-0.3.a)

SET_TARGET_PROPERTIES(vdump PROPERTIES
	    LINK_FLAGS -Wl)

INSTALL(TARGETS vdump
        RUNTIME DESTINATION ${LIPSIA_INSTALL_BIN_DIR}
        COMPONENT RuntimeLibraries)
					
