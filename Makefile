#for compile
DEBUG ?= 0

#for install
SNMP ?= 0
DB ?= 0
FPGA ?= 0
WEB ?= 0
INIT ?= 0
export AR := arm-linux-ar
export CC := arm-linux-gcc
export STRIP := arm-linux-strip
#export CC=arm-linux-gnueabi-gcc-4.7 
#export AR=arm-linux-gnueabi-ar

#gdb
ifeq ($(DEBUG),1)
export CFLAGS := -Wall -pipe -g
else
export CFLAGS := -Wall -Os -pipe
endif

export STATIC_LIB := libp350.a
export DYNAMIC_LIB := libp350.so

export LIB_INC_DIR := $(shell pwd)/lib/include
export LIB_SRC_DIR := $(shell pwd)/lib
export LIB_SO_DIR	:= $(shell pwd)/so

export ALLOC_INC_DIR := $(shell pwd)/alloc/include
export ALLOC_SRC_DIR := $(shell pwd)/alloc

export APP_DIR := $(shell pwd)/app
export DB_DIR := $(shell pwd)/db
#SUPPORT TQ335X AND TQ2416 FPGA
export FPGA_DIR := $(shell pwd)/fpga
#export WEB_DIR := $(shell pwd)/www
export WEB_DIR := $(shell pwd)/../web/www
export WWW_INDEX_HTML=${WEB_DIR}/index.html
export SNMP_DIR := $(shell pwd)/../snmp_mib_src/build/snmp
export SNMP_CONF_FILE := ${SNMP_DIR}/snmpd.conf

