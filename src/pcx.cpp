#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcx.h"

PCX_HEADER pcx_header_init (void)
{
	PCX_HEADER h = NULL;

	h = (PCX_HEADER)malloc(sizeof(*h));
	if (h)
		memset ((void*)h, 0, sizeof(*h));
	else {
		fprintf (stderr, "pcx_header_init: failed\n");
		return NULL;
	}

	h->manufacturer = 10;
	h->version = 5;

	return h;
}

void pcx_destroy (PCX_FILE p)
{
	if (p) {
		if (p->header)
			free (p->header);
		if (p->data)
			free (p->data);
		if (p->palette)
			free (p->palette);
		free (p);
	}
}

PCX_FILE pcx_init (void)
{
	PCX_FILE p = NULL;

	p = (PCX_FILE)malloc(sizeof(*p));
	if (p) {
		memset ((void*)p, 0, sizeof(*p));
		p->header = pcx_header_init();
	}

	return p;
}

void pcx_header_fill (PCX_HEADER h, FILE *f)
{
	rewind (f);
	if (!(fread(h, 128, 1, f)))
		fprintf (stderr, "pcx_header_fill: error\n");
}

void pcx_header_print (PCX_HEADER h)
{
	int i;

	if (h) {
		printf ("pcx_header_print: start\n");

		printf ("manufacturer: %d\n", h->manufacturer);
		printf ("version: %d\n", h->version);
		printf ("encoding: %d\n", h->encoding);
		printf ("bits_per_pixel: %d\n", h->bits_per_pixel);
		printf ("x_min: %d\n", h->x_min);
		printf ("y_min: %d\n", h->y_min);
		printf ("x_max: %d\n", h->x_max);
		printf ("y_max: %d\n", h->y_max);
		printf ("hdpi: %d\n", h->hdpi);
		printf ("vdpi: %d\n", h->vdpi);

		printf ("colormap: ");
		for (i = 0; i < 48; i++)
			printf ("%d ", h->colormap[i]);
		printf ("\n");

		printf ("reserved: %d\n", h->reserved);
		printf ("n_planes: %d\n", h->n_planes);
		printf ("bytes_per_line: %d\n", h->bytes_per_line);
		printf ("palette_info: %d\n", h->palette_info);
		printf ("hscreensize: %d\n", h->hscreensize);
		printf ("vscreensize: %d\n", h->vscreensize);

		printf ("filler: ");
		for (i = 0; i < 54; i++)
			printf ("%d ", h->filler[i]);
		printf ("\n");

		printf ("pcx_header_print: end\n");
	} else fprintf (stderr, "pcx_header_print: no header, error");
}

void pcx_fill (PCX_FILE p, FILE *f)
{
	PCX_HEADER h = p->header;

	if (f) {
		pcx_header_fill (h, f);
		verbose pcx_header_print (h);
		pcx_unpack (p, f);
	}
}

void pcx_unpack (PCX_FILE p, FILE *f)
{
	PCX_HEADER h = p->header;
	DWORD total_bytes;
	DWORD x_size, y_size;

	DWORD i, j, l;
	BYTE count = 1;
	BYTE repeater = 3 << 6;
	BYTE t;

	x_size = h->x_max - h->x_min + 1;
	y_size = h->y_max - h->y_min + 1;
	total_bytes = h->n_planes * h->bytes_per_line;

	verbose printf ("pcx_unpack: x_size = %d, y_size = %d, total_bytes = %d\n", x_size, y_size, total_bytes);

	if (!(p->data = (BYTE*)malloc(y_size * total_bytes * sizeof(BYTE*)))) {
		fprintf (stderr, "pcx_unpack: failed allocating %d bytes\n", y_size);
		exit (EXIT_FAILURE);
	}

	fseek (f, 128, SEEK_SET);

	if (p->header->encoding) {
		for (i = 0; i < y_size; i++) {
			j = 0;
			while (j < total_bytes) {
				if (!(fread(&t, 1, 1, f))) {
					fprintf (stderr, "pcx_unpack: failed reading file\n");
					exit (EXIT_FAILURE);
				}

				if (t > repeater) {
					count = t - repeater;
					if (!(fread(&t, 1, 1, f))) {
						fprintf (stderr, "pcx_unpack: failed reading file\n");
						exit (EXIT_FAILURE);
					}
	
					for (l = 0; l < count; l++) { 
						p->data[i*total_bytes + j] = t;
						j++;
					}
				} else {
					p->data[i*total_bytes + j] = t;
					j++;
				}
			}
		}
		p->header->encoding = 0;
	} else {
		for (i = 0; i < y_size; i++)
			for (j = 0; j < total_bytes; j++)
				if (!(fread(&(p->data[i*total_bytes + j]), 1, 1, f))) {
					fprintf (stderr, "pcx_unpack: failed reading file\n");
					exit (EXIT_FAILURE);
				}
	}

	verbose printf ("pcx_unpack: %d bytes of data read, %d bytes in file left\n", total_bytes * y_size, SEEK_END - SEEK_CUR);
}

