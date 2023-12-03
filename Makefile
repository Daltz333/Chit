# Define compiler and flags
CC = gcc
CFLAGS = -Wall -g -Wformat=2 -fsanitize=address

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

ADDRESS_SERVER_DIR = $(SRC_DIR)/address_server
PK_SERVER_DIR = $(SRC_DIR)/pk_server
CHIT_CLIENT_DIR = $(SRC_DIR)/chit_client
SHARED_DIR = $(SRC_DIR)/shared

SHARED_SRC := $(shell find $(SHARED_DIR) -name '*.c')

ADDRESS_SERVER_SRC := $(shell find $(ADDRESS_SERVER_DIR) -name '*.c') $(SHARED_SRC)
PK_SERVER_SRC := $(shell find $(PK_SERVER_DIR) -name '*.c') $(SHARED_SRC)
CHIT_CLIENT_SRC := $(shell find $(CHIT_CLIENT_DIR) -name '*.c') $(SHARED_SRC)

# Object files for each target
ADDRESS_SERVER_OBJ = $(patsubst $(ADDRESS_SERVER_DIR)/%.c, $(OBJ_DIR)/%.o, $(ADDRESS_SERVER_SRC))
PK_SERVER_OBJ = $(patsubst $(PK_SERVER_DIR)/%.c, $(OBJ_DIR)/%.o, $(PK_SERVER_SRC))
CHIT_CLIENT_OBJ = $(patsubst $(CHIT_CLIENT_DIR)/%.c, $(OBJ_DIR)/%.o, $(CHIT_CLIENT_SRC)) 

# Targets
all: address_server chit_client pk_server

address_server: $(ADDRESS_SERVER_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ -lm $^

chit_client: $(CHIT_CLIENT_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ -lm $^

pk_server: $(PK_SERVER_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ -lm $^

$(OBJ_DIR)/%.o: $(ADDRESS_SERVER_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(AUTH_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(PK_SERVER_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(CHIT_CLIENT_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)

.PHONY: all auth pk_server address_server chit_client clean
