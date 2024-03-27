# Definicje zmiennych
CC = gcc
CFLAGS = -I./libs -Wall
LDFLAGS = -L./libs/libaloneg_utils
ASAN_FLAGS = -fsanitize=address -fno-omit-frame-pointer -Wno-format-security
ASAN_LIBS  = -static-libasan -lasan
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
TARGET = a.out

# Reguła domyślna
all: $(TARGET)

# Reguła dla celu końcowego
$(TARGET): $(OBJS)
	$(CC) -g -o $@ $^ $(LDFLAGS) $(ASAN_LIBS)

# Reguła dla obiektów
%.o: %.c
	$(CC) -g -c $(CFLAGS) $(ASAN_FLAGS) $(ASAN_LIBS) $< -o $@


# Reguła debug
#debug: CFLAGS += -g
#debug: ASAN_FLAGS =
#debug: ASAN_LIBS =
#debug: $(TARGET)

# Reguła release
release: CFLAGS += -O2
release: ASAN_FLAGS =
release: ASAN_LIBS =
release: $(TARGET)

# Reguła czyszczenia
clean:
	rm -f $(OBJS) $(TARGET)
