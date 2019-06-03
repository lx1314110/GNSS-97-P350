cmake_minimum_required(VERSION 2.8)

aux_source_directory(. OUTPUT_SRC_LIST)
add_executable(p350_output ${OUTPUT_SRC_LIST})
target_link_libraries(p350_output p350 sqlite3)
install(TARGETS p350_output DESTINATION p350/app/proc)
