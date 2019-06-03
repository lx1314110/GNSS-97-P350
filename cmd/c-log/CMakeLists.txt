cmake_minimum_required(VERSION 2.8)

aux_source_directory(. LOG_SRC_LIST)
add_executable(p350log ${LOG_SRC_LIST})
target_link_libraries(p350log p350 sqlite3)
install(TARGETS p350log DESTINATION p350/app/cmd)