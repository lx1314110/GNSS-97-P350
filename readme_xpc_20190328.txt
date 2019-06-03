1.目录说明：
alloc目录
-alloc.c	存放一些表和办卡命名
-incude目录
--addr.h	存放与FPGA交互用的地址
--alloc.h	存放一些宏和结构体、公用体

cmd目录 存放P350串口命令

d-manager目录	存放用来启动其他进程和ip1725配置、升级、重启等内容

d-61850目录 存放61850模块相关的程序

d-alarm目录 存放把输入、输出告警搜集起来上传数据库和屏蔽告警

d-http目录	存放mini-httpd服务器运行程序

d-input目录	存放卫星信息解析和输入告警、输入配置、选源等

d-output目录	存放获取系统时间和输出时区、输出配置等

d-wg目录	存放与集中网管通讯相关信息

d-keylcd目录	存放液晶屏显示信息

d-eprom目录		存放用来读取eprom寄存器工具

so目录	存放sqlite3库文件

script目录	存放一些脚本和文件系统的一些升级文件
app目录   编译安装时。生成的临时文件
upgrade_web 编译安装后，生成的最终web升级包
test目录	未用

2.编译说明：
（1）导出编译环境
. /hdb8/projects/datang/p350/tools/env.sh

[注：
env.sh的内容：（/hdb8/projects/datang/p350/tools/usr/local/arm/4.3.2/bin为你的编译工具所在的位置。）
PATH=/hdb8/projects/datang/p350/tools/usr/local/arm/4.3.2/bin:$PATH

ls /hdb8/projects/datang/p350/tools/usr/local/arm/4.3.2/bin
arm-linux-addr2line  arm-linux-gcc        arm-linux-nm       arm-linux-strings                 arm-none-linux-gnueabi-cpp        arm-none-linux-gnueabi-gprof    arm-none-linux-gnueabi-size
arm-linux-ar         arm-linux-gcc-4.3.2  arm-linux-objcopy  arm-linux-strip                   arm-none-linux-gnueabi-g++        arm-none-linux-gnueabi-ld       arm-none-linux-gnueabi-sprite
arm-linux-as         arm-linux-gcov       arm-linux-objdump  arm-none-linux-gnueabi-addr2line  arm-none-linux-gnueabi-gcc        arm-none-linux-gnueabi-nm       arm-none-linux-gnueabi-strings
arm-linux-c++        arm-linux-gdb        arm-linux-ranlib   arm-none-linux-gnueabi-ar         arm-none-linux-gnueabi-gcc-4.3.2  arm-none-linux-gnueabi-objcopy  arm-none-linux-gnueabi-strip
arm-linux-c++filt    arm-linux-gdbtui     arm-linux-readelf  arm-none-linux-gnueabi-as         arm-none-linux-gnueabi-gcov       arm-none-linux-gnueabi-objdump
arm-linux-cpp        arm-linux-gprof      arm-linux-size     arm-none-linux-gnueabi-c++        arm-none-linux-gnueabi-gdb        arm-none-linux-gnueabi-ranlib
arm-linux-g++        arm-linux-ld         arm-linux-sprite   arm-none-linux-gnueabi-c++filt    arm-none-linux-gnueabi-gdbtui     arm-none-linux-gnueabi-readelf
]

(2).编译：
make clean #可选
make

(3).生成web升级文件，此步骤可跳过，直接用./build.sh就搞定了
a.生成纯app版本
make install
b.生成添加fpga和数据库db的版本
DB=1 FPGA=1 make install

生成的包在upgrade_web目录下

(4).如果需要把新生成的app打包到文件系统root.bin，请按以下步骤进行：
如果我的文件系统在目录/hdb8/projects/datang/p350/rootfs-p350/rootfs下
P350_ROOTFS=/hdb8/projects/datang/p350/rootfs-p350/rootfs
sudo rm -rf ${P350_ROOTFS}/usr/p350/app/*
sudo cp -rv app/* ${P350_ROOTFS}/usr/p350/app/
此步骤可是可选，不生成全新底层文件系统的话，根本用不着。看你需求了。

注：
(2),(3)步骤 可用脚本./build.sh代替
如果要生成debug升级包，使用 DEBUG=1 ./build.sh 命令，建议tq2416不要用，因为debug包体积大，tq2416存不下，所以升级不上去。tq335x没有问题的。

为了支持恢复出厂设置功能，新的文件系统打包必须在app文件夹下中包含www，fpga和db,并整个复制到rootfs中，详情参照p350restore脚本内容


----
关于升级包的解释说明：
app-v03.06_all_fpga_web_db_init.tar.bz2   ---app、fpga、www、数据库db和系统一些关键初始化文件的合成版本【如果未来需要把p350_v2版本升级到广电设备上，请用该版本。因为添加了init相关文件的升级，很重要】
app-v03.06_all_snmp_fpga_web_db_init.tar.bz2  -----在app-v03.06_all_fpga_web_db_init.tar.bz2的基础上添加了snmp升级；【最全功能的版本，如果广电需要snmp可用改版本】
app-v03.06_fpga_db.tar.bz2  ---仅包含app fpga db， 无系统初始化文件
app-v03.06.tar.bz2   ---仅包含app，无系统初始化文件
app-v03.06_all_fpga_web_db.tar.bz2   ---仅包含app fpga www db， 无系统初始化文件
app-v03.06_all_snmp_fpga_web_db.tar.bz2    ---仅包含app snmp fpga web db，无系统初始化文件
app-v03.06_fpga.tar.bz2 ---仅包含app，fpga， 无系统初始化文件

---关于GDB DEBUG的说明：
如果应用程序需要进行gdb调试，需要添加DEBUG=1；
如：
编译：make clean; DEBUG=1 make
打包：DEBUG=1 make install
也可以使用DEBUG=1 ./build.sh，生成DEBUG版本的安装升级包
程序升级安装到设备端后，利用主机工具arm-linux-gdb和目标板程序gdbserver配合调试

----关于Makefile中，第158和159行arm-linux-strip的作用：
[ "$(DEBUG)" != "1"  ] && find ${APP_DIR} \( -path '${APP_DIR}/www' -o -name "*.ko" \)  -prune -o -type f -print | xargs -i $(STRIP) {}; echo ""
strip会去掉程序中的符号说明，优点是可以很大程序的减少程序体积，缺点是程序没办法进行gdb调试。
（1）am335x中因为flash分区比较大，所以没必要减少程序体积，所以最好就是删除这两行的操作。甚至都可以直接使用-g参数的debug编译版本
（2）tq2416的flash空间比较小，如果广电又需要snmp等功能时，由于系统空间可能不足，这里我们就需要节约空间，减少程序体积，所以这两行的strip操作就很重要。可以大大缩小程序安装包的大小。
具体系统空间剩余多少，可以使用df -h命令或者直接在网页系统信息页面查看存储空间信息。请一定关注根目录/的使用情况：
如：
am335x:
Filesystem                Size      Used Available Use% Mounted on
ubi0:rootfs             155.8M     52.3M    103.5M  34% /
tq2416：
Filesystem                Size      Used Available Use% Mounted on
ubi0:rootfs              38.5M     18.7M     19.8M  49% /
可以看到tq2416空间存在不足，升级文件太大，可能导致失败。
