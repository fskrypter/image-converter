CC			=	g++
CFLAGS		=	-O0 -Wno-format
HEADERS		=	defs.h pcx.h bmp.h
SOURCES		=	main.cpp pcx.cpp bmp.cpp raw.cpp defs.cpp
OBJECTS		=	$(SOURCES:.c=.o)
TARGET		=	pcx

all: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(CFLAGS)

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o $(TARGET)