PCX_FILE pcx_fill_wrapper (char *name)
{
	PCX_FILE p = pcx_init();
	FILE *f = fopen(name, "rb");
	pcx_fill (p, f);
	fclose (f);

	return p;
}

#define MACRO_REALLOC(x) if (!(c = (BYTE*)realloc(c, x))) { fprintf (stderr, "pcx_compress: failed allocating %d bytes\n", x); return NULL; }

BYTE* pcx_compress (BYTE *p, DWORD n, DWORD *k)
{
	DWORD i;
	BYTE max_count = 0x3F;	// 00111111
	BYTE max_value = 0xBF;	// 10111111
	BYTE repeater  = 0xC0;	// 11000000
	BYTE *c, t, count;

	if (!(c = (BYTE*)malloc(1)) || n < 1) {
		fprintf (stderr, "pcx_compress: init failed\n");
		return NULL;
	}

	*k = 0;
	t = p[0];
	count = 1;
	for (i = 1; i < n; i++) {
		if (p[i] == t)
			count++;
		else if (count > 1) {
			MACRO_REALLOC(*k + 2)
			c[*k] = repeater + count;
			c[++(*k)] = t;
			t = p[i];
			count = 1;
			++(*k);
		} else if (count == 1) {
			if (t > max_value) {
				MACRO_REALLOC(*k + 2)
				c[*k] = repeater + count;
				++(*k);
			} else MACRO_REALLOC(*k + 1)
			c[*k] = t;
			t = p[i];
			++(*k);
		} else if (count == 0) {
			t = p[i];
			count++;
		}

		if (count == max_count) {
			MACRO_REALLOC(*k + 2)
			c[*k] = repeater + count;
			c[++(*k)] = t;
			count = 0;
			++(*k);
		}
	}

	if (count > 1) {
		MACRO_REALLOC(*k + 2)
		c[*k] = repeater + count;
		c[++(*k)] = t;
		++(*k);
	} else if (count == 1) {
		if (t > max_value) {
			MACRO_REALLOC(*k + 2)
			c[*k] = repeater + count;
			++(*k);
		} else MACRO_REALLOC(*k + 1)
		c[*k] = t;
		t = p[i];
		++(*k);
	} else if (count == 0) {
		t = p[i];
		count++;
	}

	return c;
}

#undef MACRO_REALLOC

/** Функция не используется и не нужна, в общем-то. */
/*
BYTE* pcx_decompress (BYTE *p, DWORD n, DWORD *k)
{
	DWORD i;
	BYTE *d, t, count, j;
	BYTE repeater = 3 << 6;

	*k = 0;

	if (!(d = (BYTE*)malloc(sizeof(BYTE)))) {
		fprintf (stderr, "pcx_decompress: init failed\n", (*k + 1));
		return NULL;
	}

	for (i = 0; i < n; i++) {
		if (p[i] > repeater) {
			count = p[i] - repeater;
			t = p[++i];

			if (!(d = (BYTE*)realloc(d, (*k + count)))) {
				fprintf (stderr, "pcx_decompress: failed allocating %d bytes\n", (*k + 1));
				return NULL;
			}

			for (j = 0; j < count; j++) {
				d[*k] = t;
				++(*k);
			}
		} else {
			if (!(d = (BYTE*)realloc(d, (*k + 1)))) {
				fprintf (stderr, "pcx_decompress: failed allocating %d bytes\n", (*k + 1));
				return NULL;
			}
			d[*k] = p[i];
			++(*k);
		}
	}

	return d;
}
*/

