#!/bin/bash

set -e
#set -x
UPGRADE_WEB_DIR=./upgrade_web
SW_VER=$(cat d-manager/dmgr_version.h | grep "#define" |grep "SW_VER" | awk '{print $3}' | cut -d \" -f2 | cut -d V -f2 | cut -d v -f2)

[ -n "$DEBUG" ] || DEBUG=0
DBG=${DEBUG}

ONLY_APP_TAR=${UPGRADE_WEB_DIR}/app-v${SW_VER}.tar.bz2
FPGA_APP_TAR=${UPGRADE_WEB_DIR}/app-v${SW_VER}_fpga.tar.bz2
FPGA_DB_APP_TAR=${UPGRADE_WEB_DIR}/app-v${SW_VER}_fpga_db.tar.bz2
FPGA_ALL_APP_TAR=${UPGRADE_WEB_DIR}/app-v${SW_VER}_all_fpga_web_db.tar.bz2
FPGA_SNMP_ALL_APP_TAR=${UPGRADE_WEB_DIR}/app-v${SW_VER}_all_snmp_fpga_web_db.tar.bz2
FPGA_ALL_APP_INIT_TAR=${UPGRADE_WEB_DIR}/app-v${SW_VER}_all_fpga_web_db_init.tar.bz2
FPGA_SNMP_ALL_APP_INIT_TAR=${UPGRADE_WEB_DIR}/app-v${SW_VER}_all_snmp_fpga_web_db_init.tar.bz2

TMP_APP_TAR=${UPGRADE_WEB_DIR}/app-v${SW_VER}.tar.bz2

#compile
echo "Compile ..."
make && echo "Compile finish!"

echo "Install ..."
#remove old file
rm -rf ${ONLY_APP_TAR} ${FPGA_APP_TAR} ${FPGA_DB_APP_TAR} ${FPGA_ALL_APP_TAR} ${FPGA_ALL_APP_INIT_TAR} \
	   ${FPGA_SNMP_ALL_APP_TAR} ${FPGA_SNMP_ALL_APP_INIT_TAR}

echo "Generating the (snmp_fpga_web_db_init) all_app ..."
DEBUG=${DBG} SNMP=1 DB=1 WEB=1 FPGA=1 INIT=1 make install > /dev/null  && {
	mv ${TMP_APP_TAR} ${FPGA_SNMP_ALL_APP_INIT_TAR}
	echo "Generate the (snmp_fpga_web_db_init) all_app finish!"
} || {
	echo "Generate the (snmp_fpga_web_db_init) all_app failure!"
	exit 1
}

echo "Generating the (fpga_web_db_init) all_app ..."
DEBUG=${DBG} DB=1 WEB=1 FPGA=1 INIT=1 make install > /dev/null  && {
	mv ${TMP_APP_TAR} ${FPGA_ALL_APP_INIT_TAR}
	echo "Generate the (fpga_web_db_init) all_app finish!"
} || {
	echo "Generate the (fpga_web_db_init) all_app failure!"
	exit 1
}

echo "Generating the (snmp_fpga_web_db) all_app ..."
DEBUG=${DBG} SNMP=1 DB=1 WEB=1 FPGA=1 make install > /dev/null  && {
	mv ${TMP_APP_TAR} ${FPGA_SNMP_ALL_APP_TAR}
	echo "Generate the (snmp_fpga_web_db) all_app finish!"
} || {
	echo "Generate the (snmp_fpga_web_db) all_app failure!"
	exit 1
}

echo "Generating the (fpga_web_db) all_app ..."
DEBUG=${DBG} DB=1 WEB=1 FPGA=1 make install > /dev/null  && {
	mv ${TMP_APP_TAR} ${FPGA_ALL_APP_TAR}
	echo "Generate the (fpga_web_db) all_app finish!"
} || {
	echo "Generate the (fpga_web_db) all_app failure!"
	exit 1
}

echo "Generating the fpga_db_app ..."
DEBUG=${DBG} DB=1 FPGA=1 make install > /dev/null && {
	mv ${TMP_APP_TAR} ${FPGA_DB_APP_TAR}
	echo "Generate the fpga_db_app finish!"
} || {
	echo "Generate the fpga_db_app failure!"
	exit 1
}

echo "Generating the fpga_app ..."
DEBUG=${DBG} FPGA=1 make install > /dev/null  && {
	mv ${TMP_APP_TAR} ${FPGA_APP_TAR}
	echo "Generate the fpga_app finish!"
} || {
	echo "Generate the fpga_app failure!"
	exit 1
}

echo "Generating the app ..."
DEBUG=${DBG} make install > /dev/null && {
	[ "${TMP_APP_TAR}" = "${ONLY_APP_TAR}" ] || mv ${TMP_APP_TAR} ${ONLY_APP_TAR}
	echo "Generate the app finish!"
} || {
	echo "Generate the app failure!"
	exit 1
}

echo "Install finish!"
