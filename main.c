#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <timer.h>

int main(int argc, char** argv) {
   Timer t;
   t.start();
   usleep(3000000);
   t.stop();
   printf("Timer recorded %lf ms\n",t.usAvg()/1000);
   return 0;
}