/* Конвертация PCX в сырое изображение. */
RAW_RGB pcx_to_raw (PCX_FILE p)
{
	DWORD i, j;
	DWORD width, height;
	PCX_HEADER ph = p->header;
	DWORD pcx_total_bytes;
	RAW_RGB r;
	BYTE t, l;

	if (!p || !ph) {
		fprintf (stderr, "pcx_to_raw: PCX_FILE error\n");
		return NULL;
	}

	if (!(r = raw_init())) {
		fprintf (stderr, "pcx_to_raw: raw_init() error\n");
		return NULL;
	}

	r->x_min = ph->x_min;
	r->x_max = ph->x_max;
	r->y_min = ph->y_min;
	r->y_max = ph->y_max;

	width = r->x_max - r->x_min + 1;
	height = r->y_max - r->y_min + 1;
	pcx_total_bytes = ph->n_planes * ph->bytes_per_line;

	if (!(r->data = (BYTE**)malloc(height * sizeof(BYTE*)))) {
		fprintf (stderr, "pcx_to_raw: failed allocating %d bytes\n", height);
		return NULL;
	}

	for (i = 0; i < height; i++) {
		if (!(r->data[i] = (BYTE*)malloc(3 * width))) {
			fprintf (stderr, "pcx_to_raw: failed allocating %d bytes\n", height);
			return NULL;
		}
	}


	verbose printf ("pcx_to_raw: n_planes = %d, bits_per_pixel = %d\n", ph->n_planes, ph->bits_per_pixel);
	if (ph->n_planes == 1) {
		if (ph->bits_per_pixel == 4) {
			for (i = 0; i < height; i++) {
				for (j = 0; j < width; j += 2) {
					t = p->data[i*pcx_total_bytes + (j / 2)] >> 4;
					for (l = 0; l < 3; l++)
						r->data[i][3*j + l] = ph->colormap[3*t + l];
				}
				for (j = 1; j < width; j += 2) {
					t = p->data[i*pcx_total_bytes + (j / 2)] & 0xF;
					for (l = 0; l < 3; l++)
						r->data[i][3*j + l] = ph->colormap[3*t + l];
				}
			}
		} else {
			printf ("pcx_to_raw: not supported, exiting\n");
			exit (EXIT_FAILURE);
		}
	} else if (ph->n_planes == 3) {
		if (ph->bits_per_pixel == 8) {
			for (i = 0; i < height; i++)
				for (j = 0; j < width; j++)
					for (l = 0; l < 3; l++)
						r->data[i][3*j + l] = p->data[(3*i + l)*width + j];
		} else {
			printf ("pcx_to_raw: not supported, exiting\n");
			exit (EXIT_FAILURE);
		}
	} else {
		printf ("pcx_to_raw: not supported, exiting\n");
		exit (EXIT_FAILURE);
	}

	verbose printf ("pcx_to_raw: RAW_RGB created\n");

	return r;
}

/* Конвертация сырого изображения в PCX. */
/* Если есть палитра, то будет скодировано в четырехбитный, иначе 24-битный. */
PCX_FILE raw_to_pcx (RAW_RGB r, BYTE* colormap)
{
	DWORD i, j;
	PCX_FILE p = pcx_init();
	PCX_HEADER ph = p->header;
	DWORD total_bytes, width, height;
	DWORD image_size;
	BYTE l, k, m;

	if (!r || !r->data) {
		fprintf (stderr, "raw_to_pcx: no RAW_RGB present\n");
		return NULL;
	}

	ph->encoding = 0;
	ph->x_min = r->x_min;
	ph->x_max = r->x_max;
	ph->y_min = r->y_min;
	ph->y_max = r->y_max;

	width = ph->x_max - ph->x_min + 1;
	height = ph->y_max - ph->y_min + 1;

	if (!colormap) {
		verbose printf ("raw_to_pcx: no colormap present, creating 24-bit PCX\n");

		ph->bits_per_pixel = 8;
		ph->n_planes = 3;
		ph->bytes_per_line = width + (width & 0x1);
		total_bytes = ph->bytes_per_line * 3;

		image_size = height * total_bytes;

		if (!(p->data = (BYTE*)malloc(image_size))) {
			fprintf (stderr, "raw_to_pcx: graphic data init failed\n");
			return NULL;
		}

		memset ((void*)p->data, 0, image_size);
		verbose printf ("raw_to_pcx: total_bytes = %d, image_size = %d bytes\n", total_bytes, image_size);

		for (i = 0; i < height; i++)
			for (j = 0; j < width; j++)
				for (l = 0; l < 3; l++)
					p->data[i*total_bytes + j + l*(ph->bytes_per_line)] = r->data[i][3*j + l];

		verbose printf ("raw_to_pcx: PCX_FILE created\n");
	} else {
		verbose printf ("raw_to_pcx: colormap present, creating 4-bit PCX with colormap\n");

		memcpy ((void*)ph->colormap, (const void*)colormap, 48);
		verbose printf ("raw_to_pcx: colormap written\n");

		ph->bits_per_pixel = 4;
		ph->n_planes = 1;
		ph->bytes_per_line = width / 2;
		ph->bytes_per_line += ph->bytes_per_line & 0x1;
		total_bytes = ph->bytes_per_line;

		image_size = height * total_bytes; 

		if (!(p->data = (BYTE*)malloc(image_size))) {
			fprintf (stderr, "raw_to_pcx: graphic data init failed\n");
			return NULL;
		}
		
		memset ((void*)p->data, 0, image_size);
		verbose printf ("raw_to_pcx: total_bytes = %d, image_size = %d bytes\n", total_bytes, image_size);

		for (i = 0; i < height; i++) {
			for (j = 0; j < width; j++) {
				for (k = 0; k < 16; k++) {
					m = 0;
					for (l = 0; l < 3; l++) {
						if (r->data[i][3*j+l] == colormap[3*k+l])
							m++;
					}
					if (m == 3) {
						p->data[i*ph->bytes_per_line + j / 2] += (k << (((j + 1) & 0x1) * 4));
						m = 0;
						break;
					}
				}
				if (m)
					p->data[i*ph->bytes_per_line + j / 2] += (0 << (((j + 1) & 0x1) * 4));
			}
		}
	}

	return p;	
}

