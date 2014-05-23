#ifndef DEFS_H
#define DEFS_H
#include "defs.h"
#endif

typedef struct _PCX_HEADER_t {
	BYTE 			manufacturer;					// Константа, 10 = ZSoft .pcx 
	
	BYTE 			version;						// Версия 
                             				   		// 0 = Версия 2.5 "PC Paintbrush"
                                					// 2 = Версия 2.8 с палитрой
                                					// 3 = Версия 2.8 без палитры
                             	   					// 4 = "PC Paintbrush для Windows (Plus for Windows версии 5)"
                             		   				// 5 = Версия 3.0 и выше "PC Paintbrush" и "PC Paintbrush +",
                                					// включая "Publisher's Paintbrush".
                                					// Сюда же входят и 24-битные .PCX файлы 
	
	BYTE 			encoding;						// 1 = .PCX "run length encoding" (RLE сжатие)
	BYTE 			bits_per_pixel;					// Бит на пиксель (per Plane) - 1, 2, 4, или 8 
	WORD 			x_min, y_min, x_max, y_max;		// Размеры изображения 	
	WORD 			hdpi;							// Горизонтальное разрешение DPI
	WORD 			vdpi;							// Вертикальное разрешение DPI
	BYTE 			colormap[48];					// Палитра
	BYTE 			reserved;						// Обязан быть установлен в 0
	BYTE 			n_planes;						// Число color planes
	
	WORD 			bytes_per_line;					// Количество байт на один scanline plane
													// ДОЛЖНО быть ЧЕТНЫМ числом. НЕЛЬЗЯ вычислять как Xmax-Xmin.

	WORD 			palette_info;					// Палитра
													// 1 = Color/BW,
													// 2 = Grayscale (поле игнорируется в PB IV/ IV +)

	WORD 			hscreensize;					// Горизонтальный размер экрана в пикселах. Используется только в PB IV/IV Plus
	WORD 			vscreensize;					// Вертикальный размер экрана в пикселах. Используется только в PB IV/IV Plus 
	BYTE 			filler[54];						// Пустые. Дополняют заголовок до 128 байт. Установлены в 0
} *PCX_HEADER;

typedef struct _PCX_FILE_t {
	PCX_HEADER 		header;
	BYTE* 			data;
	BYTE*			palette;
} *PCX_FILE;

PCX_HEADER pcx_header_init (void);
PCX_FILE pcx_init (void);
void pcx_destroy (PCX_FILE);

void pcx_header_fill (PCX_HEADER, FILE*);
void pcx_header_print (PCX_HEADER);
void pcx_fill (PCX_FILE, FILE*);
void pcx_unpack (PCX_FILE, FILE*);
PCX_FILE pcx_fill_wrapper (char*);

BYTE* pcx_compress (BYTE*, DWORD, DWORD*);
// BYTE* pcx_decompress (BYTE*, DWORD, DWORD*);

RAW_RGB pcx_to_raw (PCX_FILE);
PCX_FILE raw_to_pcx (RAW_RGB, BYTE*);

void pcx_write (PCX_FILE, char*, BYTE);
