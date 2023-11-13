CC = gcc

CFLAGS = -Wall

SRC = my_tar.c utils.c create_archive.c append_archive.c list_archive.c update_archive.c extract_archive.c

HEADERS = my_tar.h

TARGET = my_tar

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

fclean: clean
	rm -f $(TARGET)

.PHONY: all clean
