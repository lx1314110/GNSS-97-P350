cmake_minimum_required(VERSION 2.8)

aux_source_directory(. LCD_SRC_LIST)
add_executable(p350_keylcd ${LCD_SRC_LIST})
target_link_libraries(p350_keylcd p350 pthread)
install(TARGETS p350_keylcd DESTINATION p350/app/proc)
