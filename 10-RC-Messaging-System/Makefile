#
# Makefile - makefile to build the base system
#

# Locations

# Make environment
CC=gcc
CFLAGS=-c -Wall -Wextra 
#CFLAGS += -D_XOPEN_SOURCE=500
CFLAGS += -g 
LIBS= -lpthread
# Suffix rules
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS)  -o $@ $<  
	
# Files
OBJECT_FILES=udpserver.o maintain_thd.o hash.o
OBJECT_FILES1=udpclient.o
OBJECT_FILES2=admin_client.o
# Productions
all : udpserver udpclient admin_client

udpserver : $(OBJECT_FILES)
	$(CC)  $(OBJECT_FILES) -o $@ $(LIBS)

udpclient : $(OBJECT_FILES1)
	$(CC)  $(OBJECT_FILES1) -o $@ $(LIBS)

admin_client : $(OBJECT_FILES2)
	$(CC)  $(OBJECT_FILES2) -o $@ $(LIBS)

clean : 
	rm -f udpserver udpclient admin_client $(OBJECT_FILES) $(OBJECT_FILES1) $(OBJECT_FILES2)

