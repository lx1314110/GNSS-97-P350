cmake_minimum_required(VERSION 2.8)

aux_source_directory(. LIB_SRC_LIST)
#message("src_lis"${DIR_SIRCS_LIB})
#compatibility alloc
set(ALLOC_SRC ../alloc/alloc.c)
#add_library(p350_shared SHARED ${LIB_SRC_LIST})
add_library(p350 STATIC ${LIB_SRC_LIST} ${ALLOC_SRC})
#add_library(p350 STATIC  ${ALLOC_SRC})
#set_target_properties(p350_shared PROPERTIES OUTPUT_NAME "p350")
