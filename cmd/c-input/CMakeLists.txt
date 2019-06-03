cmake_minimum_required(VERSION 2.8)

aux_source_directory(. IN_SRC_LIST)
add_executable(p350in ${IN_SRC_LIST})
target_link_libraries(p350in p350 )
install(TARGETS p350in DESTINATION p350/app/cmd)