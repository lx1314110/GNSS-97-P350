
TARGET := p350_61850

COMMON_OBJS := lib_dbg.o lib_fpga.o lib_shm.o lib_sema.o lib_msgQ.o lib_sqlite.o 
ALLOC_OBJS := alloc.o common.o
LOCAL_OBJS := $(patsubst %.c, %.o, $(wildcard *.c))


.PHONY : all
all : $(TARGET)

$(TARGET) : $(foreach var, $(COMMON_OBJS), $(LIB_SRC_DIR)/$(var)) \
						$(foreach var, $(ALLOC_OBJS), $(ALLOC_SRC_DIR)/$(var)) \
						$(LOCAL_OBJS)
	$(CC) $(CFLAGS) -o $@ $+ -L$(LIB_SO_DIR) -lsqlite3 -lpthread


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
