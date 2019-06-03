#!/bin/bash

#set -x
ALL_APP="app www fpga db mac snmp"
tmp=upgrade_web
CUR_DIR=$(pwd -P)


for filename in `ls`
do
	filefirst=${filename%%-*}
	filelast=${filename##*.}
	for cur_app in ${ALL_APP}; do
		#if [ "${filefirst}" = "${cur_app}" ] && [ "${filelast}" = "bz2" -o "${filelast}" = "rbf" ];then
		if [ "${filefirst}" = "${cur_app}" ] && [ "${filelast}" = "bz2" ];then
			rm -rf ${tmp}/${cur_app}
			mkdir -p ${tmp}/${cur_app}
			cp -rv $filename ./${tmp}/${cur_app} && \
			md5=`md5sum ${tmp}/${cur_app}/${filename} | awk -F " " '{print $1}'`
			echo $md5 > ./${tmp}/${cur_app}/md5.txt && sync
			cd ${tmp}
			if [ "${cur_app}" = "fpga" ];then
				fpganame=${filename%.*}".tar.bz2"
				rm -rf $fpganame
				tar jcvf $fpganame fpga
			else
				#need sleep
				sync; sleep 3; tar jcvf $filename ${cur_app}
			fi
			cd -; rm -rf ${tmp}/${cur_app}; [ "${filelast}" = "bz2" ] && {
				rm -rf $filename
			}
		fi
	done
done
