cmake_minimum_required(VERSION 2.8)

aux_source_directory(. NTP_SRC_LIST)
add_executable(p350_ntp ${NTP_SRC_LIST})
target_link_libraries(p350_ntp p350 sqlite3)
install(TARGETS p350_ntp DESTINATION p350/app/proc)