#INSTALL DIR
export PROC_DIR := $(APP_DIR)/proc
export CMD_DIR := $(APP_DIR)/cmd
export INIT_DIR := $(APP_DIR)/init
export SW_VER := $(shell cat d-manager/dmgr_version.h | grep "\#define" |grep "SW_VER" | awk '{print $3}' | cut -d \" -f2 | cut -d V -f2 | cut -d v -f2)

.PHONY : all
all :
	$(MAKE) -C lib
	$(MAKE) -C alloc
	$(MAKE) -C d-manager
	$(MAKE) -C d-input
	$(MAKE) -C d-output
	$(MAKE) -C d-alarm
	$(MAKE) -C d-ntp
	$(MAKE) -C d-http
	$(MAKE) -C d-keylcd
	$(MAKE) -C d-61850
	$(MAKE) -C d-eprom
	$(MAKE) -C d-wg
	$(MAKE) -C cmd/c-sys
	$(MAKE) -C cmd/c-input
	$(MAKE) -C cmd/c-output
	$(MAKE) -C cmd/c-alarm
	$(MAKE) -C cmd/c-log
	$(MAKE) -C cmd/c-dbg
	$(MAKE) -C test
.PHONY : install
install :
	if [ ! -d "$(APP_DIR)" ];\
	then\
		mkdir $(APP_DIR);\
	fi
	
	if [ ! -d "$(CMD_DIR)" ];\
	then\
		mkdir $(CMD_DIR);\
	fi
	
	
	if [ ! -d "$(PROC_DIR)" ];\
	then\
		mkdir $(PROC_DIR);\
	fi
	
	if [ "$(INIT)" = "1" ]; then \
		mkdir -p ${INIT_DIR}; \
	else	\
		rm -rf ${INIT_DIR};	\
	fi
	
	$(MAKE) install -C lib
	$(MAKE) install -C alloc
	$(MAKE) install -C d-manager
	$(MAKE) install -C d-input
	$(MAKE) install -C d-output
	$(MAKE) install -C d-alarm
	$(MAKE) install -C d-ntp
	$(MAKE) install -C d-http
	$(MAKE) install -C d-keylcd
	$(MAKE) install -C d-61850
	$(MAKE) install -C d-wg
	$(MAKE) install -C cmd/c-sys
	$(MAKE) install -C cmd/c-input
	$(MAKE) install -C cmd/c-output
	$(MAKE) install -C cmd/c-alarm
	$(MAKE) install -C cmd/c-log
	$(MAKE) install -C cmd/c-dbg
	$(MAKE) install -C test/fpga_addr_print
	$(MAKE) install -C script
	sleep 2; \
	sync; \
	sync; \
	if [ "$(DB)" = "1" ]; then \
		mkdir -p ${APP_DIR}/db; \
		cp -rv ${DB_DIR}/p350.db  ${APP_DIR}/db; \
	else	\
		rm -rf ${APP_DIR}/db;	\
	fi; \
	if [ "$(FPGA)" = "1" ]; then \
		mkdir -p ${APP_DIR}/fpga; \
		if [ -f ${FPGA_DIR}/fpga-*tq2416.rbf ]; then cp -rv ${FPGA_DIR}/fpga-*tq2416.rbf  ${APP_DIR}/fpga/fpga_tq2416.rbf; fi; \
		if [ -f ${FPGA_DIR}/fpga-*tq335x.rbf ]; then cp -rv ${FPGA_DIR}/fpga-*tq335x.rbf  ${APP_DIR}/fpga/fpga_tq335x.rbf; fi; \
	else	\
		rm -rf ${APP_DIR}/fpga;	\
	fi; \
		\
	if [ "$(WEB)" = "1" ]; then \
		mkdir -p ${APP_DIR}/www; \
		[ -e "${WWW_INDEX_HTML}" ] && { \
			find ${WEB_DIR} -maxdepth 1 -mindepth 1  \( -name 'www_en' -o -name 'www_cn' -o -name '.git*' -o  -name '*.md5' \) -prune -o  -print | xargs -i cp -rv {} ${APP_DIR}/www; \
			sync; \
			sync; \
		} || { \
			echo "No found the ${WWW_INDEX_HTML} file, please check whether exist the ${WEB_DIR} dir!"; \
			rm -rf ${APP_DIR}/www;	\
			exit 1; \
		}	\
	else	\
		rm -rf ${APP_DIR}/www;	\
	fi; \
		\
	if [ "$(SNMP)" = "1" ]; then \
		mkdir -p ${APP_DIR}/snmp; \
		[ -e "${SNMP_CONF_FILE}" ] && { \
			find ${SNMP_DIR} -maxdepth 1 -mindepth 1  \( -name '.git*' -o  -name '*.md5' \) -prune -o  -print | xargs -i cp -rv {} ${APP_DIR}/snmp; \
			sync; \
			sync; \
		} || { \
			echo "No found the ${SNMP_CONF_FILE} file, please check whether exist the ${SNMP_DIR} dir!"; \
			rm -rf ${APP_DIR}/snmp;	\
			exit 1; \
		}	\
	else	\
		rm -rf ${APP_DIR}/snmp;	\
	fi; \
	[ "$(DEBUG)" != "1" ] && { \
			find ${APP_DIR}/proc  -type f -print | xargs -i $(STRIP) {} > /dev/null 2>&1; find ${APP_DIR}/cmd -type f -print | xargs -i $(STRIP) {} >/dev/null 2>&1; echo ""; \
			find ${APP_DIR} \( -path '${APP_DIR}/www' -o -name "*.ko" \)  -prune -o -type f -print | xargs -i $(STRIP) {}; echo "";	\
	};  \
	sync && \
	./script/create_md5_list.sh app  list.md5 && \
	sync && sleep 1 && \
	tar jcf app-v$(SW_VER).tar.bz2 app && \
	./upgrade_web.sh
	@echo SW Version: v${SW_VER}

.PHONY : clean
clean :
	$(MAKE) clean -C lib
	$(MAKE) clean -C alloc
	$(MAKE) clean -C d-manager
	$(MAKE) clean -C d-input
	$(MAKE) clean -C d-output
	$(MAKE) clean -C d-alarm
	$(MAKE) clean -C d-ntp
	$(MAKE) clean -C d-http
	$(MAKE) clean -C d-keylcd
	$(MAKE) clean -C d-61850
	$(MAKE) clean -C d-eprom
	$(MAKE) clean -C d-wg
	$(MAKE) clean -C cmd/c-sys
	$(MAKE) clean -C cmd/c-input
	$(MAKE) clean -C cmd/c-output
	$(MAKE) clean -C cmd/c-alarm
	$(MAKE) clean -C cmd/c-log
	$(MAKE) clean -C cmd/c-dbg
	$(MAKE) clean -C test
	
	rm -fR $(APP_DIR)  *.bak *.out *.bz2
