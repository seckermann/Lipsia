#the new programs coming from the lipsia sandbox package
add_subdirectory(new)
#the old programs coming from the lipsia package
add_subdirectory(old)
#old programs which are replaced by new programs but maintained due to backward compatibility
add_subdirectory(backward_compatibility)

option(DEV_PROGRAMS "Dev-Programs" OFF)
if(DEV_PROGRAMS)
    add_subdirectory(dev)
endif(DEV_PROGRAMS)
