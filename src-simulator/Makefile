# Makefile for MANAGER software
CC = gcc
CFLAGS = -Wall -pedantic -g
LDFLAGS = -lpthread -lrt

TARGET = SIMULATOR

all: $(TARGET)
	echo "Done."

# To create the EXECUTABLE we need the following objects...
$(TARGET): simulator.o sleep.o spawn-cars.o parking.o queue.o handle-entrance.o car-lifecycle.o
	$(CC) -o ../$(TARGET) simulator.o sleep.o spawn-cars.o parking.o queue.o handle-entrance.o car-lifecycle.o $(CFLAGS) $(LDFLAGS)

# To create simulator object
simulator.o: simulator.c spawn-cars.h parking.h queue.h handle-entrance.h sim-common.h ../config.h
	$(CC) -c simulator.c $(CFLAGS) $(LDFLAGS)

# To create sleep object
sleep.o: sleep.c sleep.h
	$(CC) -c sleep.c $(CFLAGS) $(LDFLAGS)

# To create spawn-cars object
spawn-cars: spawn-cars.c spawn-cars.h sleep.h queue.h sim-common.h ../config.h
	$(CC) -c spawn-cars.c $(CFLAGS) $(LDFLAGS)

# To create parking object
parking.o: parking.c parking.h
	$(CC) -c parking.c $(CFLAGS) $(LDFLAGS)

# To create queue object
queue.o: queue.c queue.h ../config.h
	$(CC) -c queue.c $(CFLAGS) $(LDFLAGS)

# To create handle entrance object
handle-entrance.o: handle-entrance.c handle-entrance.h sleep.h parking.h queue.h car-lifecycle.h sim-common.h
	$(CC) -c handle-entrance.c $(CFLAGS) $(LDFLAGS)

# To create car lifecycle object
car-lifecycle.o: car-lifecycle.c car-lifecycle.h queue.h
	$(CC) -c car-lifecycle.c $(CFLAGS) $(LDFLAGS)

clean:
	rm ../$(TARGET) *.o

.PHONY: all clean
