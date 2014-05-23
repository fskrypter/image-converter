#ifndef DEFAULT_TYPEDEFS
#define DEFAULT_TYPEDEFS
typedef unsigned char 	BYTE;
typedef unsigned short 	WORD;
typedef unsigned long 	DWORD;
typedef signed int 	LONG;
#endif

extern BYTE _v;

#define verbose if(_v)

/* Структура несжатого 24-битного изображения */
typedef struct _RAW_RGB_t {
	DWORD	x_min, y_min;
	DWORD	x_max, y_max;
	BYTE**	data;									// массив байтов
} *RAW_RGB;

RAW_RGB raw_init (void);
void raw_destroy (RAW_RGB);
void raw_copy (RAW_RGB, RAW_RGB);

RAW_RGB* raw_divide (RAW_RGB, DWORD, DWORD, BYTE*);	// разбивка сырого изображения по вертикали и горизонтали
RAW_RGB raw_glue (RAW_RGB, RAW_RGB);
RAW_RGB raw_glue_array (RAW_RGB*, BYTE);
void raw_data_alloc (RAW_RGB);
void raw_data_blank (RAW_RGB);

RAW_RGB* raw_divide_by_pcx_4bit_size (RAW_RGB, DWORD, BYTE*);