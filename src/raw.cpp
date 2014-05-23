#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"

RAW_RGB raw_init (void)
{
	RAW_RGB r = NULL;

	r = (RAW_RGB)malloc(sizeof(*r));
	if (r)
		memset ((void*)r, 255, sizeof(*r));
	else {
		fprintf (stderr, "raw_init: failed\n");
		return NULL;
	}

	r->x_min = r->y_min = r->x_max = r->y_max = 0;

	return r;
}

void raw_destroy (RAW_RGB r)
{
	DWORD i, height = r->y_max - r->y_min + 1;

	if (r) {
		if (r->data) {
			for (i = 0; i < height; i++)
				if (r->data[i])
					free (r->data[i]);
			free (r->data);
		}
		free (r);
	}
}

void raw_copy (RAW_RGB r_dest, RAW_RGB r_src)
{
	DWORD height, i, line_size;

	if (!r_src || !r_src->data || !r_dest) {
		fprintf (stderr, "raw_copy: no source or destination pointer\n");
		exit (EXIT_FAILURE);
	}

	height = r_src->y_max - r_src->y_min + 1;
	line_size = 3 * (r_src->x_max - r_src->x_min + 1);

	r_dest->y_max = r_src->y_max;
	r_dest->y_min = r_src->y_min;
	r_dest->x_max = r_src->x_max;
	r_dest->x_min = r_src->x_min;

	raw_data_alloc (r_dest);

	for (i = 0; i < height; i++)
		memcpy ((void*)r_dest->data[i], (void*)r_src->data[i], line_size);

	verbose printf ("raw_copy: done\n");
}

