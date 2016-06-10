#include <stdio.h>
#include <sys/types.h>
#include <math.h>
#include <stdbool.h>

#define RGBQUADLEN1	8		// color table is 8B long for 1 bit per pixel (2x4B)
#define RGBQUADLEN4	64
#define RGBQUADLEN8	1024
#define RGBQUADLEN24 0	// no color table is used for 24b per pixel

#define get_rgbquad_len(bpp) ( (bpp == 1 ? RGBQUADLEN1 : (bpp == 4 ? RGBQUADLEN4 : (bpp == 8 ? RGBQUADLEN8 : RGBQUADLEN24))) )

#define compression_type_str(val)  ( (val == 0) ? "RGB" : ((val == 1) ? "RLE8" : "RLE4"))

int func(int8_t byteFlag, int whichBit)
{
    if (whichBit >= 0 && whichBit < 8)
        return (byteFlag & (1<<whichBit));
    else
        return 0;
}

/* Print byte as a string of single bits */
void byte_bin_str(u_int8_t byte)
{
	u_int8_t mask = 0x80;

	for (int i = 0; i < 8; i++)
		if (byte & (mask >> i)) putchar('1'); else putchar ('0');
}

/* Read 2 bytes from the current possition from the file f */
u_int16_t read_word(FILE *f)
{
	u_int8_t byte = fgetc(f);
	u_int8_t byte2 = fgetc(f);
	return 256 * (byte2) + byte;	// little endian ad01 = ad + 256*01
}

int main(int argc, char *argv[])
{
	FILE *f = fopen(argv[1], "rb");

	// BITMAPFILEHEADER
	u_int8_t bfType[3];
	u_int32_t bfSize;		// file size
	u_int16_t bfReserved1;
	u_int16_t bfReserved2;
	u_int32_t bfOffBits;

	fgets(bfType, 3, f);
	fread(&bfSize, sizeof(u_int32_t), 1, f);
	fread(&bfReserved1, sizeof(u_int16_t), 1, f);
	fread(&bfReserved2, sizeof(u_int16_t), 1, f);
	fread(&bfOffBits, sizeof(u_int32_t), 1, f);

	printf("File Header:\n");
	printf("|- type: %s\n", bfType);
	printf("|- total size: %d\n", bfSize);
	printf("|- reserved1: %d\n", bfReserved1);
	printf("|- reserved2: %d\n", bfReserved2);
	printf("|- off bits: %d = 0x%x(data starts at this byte\n", bfOffBits, bfOffBits);

	// BITMAPINFOHEADER
	u_int32_t biSize;
	u_int32_t biWidth;
	u_int32_t biHeight;
	u_int16_t biPlanes;			// always 1
	u_int16_t biBitCount;	// bits per pixel (1,4,8,24)
	u_int32_t biCompression;	// 0 RGB, 1 RLE8, 2 RLE4
	u_int32_t biSizeImage;
	u_int32_t biXPelsPerMeter;
	u_int32_t biYPelsPerMeter;
	u_int32_t biCirUsed;
	u_int32_t biClrImportant;

	fread(&biSize, sizeof(u_int32_t), 1, f);
	fread(&biWidth, sizeof(u_int32_t), 1, f);
	fread(&biHeight, sizeof(u_int32_t), 1, f);
	fread(&biPlanes, sizeof(u_int16_t), 1, f);
	fread(&biBitCount, sizeof(u_int16_t), 1, f);
	fread(&biCompression, sizeof(u_int32_t), 1, f);
	fread(&biSizeImage, sizeof(u_int32_t), 1, f);
	fread(&biXPelsPerMeter, sizeof(u_int32_t), 1, f);
	fread(&biYPelsPerMeter, sizeof(u_int32_t), 1, f);
	fread(&biCirUsed, sizeof(u_int32_t), 1, f);
	fread(&biClrImportant, sizeof(u_int32_t), 1, f);

	printf("Info Header:\n");
	printf("|- info header size: %d\n", biSize);
	printf("|- width: %d\n", biWidth);
	printf("|- height: %d\n", biHeight);
	printf("|- planes: %d\n", biPlanes);
	printf("|- bits per pixel: %d\n", biBitCount);
	printf("|- compression: %s\n", compression_type_str(biCompression));
	printf("|- image size: %dB\n", biSizeImage);
	printf("|- XPelsPerMeter: %d\n", biXPelsPerMeter);
	printf("|- YPelsPerMeter: %d\n", biYPelsPerMeter);
	printf("|- count of colors used: %d\n", biCirUsed);
	printf("|- count of important colors: %d\n", biClrImportant);

	// Color table (if any) starts at this possition
	u_int16_t color_tab_start = (biSize - 40) + 54;
	fseek(f, color_tab_start, SEEK_SET);

	// RGBQUAD[]
	printf("Color table\n");
	for (int i = 0; i < biCirUsed; i++)
	{
		u_int8_t b;
		u_int8_t g;
		u_int8_t r;
		u_int8_t z;

		b = fgetc(f);
		g = fgetc(f);
		r = fgetc(f);
		z = fgetc(f);
	
		printf("color[%d]: #B:%x G:%x R:%x 4:%x\n", i, b, g, r, z);

	}

	// BITS	
	int byte, i = 1;
	while ((byte = fgetc(f)) != EOF)
	{
		printf("(%d) data: ", i);
		byte_bin_str(byte);
		printf(", (dec: %d)\n", byte);
		i++;
	}
	
	return 0;
}
