#!/bin/bash

#httpd not supoort:/sbin/,so add PATH
PATH=/usr/p350/app/cmd:/usr/p350/snmp/sbin:/usr/p350/snmp/bin:/sbin:/usr/sbin:/bin:/usr/bin:$PATH

MYNAME=$(basename $0)

#arg1 is web or console
arg1="$1"
[ -n "${arg1}" ] || arg1="console"
who_done=${arg1}

[ -e "/etc/default/upgrade.conf" ] && . /etc/default/upgrade.conf
[ -n "${REBOOT_FLAG}" ] || REBOOT_FLAG=1
if [ "${who_done}" = "web" ]; then
	REBOOT_FLAG=0
fi

TMP_DIR=/tmp
UPDATE_DIR=/usr/p350

#P350_TEMP_DIR=${TMP_DIR}/tmp/p350_upgrade
#new temp deal
P350_TEMP_DIR=${UPDATE_DIR}/tmp

UPGRADE_UPDATE_DIR=${UPDATE_DIR}/upgrade
#UPGRADE_TEMP_DIR=${UPDATE_DIR}/tmp
#new temp deal
UPGRADE_TEMP_DIR=${TMP_DIR}/p350_upgrade

[ -n "${ALL_APP}" ] || ALL_APP="app www fpga db mac snmp"

LOCK_FILE=/tmp/.web_upgrade_download

echo_console(){
	echo "$@"
}

show_logger(){
	local level="$1"
	local msg="$2"
	[ -n "$msg" ] || msg="$@"
	echo_console "$msg"
	if [ "$level" = "info" ]; then
		logger -p local0.info  -t ${MYNAME} "$msg"
	elif [ "$level" = "notice" ]; then
		logger -p local0.notice  -t ${MYNAME} "$msg"
	elif [ "$level" = "err" ]; then
		logger -p local0.err  -t ${MYNAME} "$msg"
	elif [ "$level" = "debug" ]; then
		logger -p local0.debug  -t ${MYNAME} "$msg"
	else
		logger -p local0.info  -t ${MYNAME} "$msg"
	fi
}

#main

#${UPDATE_DIR}/app/proc/busybox md5sum /usr/p350/tmp/www
[ -e "/usr/bin/md5sum" ] && MD5_BIN="/usr/bin/md5sum" || {
	${UPDATE_DIR}/app/proc/busybox | grep -q md5sum && MD5_BIN="${UPDATE_DIR}/app/proc/busybox md5sum"
}
[ -e "/bin/sync" ] && SYNC="/bin/sync" || {
	${UPDATE_DIR}/app/proc/busybox | grep -q sync && SYNC="${UPDATE_DIR}/app/proc/busybox sync"
}
[ -e "/bin/touch" ] && TOUCH_BIN="/bin/touch" || {
        ${UPDATE_DIR}/app/proc/busybox | grep -q touch && TOUCH_BIN="${UPDATE_DIR}/app/proc/busybox touch"
}

#new temp deal
if true; then
	[ -d "${P350_TEMP_DIR}" ] && {
		rm -rf ${UPGRADE_TEMP_DIR}
		cp -rv ${P350_TEMP_DIR} ${UPGRADE_TEMP_DIR}
	} || {
		show_logger err "upgrade: no exist ${P350_TEMP_DIR} dir"
		#echo "upgrade: no exist ${P350_TEMP_DIR} dir"
		exit 1
	}
fi

[ -d "${UPGRADE_TEMP_DIR}" ] && cd ${UPGRADE_TEMP_DIR} || {
	show_logger err "upgrade: no exist ${UPGRADE_TEMP_DIR} dir"
	#echo "upgrade: no exist ${UPGRADE_TEMP_DIR} dir"
	exit 1
}

