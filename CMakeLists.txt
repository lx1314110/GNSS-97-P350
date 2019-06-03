cmake_minimum_required(VERSION 2.8)
# let cmake output more info
#set(CMAKE_VERBOSE_MAKEFILE ON)

# use relatice path for FILE
SET(CMAKE_USE_RELATIVE_PATHS ON)
# automagically detect if we should cross-compile
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(TOOLCHAIN /home/farsight/arm-linux-gcc/usr/local/arm/4.3.2/bin/arm-linux-)
set(CMAKE_C_COMPILER	${TOOLCHAIN}gcc)
#set(CMAKE_AR	"${TOOLCHAIN}ar" CACHE FILEPATH "CW archiver" FORCE)

project(GNSS_P350)
#set(TARGET_OUT_PATH ${PROJECT_BINARY_DIR}/target)
#set(LIBRARY_OUTPUT_PATH ${TARGET_OUT_PATH}/lib)
#set(EXECUTABLE_OUTPUT_PATH ${TARGET_OUT_PATH}/bin)
set(CMAKE_INSTALL_PREFIX  ${PROJECT_BINARY_DIR})
set(P350_VERSION_MAJOR "02")
set(P350_VERSION_MINOR "04")
set(P350_VERSION_PATCH "0")

include_directories(
    alloc/include
    lib/include
)

link_directories(
	/${PROJECT_SOURCE_DIR}/so
)


# build opt
add_definitions( -Wall )
# include sub dir

add_subdirectory(lib)
add_subdirectory(d-http)
add_subdirectory(d-61850)
add_subdirectory(d-ntp)
add_subdirectory(d-manager)
add_subdirectory(d-input)
add_subdirectory(d-output)
add_subdirectory(d-alarm)
add_subdirectory(d-keylcd)
add_subdirectory(d-wg)
add_subdirectory(script)
#add cmd
add_subdirectory(cmd)

# compatibility for db
add_custom_target(db  
                  COMMENT "copy p350 db "
                  COMMAND rm  ${PROJECT_BINARY_DIR}/db -rf
                  COMMAND mkdir  ${PROJECT_BINARY_DIR}/db 
                  COMMAND cp ${PROJECT_SOURCE_DIR}/p350.db ${PROJECT_BINARY_DIR}/db/p350.db -f
                  COMMENT "p350 package db "
                  COMMAND tar jcf db-v${P350_VERSION_MAJOR}.${P350_VERSION_MINOR}.tar.bz2 db
                  )
add_custom_target(app 
                  COMMAND echo "install app "
                  COMMAND make install
                  COMMAND rm ${PROJECT_BINARY_DIR}/app -rf
                  COMMAND cp  ${PROJECT_BINARY_DIR}/p350/app  ${PROJECT_BINARY_DIR}/app -Rf
                  COMMAND echo "p350 package app "
                  COMMAND tar jcf app-v${P350_VERSION_MAJOR}.${P350_VERSION_MINOR}.tar.bz2 app
                  COMMAND rm ${PROJECT_BINARY_DIR}/app -rf
                  )
#install script

#automatic generate version file
configure_file(
  ${PROJECT_SOURCE_DIR}/d-manager/dmgr_version.h.in
  ${PROJECT_SOURCE_DIR}/d-manager/dmgr_version.h
)


# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
#set (CPACK_PACKAGE_VERSION_MAJOR "${P350_VERSION_MAJOR}")
#set (CPACK_PACKAGE_VERSION_MINOR "${P350_VERSION_MINOR}")
set(CPACK_PACKAGE_FILE_NAME app-V${P350_VERSION_MAJOR}-${P350_VERSION_MINOR} )
set(CPACK_PACKAGE_NAME app )
set (CPACK_PACKAGE_CONTACT       "linqinga@datang.com")
set( CPACK_GENERATOR  TBZ2)

include (CPack)