/* Гигантская дура, разбивающая RAW_RGB на изображения размером <=max_width на <=max_height */
/* n - количество получившихся изображений */
RAW_RGB* raw_divide (RAW_RGB r, DWORD max_width, DWORD max_height, BYTE *n)
{
	BYTE i, j;								// пара счетчиков (основные)
	BYTE v_count, h_count;					// количество изображений по вертикали и по горизонтали
	RAW_RGB *r_div;							// получившийся массив
	DWORD width, height;					// ширина и высота исходной изображения
	DWORD res_width, res_height;			// остатки от деления исходных ширины и высоты на требуемые
	DWORD k, l;								// счетчики для копирования

	verbose printf ("raw_divide: init\n");

	width = r->x_max - r->x_min + 1;
	height = r->y_max - r->y_min + 1;

	verbose printf ("raw_divide: original image width = %u, height = %u\n", width, height);

	res_width = width % max_width;
	res_height = height % max_height;
	h_count = width / max_width + (res_width ? 1 : 0);
	v_count = height / max_height  + (res_height ? 1 : 0);
	*n = h_count * v_count;

	verbose printf ("raw_divide: dividing into %d images, max_width = %u, max_height = %u, v_count = %d, h_count = %d\n",
		*n, max_width, max_height, v_count, h_count);

	if (!(r_div = (RAW_RGB*)malloc((*n) * sizeof(RAW_RGB)))) {
		fprintf (stderr, "raw_divide: init failed\n");
		exit (EXIT_FAILURE);
	}

	verbose printf ("raw_divide: init done\n");

	for (i = 0; i < v_count - (res_height ? 1 : 0); i++) {
		for (j = 0; j < h_count - (res_width ? 1 : 0); j++) {
			if (!(r_div[i*h_count + j] = raw_init())) {
				fprintf (stderr, "raw_divide: init failed on %d step\n", i*h_count + j);
				return NULL;
			}
			
			r_div[i*h_count + j]->x_min = r->x_min + j * max_width;
			r_div[i*h_count + j]->y_min = r->y_min + i * max_height;
			r_div[i*h_count + j]->x_max = r->x_min + (j + 1) * max_width - 1;
			r_div[i*h_count + j]->y_max = r->y_min + (i + 1) * max_height - 1;
			
			verbose printf ("raw_divide: step %d, x_min = %u, y_min = %u, x_max = %u, y_max = %u\n",
				i*h_count + j,
				r_div[i*h_count + j]->x_min, r_div[i*h_count + j]->y_min,
				r_div[i*h_count + j]->x_max, r_div[i*h_count + j]->y_max
			);

			raw_data_alloc (r_div[i*h_count + j]);
		
			verbose printf ("raw_divide: copy start at x = %u, y = %u; end at x = %u, y = %u\n",
				3 * j * max_width, i * max_height, (j + 1) * 3 * max_width, (i + 1) * max_height
			);

			for (k = 0; k < max_height; k++) {
				for (l = 0; l < 3 * max_width; l++)
					r_div[i*h_count + j]->data[k][l] = r->data[i * max_height + k][3 * j * max_width + l];
			}

			verbose printf ("raw_divide: image %d created\n", i*h_count + j);
		}

		verbose printf ("raw_divide: checking res_width = %d\n", res_width);

		if (res_width) {
			if (!(r_div[i*h_count + j] = raw_init())) {
				fprintf (stderr, "raw_divide: init failed on %d step\n", i);
				return NULL;
			}

			r_div[i*h_count + j]->x_min = r->x_min + j * max_width;
			r_div[i*h_count + j]->y_min = r->y_min + i * max_height;
			r_div[i*h_count + j]->x_max = r->x_min + j * max_width + res_width - 1;
			r_div[i*h_count + j]->y_max = r->y_min + (i + 1) * max_height - 1;

			verbose printf ("raw_divide: step %d, x_min = %u, y_min = %u, x_max = %u, y_max = %u\n",
				i*h_count + j,
				r_div[i*h_count + j]->x_min, r_div[i*h_count + j]->y_min,
				r_div[i*h_count + j]->x_max, r_div[i*h_count + j]->y_max
			);

			raw_data_alloc (r_div[i*h_count + j]);

			verbose printf ("raw_divide: copy start at x = %u, y = %u; end at x = %u, y = %u\n",
				3 * j * max_width, i * max_height, 3 * (j * max_width + res_width), (i + 1) * max_height
			);

			for (k = 0; k < max_height; k++) {
				for (l = 0; l < 3 * res_width; l++)
					r_div[i*h_count + j]->data[k][l] = r->data[i * max_height + k][3 * j * max_width + l];
			}
		}
	}

	verbose printf ("raw_divide: checking res_height = %d\n", res_height);

	if (res_height) {
		for (j = 0; j < h_count - (res_width ? 1 : 0); j++) {
			if (!(r_div[i*h_count + j] = raw_init())) {
				fprintf (stderr, "raw_divide: init failed on %d step\n", i*h_count + j);
				return NULL;
			}
			
			r_div[i*h_count + j]->x_min = r->x_min + j * max_width;
			r_div[i*h_count + j]->y_min = r->y_min + i * max_height;
			r_div[i*h_count + j]->x_max = r->x_min + (j + 1) * max_width - 1;
			r_div[i*h_count + j]->y_max = r->y_min + i * max_height + res_height - 1;
			
			verbose printf ("raw_divide: step %d, x_min = %u, y_min = %u, x_max = %u, y_max = %u\n",
				i*h_count + j,
				r_div[i*h_count + j]->x_min, r_div[i*h_count + j]->y_min,
				r_div[i*h_count + j]->x_max, r_div[i*h_count + j]->y_max
			);

			raw_data_alloc (r_div[i*h_count + j]);
		
			verbose printf ("raw_divide: copy start at x = %u, y = %u; end at x = %u, y = %u\n",
				3 * j * max_width, i * max_height, (j + 1) * 3 * max_width, i * max_height + res_height
			);

			for (k = 0; k < res_height; k++) {
				for (l = 0; l < 3 * max_width; l++)
					r_div[i*h_count + j]->data[k][l] = r->data[i * max_height + k][3 * j * max_width + l];
			}

			verbose printf ("raw_divide: image %d created\n", i*h_count + j);
		}

		verbose printf ("raw_divide: checking res_width = %d\n", res_width);

		if (res_width) {
			if (!(r_div[i*h_count + j] = raw_init())) {
				fprintf (stderr, "raw_divide: init failed on %d step\n", i);
				return NULL;
			}

			r_div[i*h_count + j]->x_min = r->x_min + j * max_width;
			r_div[i*h_count + j]->y_min = r->y_min + i * max_height;
			r_div[i*h_count + j]->x_max = r->x_min + j * max_width + res_width - 1;
			r_div[i*h_count + j]->y_max = r->y_min + i * max_height + res_height - 1;

			verbose printf ("raw_divide: step %d, x_min = %u, y_min = %u, x_max = %u, y_max = %u\n",
				i*h_count + j,
				r_div[i*h_count + j]->x_min, r_div[i*h_count + j]->y_min,
				r_div[i*h_count + j]->x_max, r_div[i*h_count + j]->y_max
			);

			raw_data_alloc (r_div[i*h_count + j]);

			verbose printf ("raw_divide: copy start at x = %u, y = %u; end at x = %u, y = %u\n",
				3 * j * max_width, i * max_height, 3 * (j * max_width + res_width), i * max_height + res_height
			);

			for (k = 0; k < res_height; k++) {
				for (l = 0; l < 3 * res_width; l++)
					r_div[i*h_count + j]->data[k][l] = r->data[i * max_height + k][3*j*max_width + l];
			}
		}
	}

	verbose printf ("raw_divide: done\n");

	return r_div;
}

RAW_RGB* raw_divide_by_pcx_4bit_size (RAW_RGB r, DWORD size, BYTE *n)
{
	DWORD data_size = size - 128;
	DWORD line_size = (r->x_max - r->x_min + 1) / 2 + (r->x_max - r->x_min + 1) % 2;
	DWORD curr_data_size = line_size * (r->y_max - r->y_min + 1);

	return raw_divide (
		r, 
		r->x_max - r->x_min + 1,
		(r->y_max - r->y_min + 1) / (curr_data_size / data_size + 1) + 1,
		&(*n)
	);
}

