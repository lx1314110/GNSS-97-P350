cmake_minimum_required(VERSION 2.8)

aux_source_directory(. SYS_SRC_LIST)
add_executable(p350sys ${SYS_SRC_LIST})
target_link_libraries(p350sys p350 sqlite3)

# update date gcc-version
add_custom_command(TARGET p350sys 
            POST_BUILD 
            COMMAND  touch ${PROJECT_SOURCE_DIR}/cmd/c-sys/csys_parser.c
            )
install(TARGETS p350sys DESTINATION p350/app/cmd)