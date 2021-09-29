# Makefile for MANAGER software
CC = gcc
CFLAGS = -Wall -pedantic -g
LDFLAGS = -lpthread -lrt

TARGET = MANAGER

all: $(TARGET)
	echo "Done."

# To create the executable we need the following objects...
$(TARGET): manager.o plates-hash-table.o manage-entrance.o
	$(CC) -o ../$(TARGET) manager.o plates-hash-table.o manage-entrance.o $(CFLAGS) $(LDFLAGS)

# To create manager object
manager.o: manager.c plates-hash-table.h manage-entrance.h man-common.h ../config.h
	$(CC) -c manager.c $(CFLAGS) $(LDFLAGS)

# To create plates-hash-table object
plates-hash-table.o: plates-hash-table.c plates-hash-table.h
	$(CC) -c plates-hash-table.c $(CFLAGS) $(LDFLAGS)

# To create manage-entrance object
manage-entrace.o: manage-entrance.c manage-entrance.h plates-hash-table.h man-common.h ../config.h
	$(CC) -c manage-entance.c $(CFLAGS) $(LDFLAGS)

clean:
	rm ../$(TARGET) *.o

.PHONY: all clean
