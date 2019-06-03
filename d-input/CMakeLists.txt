cmake_minimum_required(VERSION 2.8)

aux_source_directory(. INPUT_SRC_LIST)
add_executable(p350_input ${INPUT_SRC_LIST})
target_link_libraries(p350_input p350 sqlite3)
install(TARGETS p350_input DESTINATION p350/app/proc)