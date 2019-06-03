#!/bin/bash
set -e
#set -x
pdir="$1"
md5_list_file="$2"

[ -d "$pdir" ] || {
	
	echo usage: $0 dir
	exit 1
}
[ -n "$md5_list_file" ] || md5_list_file=list.md5

cd $pdir
echo "creating all file md5 in the \"$pdir\"..."
#find -type f | xargs md5sum > $md5_list_file
find . \( -path './.git' -o  -name "${md5_list_file}" \) -prune -o -type f -print | xargs md5sum > $md5_list_file

cd -
