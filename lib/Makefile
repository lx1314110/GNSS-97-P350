
LOCAL_OBJS := $(patsubst %.c, %.o, $(wildcard *.c))


.PHONY : all
all : $(STATIC_LIB) $(DYNAMIC_LIB)

$(STATIC_LIB) : $(LOCAL_OBJS)
	$(AR) -rs $@ $(LOCAL_OBJS)

$(DYNAMIC_LIB) : $(LOCAL_OBJS)
	$(CC) -shared -fpic -o $@ $(LOCAL_OBJS)


%.o : %.c
	$(CC) -c $(CFLAGS) -fpic -I$(LIB_INC_DIR) -I$(ALLOC_INC_DIR) $< -o $@


.PHONY : install
install :


.PHONY : clean
clean :
	rm -f *.a *.so *.o *.gch *.gdb *.elf *.elf2flt *.bak
