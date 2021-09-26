
#include <time.h>
#include "sleep.h"

void sleep_for(int s, int ns) {
   // for debugging...
   s = 1;
   long int scale = 1; /* slow down time for debugging, 1 = no change */

   struct timespec remaining, requested = {s * scale, ns * scale};
   nanosleep(&requested, &remaining);
}