cmake_minimum_required(VERSION 2.8)

aux_source_directory(. MANAGER_SRC_LIST)
add_executable(p350_manager ${MANAGER_SRC_LIST})
target_link_libraries(p350_manager p350 sqlite3)
install(TARGETS p350_manager DESTINATION p350/app/proc)