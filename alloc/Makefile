
LOCAL_OBJS := $(patsubst %.c, %.o, $(wildcard *.c))


.PHONY : all
all : $(LOCAL_OBJS)

%.o : %.c
	$(CC) -c $(CFLAGS) -I$(LIB_INC_DIR) -I$(ALLOC_INC_DIR) $< -o $@


.PHONY : install
install : 


.PHONY : clean
clean : 
	rm -f *.o *.gch *.gdb *.elf *.elf2flt *.bak
