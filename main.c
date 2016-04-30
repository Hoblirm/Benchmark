#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string>

bool is_big_endian()
{
  union
  {
    int i;
    char c[4];
  } data;

  data.i = 1;
  return (data.c[0] != 1);
}

void print_binary(char* buffer, int size)
{
  for (int i = 0; i < size; ++i)
  {
    for (int z = 128; z > 0; z >>= 1)
    {
      ((buffer[i] & z) == z) ? printf("1") : printf("0");
      if (z == 16)
        printf(" ");
    }
    printf(" | ");
  }
  printf("\n");
}
char buffer[16] = { 0 };

void write(uint64_t v, char* buffer, int byte, int offset)
{
  char remainder = *(((char*) &v) + 0) << (8 - offset);

  v >>= offset;

  if (!is_big_endian())
  {
    v = __builtin_bswap64(v);
  }

  *((uint64_t*) (buffer + byte)) |= v;

  uint64_t mask = 0xFF00000000000000;
  mask = mask << (8 - offset);
  v |= mask;

  *((uint64_t*) (buffer + byte)) &= v;

//  char remainder = *(((char*)&v)+0) << (8-offset);
  *(buffer + byte + 8) |= remainder;

  char cmask = 0xFF;
  cmask = cmask >> offset;
  remainder |= cmask;
  *(buffer + byte + 8) &= remainder;
}

/*
 * This method packs data with a given size_in_bits into a buffer with a
 * specified buffer_offset in bits.  The packing process performs the
 * necessary bitwise operations to ensure only the bits belonging to data
 * are written to buffer. **Note: this method has undefined behavior if the
 * parameter criteria below are not met.
 * @param data - Contents that will be packed into buffer.  It is assumed
 *               to be in Big Endian format, and that all unused bits are
 *               unset. The bitwise operations performed in this method
 *               may alter the contents of data. If data is to be used
 *               after this method, a temporary copy should be passed in.
 * @param size_in_bits - Specifies the bit size of data.  This value must
 *                       be one or greater.
 * @param buffer - Location data is written to.  This method always writes
 *                 to the first byte in buffer.
 * @param buffer_offset - Offset in bits that specifies where data will begin
 *                        in buffer's first byte.  Must be between 0(MSB) and 7(LSB).
 */
