# Makefile for MANAGER software
CC = gcc
CFLAGS = -Wall -pedantic -g
LDFLAGS = -lpthread -lrt

TARGET = MANAGER

all: $(TARGET)
	echo "Done."

# To create the executable we need the following objects...
$(TARGET): manager.o plates-hash-table.o manage-entrance.o manage-exit.o manage-gate.o display-status.o
	$(CC) -o ../$(TARGET) manager.o plates-hash-table.o manage-entrance.o manage-exit.o manage-gate.o display-status.o $(CFLAGS) $(LDFLAGS)

# To create manager object
manager.o: manager.c plates-hash-table.h manage-entrance.h manage-exit.h manage-gate.h display-status.h man-common.h ../config.h
	$(CC) -c manager.c $(CFLAGS) $(LDFLAGS)

# To create plates-hash-table object
plates-hash-table.o: plates-hash-table.c plates-hash-table.h
	$(CC) -c plates-hash-table.c $(CFLAGS) $(LDFLAGS)

# To create manage-entrance object
manage-entrance.o: manage-entrance.c manage-entrance.h plates-hash-table.h man-common.h
	$(CC) -c manage-entrance.c $(CFLAGS) $(LDFLAGS)

# To create manage-exit object
manage-exit.o: manage-exit.c manage-exit.h plates-hash-table.h man-common.h
	$(CC) -c manage-exit.c $(CFLAGS) $(LDFLAGS)

# To create manage-gate object
manage-gate.o: manage-gate.c manage-gate.h man-common.h
	$(CC) -c manage-gate.c $(CFLAGS) $(LDFLAGS)

# To create display-status object
display-status.o: display-status.c display-status.h man-common.h
	$(CC) -c display-status.c $(CFLAGS) $(LDFLAGS)

clean:
	rm ../$(TARGET) *.o

.PHONY: all clean
