# Directories
SRC := src
INCLUDE := include

# Compile Flags
CC := g++
CFLAGS := -I$(INCLUDE) -Wall -Wextra -Werror -Ofast -funroll-loops -march=native -std=c++2a
LDFLAGS := -flto
TARGET := neh

all: clean $(TARGET)

$(TARGET): main.o 
	$(CC) $(LDFLAGS) $^ -o $@

main.o:
	$(CC) $(CFLAGS) -c $(SRC)/main.cpp -o $@

clean:
	rm -rf *.o
	rm -rf $(TARGET)