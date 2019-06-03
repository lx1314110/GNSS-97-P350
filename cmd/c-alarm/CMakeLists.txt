cmake_minimum_required(VERSION 2.8)

aux_source_directory(. ALM_SRC_LIST)
add_executable(p350alm ${ALM_SRC_LIST})
target_link_libraries(p350alm p350 sqlite3)
install(TARGETS p350alm DESTINATION p350/app/cmd)