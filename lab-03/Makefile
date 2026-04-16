CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
TARGET = prog.bin
OBJS = main.o

.PHONY: all run clean

all: $(TARGET)

# Линковка: склеиваем .o файлы в один бинарный
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# Правило для компиляции каждого .c в .o
# $< — это имя .c файла, $@ — имя .o файла
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)