void pack(char* data, int size_in_bits, char* buffer, int buffer_offset)
{
  /*
   * We are assuming the data passed in is Big Endian.  If the size_in_bits
   * is not a multiple of 8, a data_offset will be applied.  Note that this
   * is separate from the buffer_offset parameter.
   * Ex. (Assuming size_in_bits of 12).
   *  | 0 0 0 0  1 1 1 1 | 1 1 1 1  1 1 1 1 | = data
   *  | 0 1 2 3  4 5 6 7 | 0 1 2 3  4 5 6 7 |
   *             ^
   *             data_offset
   */
  int data_offset = 7 - ((size_in_bits - 1) % 8);

  /*
   * We need to ensure that only the bits that belong to data are
   * assigned to buffer.  This is achieved by doing a "masked assignment".
   * A masked assignment involves three steps:
   * 1. Perform a bitwise OR between the data and buffer bytes. Remember
   *    it is assumed the unused bits in data are unset.
   * 2. Perform a bitwise OR between data and a mask.  The mask has all
   *    the unused bits set.
   * 3. Perform a bitwise AND between the masked data and buffer bytes.
   *
   * A masked assignment will only need to be done on the head and tail
   * bytes.  The example below shows how the head_mask is derived.
   *
   *  | 1 1 1 0  0 0 0 0 | = head_mask (1 denotes bits not used by data)
   *  | 0 1 2 3  4 5 6 7 |
   *          ^
   *          buffer_offset (bit where data starts)
   *          |<---------| head_mask is shifted left by (8-buffer_offset)
   */
  int head_mask = 0xFF << 8 - buffer_offset;

  /*
   * If the buffer_offset offset specified is to the left of data_offset.
   * Ex.
   *       buffer_offset (bit where data must start in buffer)
   *       v
   * | 0 1 2 3  4 5 6 7 |
   *            ^
   *            data_offset (start bit of data)
   *       |<---| left_shift = data_offset - buffer_offset
   * |----------->| tail_mask is shifted right by
   *
   */
  if (buffer_offset <= data_offset)
  {
    int number_of_bytes = ((size_in_bits - 1) / 8) + 1;
    int left_shift = data_offset - buffer_offset;

    /*
     *  | 1 1 1 1  1 0 0 0 | = data (1 denotes bits used by data)
     *
     *               |<----| left_shift
     *
     *  | 0 0 0 0  0 1 1 1 | = tail_mask (1 denotes bits not used by data)
     *  | 0 1 2 3  4 5 6 7 |
     *
     *  |----------->| tail_mask is shifted right by (8-left_shift)
     */
    int tail_mask = 0xFF >> 8 - left_shift;

    /*
     * Special case of data only being 1 byte.  In this case both
     * the head and tail masks are applied to the same byte.
     */
    if (number_of_bytes == 1)
    {
      //Perform shift for bit offset.
      data[0] <<= left_shift;

      //Perform masked assignment with head and tail mask.
      buffer[0] |= data[0];
      data[0] |= head_mask;
      data[0] |= tail_mask;
      buffer[0] &= data[0];
    }
    else
    {
      /*
       * We will go through each data byte, apply the bit offset by shifting, and then write the byte
       * to buffer.
       */

      //Perform bit offset shift for head byte.
      data[0] <<= left_shift;
      data[0] |= data[1] >> (8 - left_shift); //Get shifted bits from right neighbor

      //Perform masked assignment for head byte.
      buffer[0] |= data[0];
      data[0] |= head_mask;
      buffer[0] &= data[0];

      /*
       * Assign all bytes between head and tail.
       */
      int i;
      for (i = 1; i < number_of_bytes - 1; ++i)
      {
        //Perform bit offset shift
        data[i] <<= left_shift;
        data[i] |= data[i + 1] >> (8 - left_shift); //Get shifted bits from right neighbor

        //No mask, we can perform a normal assignment, hurray!
        buffer[i] = data[i];
      }

      //Perform bit offset shift for tail byte.
      data[i] <<= left_shift;

      //Perform masked assignment for tail byte.
      buffer[i] |= data[i];
      data[i] |= tail_mask;
      buffer[i] &= data[i];
    }
  }
  /*
   * If the buffer_offset offset specified is to the right of data_offset.
   * Ex.
   *                buffer_offset (bit where data must start in buffer)
   *                v
   * | 0 1 2 3  4 5 6 7 |
   *            ^
   *            data_offset (start bit of data)
   *            |-->| right_shift = buffer_offset - data_offset
   */
  else //(data_offset < buffer_offset)
  {
    int number_of_bytes = ((size_in_bits - 1) / 8) + 2;
    int right_shift = buffer_offset - data_offset;

    /*
     *  | 1 1 1 0  0 0 0 0 | = data (1 denotes bits used by data)
     *
     *  |---->| right_shift
     *
     *  | 0 0 0 1  1 1 1 1 | = tail_mask (1 denotes bits not used by data)
     *  | 0 1 2 3  4 5 6 7 |
     *
     *  |---->| tail_mask is shifted right by right_shift
     */
    int tail_mask = 0xFF >> right_shift;

    /*
     * Since a right shift is being performed, each byte must collect shifted
     * bits from its left neighbor.  We want to ensure that the "neighbor" byte
     * isn't written to prior to the bits being collected.  Therefore we go
     * through the bytes in reverse order, starting with the tail byte.
     */

    int i = number_of_bytes - 1; //tail byte index
    //Perform bit offset shift for tail byte.
    data[i] |= data[i - 1] << (8 - right_shift); //Get shifted bits from left neighbor

    //Perform masked assignment for tail byte.
    buffer[i] |= data[i];
    data[i] |= tail_mask;
    buffer[i] &= data[i];
    --i; //Decrement i to move to next byte.

    /*
     * Assign all bytes between head and tail.
     */
    for (; i > 0; --i)
    {
      //Perform bit offset shift
      data[i] >>= right_shift;
      data[i] |= data[i - 1] << (8 - right_shift); //Get shifted bits from left neighbor

      //No mask, we can perform a normal assignment, hurray!
      buffer[i] = data[i];
    }

    //Perform bit offset shift for head byte.
    data[0] >>= right_shift;

    //Perform masked assignment for head byte.
    buffer[0] |= data[0];
    data[0] |= head_mask;
    buffer[0] &= data[0];
  }

}

uint64_t read(char* buffer, int byte, int offset)
{
  return *((uint64_t*) (buffer + byte));
}

int main(int argc, char** argv)
{
  int i = 0;
  union data_t
  {
    uint64_t i;
    char data[9];
  };

  data_t b;
  b.data[8] = '\0';
  b.i = 0x0102030405060708;
  b.i = __builtin_bswap64(b.i);
  print_binary((char*) &b, 9);

  pack((char*) &b, 57, &buffer[0], 1);
  print_binary((char*) &buffer, 9);
  return 0;
}
