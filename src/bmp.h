#ifndef DEFS_H
#define DEFS_H
#include "defs.h"
#endif

/* BITMAPFILEHEADER */
typedef struct _BMP_HEADER_t { 
    BYTE                type[2];        // сигнатура, для Windows = "BM"
    DWORD               size;           // размер файла в байтах
    WORD                reserved_1; 
    WORD                reserved_2; 
    DWORD               offset;         // смещение графических данных относительно начала файла
} *BMP_HEADER;

/* BITMAPINFOHEADER */
typedef struct _BMP_INFO_t {
    DWORD               size;           // размер информационного заголовка в байтах
    LONG                width;          // ширина изображения
    LONG                height;         // высота изображения
    WORD                n_planes;       // количество цветовых плоскостей = 1
    WORD                bits_per_pixel; // количество битов на пиксель

    DWORD               compression;    // используемый метод компрессии
                                        // 0 - никакой компрессии (BI_RGB)
                                        // 1 - RLE 8 бит на пиксель, только для 8bpp-изображений (BI_RLE8)
                                        // 2 - RLE 4 бит на пиксель, только для 4bpp-изображений (BI_RLE4)
                                        // ... (см. вики)

    DWORD               image_size;     // размер изображения (графических данных)
    LONG                hdpm;           // вертикальное разрешение изображения (точек на метр)
    LONG                vdpm;           // горизонтальное разрешение изображения (точек на метр)
    DWORD               clr_palette;    // количество цветов в палитре
    DWORD               clr_used;       // количество реально используемых цветов (0, если используются все цвета), обычно игнорируется
} *BMP_INFO;

typedef struct _BMP_FILE_t {
    BMP_HEADER          header;
    BMP_INFO            info;
    DWORD*              palette;
    BYTE*               data;
} *BMP_FILE;

BMP_HEADER bmp_header_init (void);
BMP_INFO bmp_info_init (void);
BMP_FILE bmp_init (void);
void bmp_destroy (BMP_FILE);

void bmp_header_fill (BMP_HEADER, FILE*);
void bmp_header_print (BMP_HEADER);
void bmp_info_fill (BMP_INFO, FILE*);
void bmp_info_print (BMP_INFO);
void bmp_fill (BMP_FILE, FILE*);
void bmp_unpack (BMP_FILE, FILE*);
BMP_FILE bmp_fill_wrapper (char*);

void bmp_write (BMP_FILE, char*);

RAW_RGB bmp_to_raw (BMP_FILE);
BMP_FILE raw_to_bmp (RAW_RGB);