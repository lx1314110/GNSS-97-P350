cmake_minimum_required(VERSION 2.8)

aux_source_directory(. WG_SRC_LIST)
add_executable(p350_wg ${WG_SRC_LIST})
target_link_libraries(p350_wg p350 sqlite3)
install(TARGETS p350_wg DESTINATION p350/app/proc)