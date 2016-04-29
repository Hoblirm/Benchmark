#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string>

bool is_big_endian()
{
  union {
    int i;
    char c[4];
  } data;

  data.i = 1;
  return (data.c[0]!=1);
}

void print_binary(char* buffer,int size)
{
for (int i=0;i<size;++i)
{
  for (int z=128;z>0;z>>=1)
  {
      ((buffer[i] & z) == z) ? printf("1") : printf("0");
      if (z==16) printf(" ");
  }
  printf(" | ");
}
printf("\n");
}
char data[9] = {0};

void write(uint64_t v, char* buffer, int byte, int offset)
{
  char remainder = *(((char*)&v)+0) << (8-offset);

  v >>= offset;

  if (!is_big_endian())
  {
    v = __builtin_bswap64(v);
  }

  *((uint64_t*)(buffer + byte)) |= v;

  uint64_t mask = 0xFF00000000000000;
  mask = mask << (8-offset);
  v |= mask;

  *((uint64_t*)(buffer + byte)) &= v;

//  char remainder = *(((char*)&v)+0) << (8-offset);
  *(buffer + byte + 8) |= remainder;

  char cmask = 0xFF;
  cmask = cmask >> offset;
  remainder |= cmask;
  *(buffer + byte + 8) &= remainder;
}

uint64_t read(char* buffer, int byte, int offset)
{
  return *((uint64_t*)(buffer + byte));
}


int main(int argc, char** argv) {
   int i=0;
   uint64_t b = 0x0102030405060708;
   uint64_t s = __builtin_bswap64(b);
   print_binary((char*)&s,8);
   write(b,data,i,7);

   print_binary(data, sizeof(data));
   b = read(data,i,0);

   s = __builtin_bswap64(b);
   print_binary((char*)&s,8);
   
   return 0;
}
