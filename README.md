# carpark-simulator
todo


# Simulator command
gcc -o ../SIMULATOR simulator.c parking.c queue.c sleep.c spawn-cars.c handle-entrance.c car-lifecycle.c -Wall -lpthread -lrt

# Manager command
gcc -o ../MANAGER manager.c plates-hash-table.c manage-entrance.c -Wall -lpthread -lrt
