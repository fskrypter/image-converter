#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "bmp.h"
#include "pcx.h"

#define MAX 1000

void read_colormap (BYTE*, char*);

int main (int argc, char *argv[])
{
	int opt;
	char help[] = "Use: [-f filename], [-v] for verbose output.";
	BYTE count, colormap[48], i;

	char str[16];

	_v = 0;

	while ((opt = getopt(argc, argv, "v")) != -1) {
		switch (opt) {
//			case 'f':
//				f1 = fopen("files/test.bmp", "rb");
//				break;
			case 'v':
				_v = 1;
				break;
			default:
				printf ("%s\n", help);
				break;
		}
	}

	read_colormap (colormap, "colormap");

	return 0;
}

void read_colormap (BYTE* colormap, char *colormap_file)
{
	BYTE i;
	char str[8];
	unsigned int tmp;
	FILE *f = fopen(colormap_file, "r");

	if (!f) {
		fprintf (stderr, "read_colormap: no file given\n");
		exit (EXIT_FAILURE);
	}

	for (i = 0; i < 16; i++) {
		if (!fgets(str, 8, f)) {
			fprintf (stderr, "read_colormap: file read error\n");
			exit (EXIT_FAILURE);
		}
		str[7] = '\0';
		verbose printf ("read_colormap: %s\n", str);
		sscanf (str, "%x\n", &tmp);
		verbose printf ("read_colormap: %u %u %u\n", (tmp >> 8) & 0x00FF, (tmp >> 16), tmp & 0x0000FF);
		colormap[i*3] = (tmp >> 16);
		colormap[i*3 + 1] = (tmp >> 8) & 0x00FF;
		colormap[i*3 + 2] = tmp & 0x0000FF;
	}

	fclose (f);
}