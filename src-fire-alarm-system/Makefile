# ===================MAKEFILE FOR FIRE ALARM SYSTEM===================
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -g
LDFLAGS = -lpthread -lrt

TARGET = FIRE-ALARM-SYSTEM

all: $(TARGET)
	echo "Done."

# To create the executable we need the following objects...
$(TARGET): fire-alarm.o monitor-temp.o fire-evac.o fire-gate.o fire-common.o
	$(CC) -o ../$(TARGET) fire-alarm.o monitor-temp.o fire-evac.o fire-gate.o fire-common.o $(CFLAGS) $(LDFLAGS)

# To create MAIN fire-alarm object
fire-alarm.o: fire-alarm.c monitor-temp.h fire-evac.h fire-gate.h fire-common.h ../config.h
	$(CC) -c fire-alarm.c $(CFLAGS) $(LDFLAGS)

# To create monitor-temp object
monitor-temp.o: monitor-temp.c monitor-temp.h fire-common.h
	$(CC) -c monitor-temp.c $(CFLAGS) $(LDFLAGS)

# To create fire-evac object
fire-evac.o: fire-evac.c fire-evac.h fire-common.h
	$(CC) -c fire-evac.c $(CFLAGS) $(LDFLAGS)

# To create fire-gate object
fire-gate.o: fire-gate.c fire-gate.h fire-common.h
	$(CC) -c fire-gate.c $(CFLAGS) $(LDFLAGS)

# To create fire-common object
fire-common.o: fire-common.c fire-common.h
	$(CC) -c fire-common.c $(CFLAGS) $(LDFLAGS)

clean:
	rm ../$(TARGET) *.o

.PHONY: all clean
