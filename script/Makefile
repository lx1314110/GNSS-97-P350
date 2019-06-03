
.PHONY : install
install :
	if [ -d "$(CMD_DIR)" ]; then \
		if [ -d "./cmd" ]; then \
			cp -r ./cmd/* $(CMD_DIR); \
		fi; \
	fi
	if [ -d "$(INIT_DIR)" ]; then \
		if [ -d "./init" ]; then \
			cp -r ./init/* $(INIT_DIR);\
		fi \
	fi
	if [ -d "$(PROC_DIR)" ]; then \
		[ -e "busybox" ] && cp busybox $(PROC_DIR);\
	fi
