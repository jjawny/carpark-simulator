# ===================MAKEFILE FOR SIMULATOR===================
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g
LDFLAGS = -lpthread -lrt

TARGET = SIMULATOR

all: $(TARGET)
	echo "Done."

# To create the EXECUTABLE we need the following objects...
$(TARGET): simulator.o sleep.o spawn-cars.o parking.o queue.o simulate-entrance.o car-lifecycle.o simulate-exit.o simulate-temp.o
	$(CC) -o ../$(TARGET) simulator.o sleep.o spawn-cars.o parking.o queue.o simulate-entrance.o car-lifecycle.o simulate-exit.o simulate-temp.o $(CFLAGS) $(LDFLAGS)

# To create MAIN simulator object
simulator.o: simulator.c spawn-cars.h parking.h queue.h simulate-entrance.h simulate-exit.h simulate-temp.h sim-common.h ../config.h
	$(CC) -c simulator.c $(CFLAGS) $(LDFLAGS)

# To create sleep object
sleep.o: sleep.c sleep.h sim-common.h
	$(CC) -c sleep.c $(CFLAGS) $(LDFLAGS)

# To create spawn-cars object
spawn-cars: spawn-cars.c spawn-cars.h sleep.h queue.h sim-common.h
	$(CC) -c spawn-cars.c $(CFLAGS) $(LDFLAGS)

# To create parking object
parking.o: parking.c parking.h
	$(CC) -c parking.c $(CFLAGS) $(LDFLAGS)

# To create queue object
queue.o: queue.c queue.h
	$(CC) -c queue.c $(CFLAGS) $(LDFLAGS)

# To create simulate entrance object
simulate-entrance.o: simulate-entrance.c simulate-entrance.h sleep.h parking.h queue.h car-lifecycle.h sim-common.h
	$(CC) -c simulate-entrance.c $(CFLAGS) $(LDFLAGS)

# To create car lifecycle object
car-lifecycle.o: car-lifecycle.c car-lifecycle.h sleep.h queue.h parking.h sim-common.h
	$(CC) -c car-lifecycle.c $(CFLAGS) $(LDFLAGS)

# To create simulate exit object
simulate-exit.o: simulate-exit.c simulate-exit.h sleep.h parking.h queue.h sim-common.h
	$(CC) -c simulate-exit.c $(CFLAGS) $(LDFLAGS)

# To create simulate temp object
simulate-temp.o: simulate-temp.c simulate-temp.h sleep.h parking.h sim-common.h
	$(CC) -c simulate-temp.c $(CFLAGS) $(LDFLAGS)

clean:
	rm ../$(TARGET) *.o

.PHONY: all clean
