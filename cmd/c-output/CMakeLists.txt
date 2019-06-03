cmake_minimum_required(VERSION 2.8)

aux_source_directory(. OUT_SRC_LIST)
add_executable(p350out ${OUT_SRC_LIST})
target_link_libraries(p350out p350 )
install(TARGETS p350out DESTINATION p350/app/cmd)