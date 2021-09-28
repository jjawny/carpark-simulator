# Makefile for Manager software
CC = gcc
CFLAGS = -Wall -pedantic -g
LDFLAGS = -lpthread -lrt

TARGET = MANAGER

all: $(TARGET)
	echo "Done."

# To create the executable we need the following objects...
$(TARGET): manager.o plates-hash-table.o manage-entrance.o
	$(CC) -o ../$(TARGET) manager.o plates-hash-table.o manage-entrance.o $(CFLAGS) $(LDFLAGS)

# To create manager object file we need the following...
manager.o: manager.c parking-types.h plates-hash-table.h manage-entrance.h man-common.h ../config.h
	$(CC) -c manager.c $(CFLAGS) $(LDFLAGS)

# To create manager object file we need the following...
plates-hash-table.o: plates-hash-table.c plates-hash-table.h
	$(CC) -c plates-hash-table.c $(CFLAGS) $(LDFLAGS)

# To create manager object file we need the following...
manage-entrace.o: manage-entrance.c manage-entrance.h parking-types.h plates-hash-table.h man-common.h ../config.h
	$(CC) -c manage-entance.c $(CFLAGS) $(LDFLAGS)

clean:
	rm ../$(TARGET) *.o

.PHONY: all clean
