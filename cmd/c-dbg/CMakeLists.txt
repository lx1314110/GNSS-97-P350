cmake_minimum_required(VERSION 2.8)

aux_source_directory(. DBG_SRC_LIST)
add_executable(p350dbg ${DBG_SRC_LIST})
target_link_libraries(p350dbg p350 )
install(TARGETS p350dbg DESTINATION p350/app/cmd)