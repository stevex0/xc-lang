COMPILER := g++
OPTIONS := -std=c++17 -O3 -Wall -Wextra -pedantic -Wpedantic

SRC_DIR := ../src
OBJ_DIR := ./obj

BIN_DIR := ./bin
BINARY := $(BIN_DIR)/xc

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))

all: init clean build run

init:
	@ mkdir -p $(OBJ_DIR) $(BIN_DIR)

clean:
	@ rm -f $(OBJ_DIR)/* $(BIN_DIR)/*

build: $(BINARY)

$(BINARY): $(OBJ_FILES)
	@ $(COMPILER) $(OBJ_DIR)/*.o -static-libstdc++ -o $(BINARY)
	@ echo -e " -> " $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@ $(COMPILER) $(OPTIONS) -c $^ -o $@
	@ echo -e " -> " $@

run: $(BINARY)
ifdef test-args
ifeq ($(test-args), _)
	$(BINARY)
else
	$(BINARY) $(test-args)
endif
endif

.PHONY: all init clean build run