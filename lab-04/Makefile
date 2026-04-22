CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
TARGET = prog.bin
# Заменяем algoritm.o на btree.o
OBJS = main.o btree.o
# Указываем заголовочный файл как зависимость
DEPS = btree.h

.PHONY: all run clean

all: $(TARGET)

# Линковка
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Компиляция .c в .o
# Добавляем $(DEPS) в зависимости, чтобы изменение в .h вызывало пересборку
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) output.txt