
TARGET := fpga_addr

COMMON_OBJS := lib_dbg.o lib_fpga.o
ALLOC_OBJS := alloc.o
LOCAL_OBJS := $(patsubst %.c, %.o, $(wildcard *.c))


.PHONY : all
all : $(TARGET)

$(TARGET) : $(foreach var, $(COMMON_OBJS), $(LIB_SRC_DIR)/$(var)) \
						$(foreach var, $(ALLOC_OBJS), $(ALLOC_SRC_DIR)/$(var)) \
						$(LOCAL_OBJS)
	$(CC) $(CFLAGS) -o $@ $+ -lpthread


%.o : %.c
	$(CC) -c $(CFLAGS) -I$(LIB_INC_DIR) -I$(ALLOC_INC_DIR) $< -o $@


.PHONY : install
install :
	if [ -d "$(PROC_DIR)" ];\
	then\
		cp $(TARGET) $(PROC_DIR);\
	fi


.PHONY : clean
clean :
	rm -f $(TARGET) *.o *.gch *.gdb *.elf *.elf2flt *.bak
