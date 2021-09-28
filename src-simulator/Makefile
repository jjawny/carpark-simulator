# Makefile for Manager software
CC = gcc
CFLAGS = -Wall -pedantic -g
LDFLAGS = -lpthread -lrt

TARGET = SIMULATOR

all: $(TARGET)
	echo "Done."

# To create the executable we need the following objects...
$(TARGET): simulator.o sleep.o spawn-cars.o parking.o queue.o handle-entrance.o
	$(CC) -o ../$(TARGET) simulator.o sleep.o spawn-cars.o parking.o queue.o handle-entrance.o $(CFLAGS) $(LDFLAGS)

# To create simulator object file we need the following...
simulator.o: simulator.c sleep.h spawn-cars.h parking.h queue.h handle-entrance.h sim-common.h ../config.h
	$(CC) -c simulator.c $(CFLAGS) $(LDFLAGS)

# To create sleep object file we need the following...
sleep.o: sleep.c sleep.h
	$(CC) -c sleep.c $(CFLAGS) $(LDFLAGS)

# To create spawn cars object file we need the following...
spawn-cars: spawn-cars.c spawn-cars.h sleep.h sim-common.h
	$(CC) -c spawn-cars.c $(CFLAGS) $(LDFLAGS)

# To create parking object file we need the following...
parking.o: parking.c parking.h
	$(CC) -c parking.c $(CFLAGS) $(LDFLAGS)

# To create queue object file we need the following...
queue.o: queue.c queue.h
	$(CC) -c queue.c $(CFLAGS) $(LDFLAGS)

# To create handle entrance object file we need the following...
handle-entrance.o: handle-entrance.c sleep.h parking.h queue.h sim-common.h
	$(CC) -c handle-entrance.c $(CFLAGS) $(LDFLAGS)

clean:
	rm ../$(TARGET) *.o

.PHONY: all clean