/* Склеиваем два сырых изображения. При наложении приоритет имеет НЕ белый цвет. */
RAW_RGB raw_glue (RAW_RGB r1, RAW_RGB r2)
{
	DWORD i, j;
	RAW_RGB res;
	DWORD height, width;
	DWORD total_height, total_width;
	BYTE default_count = 0, k;

	if (!(res = raw_init())) {
		fprintf (stderr, "raw_glue: init failed");
		return NULL;
	}

	if (!r1 || !r2 || !r1->data || !r2->data) {
		fprintf (stderr, "raw_glue: image not found");
		return NULL;
	}

	verbose printf ("raw_glue: init\n");

	res->x_min = (r1->x_min > r2->x_min) ? r2->x_min : r1->x_min;
	res->y_min = (r1->y_min > r2->y_min) ? r2->y_min : r1->y_min;
	res->x_max = (r1->x_max > r2->x_max) ? r1->x_max : r2->x_max;
	res->y_max = (r1->y_max > r2->y_max) ? r1->y_max : r2->y_max;

	total_height = res->y_max - res->y_min + 1;
	total_width  = res->x_max - res->x_min + 1;

	verbose printf ("raw_glue: x_min = %u, x_max = %u, y_min = %u, y_max = %u\n",
		res->x_min, res->x_max, res->y_min, res->y_max
	);

	raw_data_alloc (res);
	raw_data_blank (res);

	height = r1->y_max - r1->y_min + 1;
	width = r1->x_max - r1->x_min + 1;
	verbose {
		printf ("raw_glue: first image width = %u, height = %u\n", width, height);
		printf ("raw_glue: total_height = %u, r1->y_max = %u\n", total_height, r1->y_max);
		printf ("raw_glue: height start = %u\n", (r1->y_min - res->y_min));
		printf ("raw_glue: height end = %u\n", (r1->y_min - res->y_min) + height - 1);
	}
	for (i = 0; i < height; i++) {
		for (j = 0; j < 3 * width; j++)
			res->data[(r1->y_min - res->y_min) + i][3 * (r1->x_min - res->x_min) + j] = r1->data[i][j];
	}
	verbose printf ("raw_glue: first image copied\n");

	height = r2->y_max - r2->y_min + 1;
	width = r2->x_max - r2->x_min + 1;
	verbose {
		printf ("raw_glue: second image width = %u, height = %u\n", width, height);
		printf ("raw_glue: total_height = %u, r2->y_max = %u\n", total_height, r2->y_max);
		printf ("raw_glue: height start = %u\n", (r2->y_min - res->y_min));
		printf ("raw_glue: height end = %u\n", (r2->y_min - res->y_min) + height - 1);
	}
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			/* При наложении карт дефолтным считаем белый (0xFFFFFF), поэтому можно делать побитовое И. */
			if ( r2->y_min + i >= r1->y_min && r2->y_min + i <= r1->y_max &&
				 j >= r1->x_min - res->x_min && j <= r1->x_max - res->x_min
			) for (k = 0; k < 3; k++)
				res->data[(r2->y_min - res->y_min) + i][3 * (r2->x_min - res->x_min) + 3 * j + k] &= r2->data[i][3 * j + k];
			else for (k = 0; k < 3; k++)
				res->data[(r2->y_min - res->y_min) + i][3 * (r2->x_min - res->x_min) + 3 * j + k] = r2->data[i][3 * j + k];
		}
	}
	verbose printf ("raw_glue: second image copied\n");

	return res;
}

/* Склеиваем целый массив сырых изображений. */
RAW_RGB raw_glue_array (RAW_RGB *r_div, BYTE n)
{
	BYTE i;
	RAW_RGB r1, r2;

	if (n == 1) {
		printf ("raw_glue_array: there is only one image in the array\n");
		return r_div[0];
	} else if (n < 1) {
		fprintf (stderr, "raw_glue_array: no images present\n"); 
		return NULL;
	}

	r1 = raw_glue(r_div[0], r_div[1]);
	for (i = 2; i < n; i++) {
		r2 = r1;
		r1 = raw_glue(r2, r_div[i]);
		raw_destroy(r2);
	}

	return r1;
}

void raw_data_alloc (RAW_RGB r)
{
	DWORD i;
	DWORD height, width;

	width = r->x_max - r->x_min + 1;
	height = r->y_max - r->y_min + 1;

	if (!(r->data = (BYTE**)malloc(height * sizeof(BYTE*)))) {
		fprintf (stderr, "raw_data_alloc: failed allocating row pointer array\n");
		exit (EXIT_FAILURE);
	}

	for (i = 0; i < height; i++) {
		if (!(r->data[i] = (BYTE*)malloc(3 * width))) {
			fprintf (stderr, "raw_data_alloc: failed allocating %u row\n", i);
			exit (EXIT_FAILURE);
		}
	}

	verbose printf ("raw_data_alloc: done\n");
}

void raw_data_blank (RAW_RGB r)
{
	DWORD i;
	DWORD height, width;

	width = r->x_max - r->x_min + 1;
	height = r->y_max - r->y_min + 1;

	if (!r->data) {
		fprintf (stderr, "raw_data_blank: failed allocating row pointer array\n");
		exit (EXIT_FAILURE);
	}

	for (i = 0; i < height; i++)
		memset (r->data[i], 0xFF, 3 * width);

	verbose printf ("raw_data_blank: done\n");
}