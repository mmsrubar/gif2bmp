#ifndef BITS_H
#define BITS_H

/* write lower nibble to the byte */
#define WRITE_LOW_NIBBLE(byte, nibble) byte = (byte & 0xF0) | (nibble & 0xF);
/* write highest nibble to the byte */
#define WRITE_HIGH_NIBBLE(byte, nibble) byte = (byte & 0x0F) | ((nibble & 0xF) << 4); // write high quartet

#define BYTE_TO_BIN_STR(byte) \
	for (int i = 0; i < 8; i++) \
		(byte & (0x80 >> i)) ? putchar('1') : putchar ('0');

/* The smallest number greater or equal to input which is divisible by 4 */
#define NUM_DIVISIBLE_BY_FOUR(input) ((input + 3) & ~3)

/* Set a specific bit */
#define BIT_SET(byte, position) (byte |= 1 << position)

/* Return n-th power of 2 */
#define POW2(n) (1 << (n))

/* Returns some non-zero value if the bit on the n-th position isn't zero */
#define GET_BIT(b, n) ((n >= 0 && n < 8) ? (b & (1 << n)) : 0)

#endif	// BITS_H
