# Nome dell'eseguibile
TARGET = frogger

# Cartelle
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# File sorgenti
SRCS = $(wildcard $(SRC_DIR)/*.c)

# File oggetto
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS)) $(BUILD_DIR)/sprites.o

# Compilatore e flags
CC = gcc
CFLAGS = -Wall -Wextra -I$(INCLUDE_DIR)
LDFLAGS = -lncurses -lm

# Regola di default
all: $(BUILD_DIR) $(TARGET)

# Creazione della cartella build
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Regola per costruire l'eseguibile
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Regola per costruire i file oggetto
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

# Pulizia dei file compilati
clean:
	@rm -rf $(BUILD_DIR) $(TARGET)

# Regole di utilità
.PHONY: all clean