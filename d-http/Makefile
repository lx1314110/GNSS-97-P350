
TARGET := p350_http

.PHONY : all
all : $(TARGET)

$(TARGET) : mini_httpd.o match.o tdate_parse.o
	$(CC) $(CFLAGS) mini_httpd.o match.o tdate_parse.o -o $(TARGET)

mini_httpd.o:	mini_httpd.c version.h port.h match.h tdate_parse.h mime_encodings.h mime_types.h
	$(CC) $(CFLAGS) -c mini_httpd.c

match.o:	match.c match.h
	$(CC) $(CFLAGS) -c match.c

tdate_parse.o:	tdate_parse.c tdate_parse.h
	$(CC) $(CFLAGS) -c tdate_parse.c

mime_encodings.h: mime_encodings.txt
	rm -f mime_encodings.h
	sed < mime_encodings.txt > mime_encodings.h \
	  -e 's/#.*//' -e 's/[ 	]*$$//' -e '/^$$/d' \
	  -e 's/[ 	][ 	]*/", 0, "/' -e 's/^/{ "/' -e 's/$$/", 0 },/'

mime_types.h: mime_types.txt
	rm -f mime_types.h
	sed < mime_types.txt > mime_types.h \
	  -e 's/#.*//' -e 's/[ 	]*$$//' -e '/^$$/d' \
	  -e 's/[ 	][ 	]*/", 0, "/' -e 's/^/{ "/' -e 's/$$/", 0 },/'



.PHONY : install
install :
	if [ -d "$(PROC_DIR)" ];\
	then\
		cp $(TARGET) $(PROC_DIR);\
		cp p350.conf $(APP_DIR);\
	fi


.PHONY : clean
clean :
	rm -f $(TARGET) *.o *.gch *.gdb *.elf *.elf2flt *.bak

