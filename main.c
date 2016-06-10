#define _POSIX_C_SOURCE 2 
/*
 * GIF to BMP convertor
 *
 * This simple application performs a few opperation with GIF images. See help
 * for more details.
 *
 * Login: xsruba03
 * Autor:	Michal Šrubař
 * Email: xsruba03@stud.fit.vutbr.cz
 * Date: 	Thu Feb 18 07:24:03 CET 2016
 * Seminar: KKO (Data Coding and Compression)
 *
 */
#include "gif.h"
#include "gif_print.h"
#include "gif_lzw.h"
#include "gif_rgb.h"
#include "gif2bmp.h"
#include <unistd.h>

#define LOGIN	"xsruba03"

/* close a file descriptor the save way */
#define SAVE_FCLOSE(f) if (f != NULL) fclose(f);

const char *help = {
	"If there is no -r -t or -p option then the conversion from GIF to BMP is\n"
	"performed. If there are no -i or -o options then stdin and stdout is\n"
	"used to read and write data.\n"
	"\nOPTIONS:\n"
	"-i input\tGIF image\n"
	"-o output\tBMP image\n"
	"-l file\t\toutput log file\n"
	"-p\t\tprint internal structure of input GIF image\n"
	"-r\t\tconvert GIF image to rgb colors\n"
	"-t\t\ttest format of a GIF image\n"
	"-h\t\tprint help\n"
	"\nRETURN VALUES:\n"
	"0\tsuccess\n"
	"1\terror occured\n"
};

void usage(const char *name) {
	printf("\ngif2bmp - convert 8bit static GIF image to bitmap\n");
	printf("\nUsage: %s [-i img.gif] [-o img.bmp] [-l log] [-p] [-r] [-t] [-h]\n", name);
	printf("\n%s", help);
}

int main(int argc, char *argv[])
{
	FILE *in = stdin;
	FILE *out = stdout;
	FILE *log_out = NULL;
	char ch;
	bool print = false;
	bool rgb = false;
	bool test = false;
	bool log = false;
	int ret = 0;
	tGIF *img = NULL;

  while ((ch = getopt(argc, argv, "i:o:l:ptrh")) != EOF) {
    switch (ch) {
      case 'i':
				if ((in = fopen(optarg, "rb")) == NULL) {
					perror("fopen()");
					return 2;
				}
				break;
      case 'o':
				if ((out = fopen(optarg, "wb")) == NULL) {
					perror("fopen()");
					return 2;
				}
        break;
			case 'l':
				log = true;
				if ((log_out = fopen(optarg, "w")) == NULL) {
					perror("fopen()");
					return 2;
				}
				break;
			case 'p':
				print = true;
				break;
			case 't':
				test = true;
				break;
			case 'r':
				rgb = true;
				break;
			case 'h':
				usage(argv[0]);
				return 0;
      default:
        fprintf(stderr, "Unknown option: '%s'\n", optarg);
        return 1;
    }
  }
 
  argc -= optind;
  argv += optind;

	if (print)
		ret = gif_print_info(in);
	else if (rgb)
		// save RGB values for each pixel
		ret = gif_save_rgb(in, out);
	else if (test)
	{
		// load image into memory
		if ((img = gif_load(in)) == NULL)
			ret = 1;
		// if we got here the we've read the image successfully
		gif_unload(img);
	}
	else
	{
		tGIF2BMP gifToBmp;
		if (gif2bmp(&gifToBmp, in, out) != 0)
			ret = 1;

		if (log)
		{
			fprintf(log_out, "login = %s\n", LOGIN);
			fprintf(log_out, "uncodedSize = %ld\n", gifToBmp.bmpSize);
			fprintf(log_out, "codedSize = %ld\n", gifToBmp.gifSize);
		}
	}

	SAVE_FCLOSE(in);
	SAVE_FCLOSE(out);
	SAVE_FCLOSE(log_out);

	return ret;
}
