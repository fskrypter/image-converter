#ifndef DEFS_H
#define DEFS_H
#include "defs.h"
#endif

/* BITMAPFILEHEADER */
typedef struct _BMP_HEADER_t { 
    BYTE                type[2];        // ���������, ��� Windows = "BM"
    DWORD               size;           // ������ ����� � ������
    WORD                reserved_1; 
    WORD                reserved_2; 
    DWORD               offset;         // �������� ����������� ������ ������������ ������ �����
} *BMP_HEADER;

/* BITMAPINFOHEADER */
typedef struct _BMP_INFO_t {
    DWORD               size;           // ������ ��������������� ��������� � ������
    LONG                width;          // ������ �����������
    LONG                height;         // ������ �����������
    WORD                n_planes;       // ���������� �������� ���������� = 1
    WORD                bits_per_pixel; // ���������� ����� �� �������

    DWORD               compression;    // ������������ ����� ����������
                                        // 0 - ������� ���������� (BI_RGB)
                                        // 1 - RLE 8 ��� �� �������, ������ ��� 8bpp-����������� (BI_RLE8)
                                        // 2 - RLE 4 ��� �� �������, ������ ��� 4bpp-����������� (BI_RLE4)
                                        // ... (��. ����)

    DWORD               image_size;     // ������ ����������� (����������� ������)
    LONG                hdpm;           // ������������ ���������� ����������� (����� �� ����)
    LONG                vdpm;           // �������������� ���������� ����������� (����� �� ����)
    DWORD               clr_palette;    // ���������� ������ � �������
    DWORD               clr_used;       // ���������� ������� ������������ ������ (0, ���� ������������ ��� �����), ������ ������������
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