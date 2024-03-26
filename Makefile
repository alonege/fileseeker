# Definicje zmiennych
CC = gcc
CFLAGS = -I./libs -Wall
LDFLAGS = -L./libs -lal_lib
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer -Wno-format-security
ASAN_LIBS  = -static-libasan
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
TARGET = program

# Reguła domyślna
all: $(TARGET)

# Reguła dla celu końcowego
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(ASAN_LIBS)

# Reguła dla obiektów
%.o: %.c
	$(CC) -c $(CFLAGS) $(ASAN_FLAGS) $< -o $@

# Reguła czyszczenia
clean:
	rm -f $(OBJS) $(TARGET)
