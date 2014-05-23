#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"

BMP_HEADER bmp_header_init (void)
{
	BMP_HEADER h = NULL;

	h = (BMP_HEADER)malloc(sizeof(*h));
	if (h)
		memset ((void*)h, 0, sizeof(*h));
	else {
		fprintf (stderr, "bmp_header_init: failed\n");
		return NULL;
	}

	h->type[0] = 'B';
	h->type[1] = 'M';
	h->reserved_1 = 0;

	return h;
}

BMP_INFO bmp_info_init (void)
{
	BMP_INFO h = NULL;

	h = (BMP_INFO)malloc(sizeof(*h));
	if (h)
		memset ((void*)h, 0, sizeof(*h));
	else {
		fprintf (stderr, "bmp_info_init: failed\n");
		return NULL;
	}

	h->n_planes = 1;
	h->hdpm = 0;
	h->vdpm = 0;
	h->clr_palette = 0;
	h->clr_used = 0;

	return h;
}

BMP_FILE bmp_init (void)
{
	BMP_FILE p = NULL;

	p = (BMP_FILE)malloc(sizeof(*p));
	if (p) {
		memset ((void*)p, 0, sizeof(*p));
		p->header = bmp_header_init();
		p->info = bmp_info_init();
	}

	return p;
}

void bmp_destroy (BMP_FILE p)
{
	if (p) {
		if (p->header)
			free (p->header);
		if (p->info)
			free (p->info);
		if (p->palette)
			free (p->palette);
		if (p->data)
			free (p->data);
		free (p);
	}
}

void bmp_header_fill (BMP_HEADER h, FILE *f)
{
	rewind (f);
	if (!(fread(h, 14, 1, f)))
		fprintf (stderr, "bmp_header_fill: error\n");
}

void bmp_header_print (BMP_HEADER h)
{
	if (h) {
		printf ("bmp_header_print: start\n");
		printf ("type: %c%c\n", h->type[0], h->type[1]);
		printf ("size: %d\n", h->size);
		printf ("reserved_1: %d\n", h->reserved_1);
		printf ("reserved_2: %d\n", h->reserved_2);
		printf ("offset: %d\n", h->offset);
		printf ("bmp_header_print: end\n");
	} else fprintf (stderr, "bmp_header_print: no header, error");
}

void bmp_info_fill (BMP_INFO h, FILE *f)
{
	fseek (f, 14, SEEK_SET);
	if (!(fread(h, 40, 1, f)))
		fprintf (stderr, "bmp_info_fill: error\n");
}

void bmp_info_print (BMP_INFO h)
{
	if (h) {
		printf ("bmp_info_print: start\n");
		printf ("size: %d\n", h->size);
		printf ("width: %d\n", h->width);
		printf ("height: %d\n", h->height);
		printf ("n_planes: %d\n", h->n_planes);
		printf ("bits_per_pixel: %d\n", h->bits_per_pixel);
		printf ("compression: %d\n", h->compression);
		printf ("image_size: %d\n", h->image_size);
		printf ("hdpm: %d\n", h->hdpm);
		printf ("vdpm: %d\n", h->vdpm);
		printf ("clr_palette: %d\n", h->clr_palette);
		printf ("clr_used: %d\n", h->clr_used);
		printf ("bmp_info_print: end\n");
	} else fprintf (stderr, "bmp_info_print: no header, error");
}

void bmp_fill (BMP_FILE p, FILE *f)
{
	BMP_HEADER h = p->header;
	BMP_INFO info = p->info;

	if (f) {
		bmp_header_fill (h, f);
		bmp_info_fill (info, f);
		verbose bmp_header_print (h);
		verbose bmp_info_print (info);
		bmp_unpack (p, f);
	}
}

void bmp_unpack (BMP_FILE p, FILE* f)
{
	DWORD i, j;
	DWORD offset = p->header->reserved_2;
	DWORD height = abs(p->info->height);
	DWORD row_size = ((p->info->bits_per_pixel * p->info->width + 31) / 32) * 4;

	verbose printf ("bmp_unpack: offset = %d, row_size = %d\n", offset, row_size);

	fseek (f, offset, SEEK_SET);

	if (!(p->data = (BYTE*)malloc(row_size * abs(p->info->height)))) {
		fprintf (stderr, "bmp_unpack: error allocating %d bytes\n", row_size * abs(p->info->height));
		exit (EXIT_FAILURE);
	}

/*
	if (!fread(&(p->data), 1, p->info->image_size, f)) {
		fprintf (stderr, "bmp_unpack: error reading file to pixel array\n");
		exit (EXIT_FAILURE);
	}
*/

	for (i = 0; i < height; i++) {
		for (j = 0; j < row_size; j++) {
			if (!fread(&(p->data[i*row_size + j]), 1, 1, f)) {
				fprintf (stderr, "bmp_unpack: error reading file to pixel array\n");
				exit (EXIT_FAILURE);
			}
		}
//		verbose printf ("bmp_unpack: read %d row\n", i);
	}

	verbose printf ("bmp_unpack: read %d bytes, %d bytes in file left\n", row_size * height, SEEK_END - SEEK_CUR);	
}

BMP_FILE bmp_fill_wrapper (char *name)
{
	BMP_FILE b = bmp_init();
	FILE *f = fopen(name, "rb");
	bmp_fill (b, f);
	fclose (f);

	return b;
}

