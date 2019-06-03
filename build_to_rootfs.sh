#!/bin/bash
set -e

ROOTFS_P350_USR=/hdb8/projects/datang/p350/rootfs-p350/rootfs/usr/p350
ROOTFS_P350_APP=${ROOTFS_P350_USR}/app

[ -d app ] || {
	echo "app not exist, please first compile the project."
	exit 1
}
echo "Copy the app ..."
sudo rm -rf ${ROOTFS_P350_APP}
sudo cp -rv app  ${ROOTFS_P350_USR}

echo "Copy the cmd ..."
sudo rm -rf ${ROOTFS_P350_USR}/../../bin/p350*
##old, we arenot need so do
#sudo cp -rv ${ROOTFS_P350_APP}/cmd/*  ${ROOTFS_P350_USR}/../../bin

echo "Copy other tools ..."
sudo cp -rv d-eprom/eprom  ${ROOTFS_P350_USR}/../../root/

echo "success!"
