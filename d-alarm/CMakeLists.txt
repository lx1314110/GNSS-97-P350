cmake_minimum_required(VERSION 2.8)

aux_source_directory(. ALARM_SRC_LIST)
add_executable(p350_alarm ${ALARM_SRC_LIST})
target_link_libraries(p350_alarm p350 sqlite3)
install(TARGETS p350_alarm DESTINATION p350/app/proc)