/* Запись PCX в файл. Рекомендуется использовать сжатие. */
void pcx_write (PCX_FILE p, char *file_name, BYTE compress)
{
	FILE *f;
	DWORD height, image_size;
	BYTE *res;
	DWORD res_size;

	DWORD i, total_bytes;

	if (!p || !p->header || !p->data) {
		fprintf (stderr, "pcx_write: no PCX_FILE to write is present\n");
		exit (EXIT_FAILURE);
	}

	if (!(f = (FILE*)fopen(file_name, "wb"))) {
		fprintf (stderr, "pcx_write: failed creating file\n");
		exit (EXIT_FAILURE);
	}

	height = p->header->y_max - p->header->y_min + 1;

	p->header->encoding = compress;
	verbose printf ("pcx_write: writing 128 byte header to %s\n", file_name);

	if (!fwrite(p->header, 128, 1, f)) {
		fprintf (stderr, "bmp_write: failed writing header to file\n");
		exit (EXIT_FAILURE);
	}

	verbose {
		printf ("pcx_write: written header\n");
		pcx_header_print (p->header);
	}

	total_bytes = p->header->bytes_per_line * p->header->n_planes;
	image_size = height * total_bytes;

	verbose printf ("pcx_write: image_size = %d bytes, total_bytes = %d\n", image_size, total_bytes);

	if (!compress) {
		if (!fwrite(p->data, image_size, 1, f)) {
			fprintf (stderr, "pcx_write: failed writing graphic data to file\n");
			exit (EXIT_FAILURE);
		}
	} else {
		verbose printf ("pcx_write: using compression\n");

		/** Сжатие переделано, поскольку некоторые вьюверы рвут и сдвигают изображение, если сжимать все разом, а не построчно. **/
		/*
		if (!(res = pcx_compress(p->data, image_size, &res_size))) {
			fprintf (stderr, "pcx_write: failed compressing\n");
			exit (EXIT_FAILURE);
		}

		verbose printf ("pcx_write: res_size = %d\n", res_size);
		if (!fwrite(res, res_size, 1, f)) {
			fprintf (stderr, "pcx_write: failed writing graphic data to file\n");
			exit (EXIT_FAILURE);
		}

		free (res);
		*/

		for (i = 0; i < height; i++) {
			if (!(res = pcx_compress(p->data + i*total_bytes, total_bytes, &res_size))) {
				fprintf (stderr, "pcx_write: failed compressing on %d line\n", i);
				exit (EXIT_FAILURE);
			}
			if (!fwrite(res, res_size, 1, f)) {
				fprintf (stderr, "pcx_write: failed writing graphic data to file\n");
				exit (EXIT_FAILURE);
			}
			free (res);
		}
	}

	verbose printf ("pcx_write: %s successfully written\n", file_name);

	fclose (f);
}