[ -e "%LOCK_FILE" ] && {
	show_logger err ${MYNAME} "$LOCK_FILE exist!"
	#echo "$LOCK_FILE exist!"
	exit 1;
}
${TOUCH_BIN} $LOCK_FILE
success_flag=0
for filename in `ls`
do
	filefirst=${filename%%-*}
	filelast=${filename##*.}
	for cur_app in ${ALL_APP}; do
		if [ "${filefirst}" = "${cur_app}" -a "${filelast}" = "bz2" ];then
			#backup old software
			rm -fR ${UPDATE_DIR}/bak/${cur_app}*.tar.bz2
			[ -d "${UPDATE_DIR}/${cur_app}" ] && {
				cd ${UPDATE_DIR}
				mkdir -p ${UPDATE_DIR}/bak
				tar jcf ${UPDATE_DIR}/bak/${cur_app}.tar.bz2 ${cur_app}
			}
		
			#untar new software
			if `tar jxf ${UPGRADE_TEMP_DIR}/${filename} -C ${UPGRADE_TEMP_DIR}`; then
				if [ "${cur_app}" = "fpga" ];then
					upname=$(ls ${UPGRADE_TEMP_DIR}/${cur_app} | grep ".rbf$")
				else
					upname=$(ls ${UPGRADE_TEMP_DIR}/${cur_app} | grep "^${cur_app}" | grep "bz2$")
				fi
				md5_sum=`${MD5_BIN} ${UPGRADE_TEMP_DIR}/${cur_app}/${upname} | awk -F " " '{print $1}'`
				md5=`cat ${UPGRADE_TEMP_DIR}/${cur_app}/md5.txt`
				
				if [ "${md5_sum}" = "${md5}" ];then
					if [ "${cur_app}" = "fpga" ];then	
						rm ${UPGRADE_TEMP_DIR}/${cur_app}/md5.txt
						mv ${UPGRADE_TEMP_DIR}/${cur_app}/$upname ${UPGRADE_TEMP_DIR}/${cur_app}/fpga.rbf
						cp -rv ${UPGRADE_TEMP_DIR}/${cur_app} ${UPGRADE_UPDATE_DIR} && {
							success_flag=1
							#echo "Copying the ${cur_app} success!"
							logger -p local0.info  -t ${MYNAME} "Copying the ${cur_app} success!"
						}
					else
						upname=$(ls ${UPGRADE_TEMP_DIR}/${cur_app} | grep "tar.bz2")
						tar jxf ${UPGRADE_TEMP_DIR}/${cur_app}/${upname} -C ${UPGRADE_UPDATE_DIR} && {
							#echo "Unpack the ${cur_app} success!"
							show_logger info "Uppack the ${cur_app} success!"
							success_flag=1
						}
					fi
					${SYNC}
					${SYNC}
				else
					#echo "checking tar package md5 error!"
					show_logger err "Checking tar package md5 error!"
				fi
				
				# when debug ,disable it
				rm -fR ${UPGRADE_TEMP_DIR}/${cur_app}
			else
				#echo "untar package ${UPGRADE_TEMP_DIR}/${filename} error!"
				show_logger err "untar package ${UPGRADE_TEMP_DIR}/${filename} error!"
			fi
			
		fi

		#for the product flashing the mac
		if [ "${filefirst}" = "${cur_app}" -a "${filelast}" = "db" ];then
			if [ "${cur_app}" = "mac" ];then
				upname=$(ls ${UPGRADE_TEMP_DIR} | grep "^${filefirst}" | grep ".${filelast}$")
				[ -d "${UPDATE_DIR}/mac" ] && rm -rf ${UPDATE_DIR}/mac
				mkdir -p ${UPDATE_DIR}/mac
				[ -d "/data/mac" ] && rm -rf /data/mac
				mkdir -p /data/mac
				cp -rv ${UPGRADE_TEMP_DIR}/${upname}  ${UPDATE_DIR}/mac/${filefirst}.${filelast}
				cp -rv ${UPGRADE_TEMP_DIR}/${upname}  /data/mac/${filefirst}.${filelast} && {
					success_flag=1
					#echo "Copying the ${cur_app} success!"
					show_logger info "Copying the ${cur_app} success!"
				}
				${SYNC}                                                                      
				${SYNC}
			fi
		fi

	done
done
rm -rf $LOCK_FILE
[ "${success_flag}" = "1" ] && {
	#echo "Unpack and verifythe upgrade package success, Rebooting ..."
	show_logger info "Unpack and verifythe upgrade package success!"
	if [ "${REBOOT_FLAG}" = "1" ]; then
		show_logger notice "Rebooting ..."
		reboot
	fi
}||{
	show_logger err "Unpack and verifythe upgrade package failture!"
	exit 1;
}
exit 0