RAW_RGB bmp_to_raw (BMP_FILE p)
{
	DWORD i, j, row_size, height;
	RAW_RGB r = raw_init();
	BMP_INFO pi;
	BYTE l;

	if (!r) {
		fprintf (stderr, "bmp_to_raw: init failed\n");
		return NULL;
	}

	if (!p || !p->header || !p->info || !p->data) {
		fprintf (stderr, "bmp_to_raw: no BMP present\n");
		return NULL;
	}

	pi = p->info;

	r->x_min = r->y_min = 0;
	r->x_max = pi->width - 1;
	height = abs(pi->height);
	r->y_max = height - 1;

	if (!(r->data = (BYTE**)malloc(abs(pi->height) * sizeof(BYTE*)))) {
		fprintf (stderr, "bmp_to_raw: failed allocating raw data array\n");
		return NULL;
	}

	row_size = ((pi->bits_per_pixel * pi->width + 31) / 32) * 4;

	if (pi->height > 0 && pi->compression == 0) {
		for (i = 0; i < height; i++) {
			if (!(r->data[i] = (BYTE*)malloc(3 * pi->width))) {
				fprintf (stderr, "bmp_to_raw: failed allocating %d bytes on %d step\n", 3 * pi->width, i);
				return NULL;
			}
			for (j = 0; j < 3 * pi->width; j += 3)
				for (l = 0; l < 3; l++)
					r->data[i][j + l] = p->data[(height - i - 1) * row_size + j + 2 - l];	
		}
	} else if (pi->height < 0 && pi->compression == 0) {
		for (i = 0; i < height; i++) {
			if (!(r->data[i] = (BYTE*)malloc(3 * pi->width))) {
				fprintf (stderr, "bmp_to_raw: failed allocating %d bytes on %d step\n", 3 * pi->width, i);
				return NULL;
			}
			for (j = 0; j < 3 * pi->width; j += 3)
				for (l = 0; l < 3; l++)
					r->data[i][j + l] = p->data[i * row_size + j + 2 - l];	
		} 
	} else {
		fprintf (stderr, "bmp_to_raw: unsupported file type\n");
		return NULL;
	}

	verbose printf ("bmp_to_raw: RAW_RGB created\n");

	return r;
}

BMP_FILE raw_to_bmp (RAW_RGB r)
{
	DWORD i, j;
	BMP_FILE p = bmp_init();
	BMP_HEADER ph = p->header;
	BMP_INFO pi = p->info;
	DWORD row_size;
	BYTE l;

	if (!p || !ph || !pi) {
		fprintf (stderr, "raw_to_bmp: init failed\n");
		exit (EXIT_FAILURE);
	}

//	verbose printf ("raw_to_bmp: started\n");

	ph->reserved_2 = 54;
	ph->offset = 0;
	pi->size = 40;
	pi->width = r->x_max - r->x_min + 1;
	pi->height = r->y_max - r->y_min + 1;

	verbose printf (
		"raw_to_bmp: reserved_2 = %d, offset = %d, size = %d, width = %d, height = %d\n",
		ph->reserved_2, ph->offset, pi->size, pi->width, pi->height
	);

	/* Эти свойства можно варьировать, пока пускай так будет */
	pi->compression = 0;
	pi->bits_per_pixel = 24;

	row_size = ((pi->bits_per_pixel * pi->width + 31) / 32) * 4;

	pi->image_size = row_size * pi->height;

	verbose printf ("raw_to_bmp: row_size = %d, image_size = %d\n", row_size, pi->image_size);

	if (!(p->data = (BYTE*)malloc(3 * row_size * pi->height))) {
		fprintf (stderr, "raw_to_bmp: failed allocating %d bytes\n", 3 * row_size * pi->height);
		return NULL;
	}

	ph->size = 54 + 3 * row_size * pi->height;

	verbose printf ("raw_to_bmp: file size = %d bytes\n", ph->size);

	for (i = 0; i < pi->height; i++) {
		for (j = 0; j < 3 * pi->width; j += 3)
			for (l = 0; l < 3; l++)
				p->data[(pi->height - i - 1)*row_size + j + l] = r->data[i][j + 2 - l];
		for (; j < row_size; j++)
			p->data[(pi->height - i - 1)*row_size + j] = 0;
//		verbose printf ("raw_to_bmp: raw %d processed\n", i);
	}

	verbose printf ("raw_to_bmp: BMP_FILE created\n");

	return p;
}

void bmp_write (BMP_FILE p, char *file_name)
{
	FILE *f;

	if (!p || !p->header || !p->info || !p->data) {
		fprintf (stderr, "bmp_write: no BMP_FILE to write is present\n");
		exit (EXIT_FAILURE);
	}

	if (!(f = (FILE*)fopen(file_name, "wb"))) {
		fprintf (stderr, "bmp_write: failed creating file\n");
		exit (EXIT_FAILURE);
	}


	verbose printf ("bmp_write: writing %d bytes to %s\n", p->header->size, file_name);

	if (!fwrite(p->header, 14, 1, f)) {
		fprintf (stderr, "bmp_write: failed writing header to file\n");
		exit (EXIT_FAILURE);
	}

	verbose {
		printf ("bmp_write: written header\n");
		bmp_header_print (p->header);
	}

	if (!fwrite(p->info, 40, 1, f)) {
		fprintf (stderr, "bmp_write: failed writing info to file\n");
		exit (EXIT_FAILURE);
	}

	verbose {
		printf ("bmp_write: written info\n");
		bmp_info_print (p->info);
	}

	if (!fwrite(p->data, p->info->image_size, 1, f)) {
		fprintf (stderr, "bmp_write: failed writing graphic data to file\n");
		exit (EXIT_FAILURE);
	}

	fclose (f);
}