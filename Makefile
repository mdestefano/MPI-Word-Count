CC		:= mpicc
C_FLAGS := -Wall -Wextra

BIN		:= bin
SRC		:= src
INCLUDE	:= include
LIB		:= lib

LIBRARIES	:=

ifeq ($(OS),Windows_NT)
EXECUTABLE	:= main.exe
else
EXECUTABLE	:= main
endif

NP := 2
DBG := 

all: $(BIN)/$(EXECUTABLE)

clean:
	-$(RM) $(BIN)/$(EXECUTABLE)

run: all
	mpirun -np $(NP) ./$(BIN)/$(EXECUTABLE) input/index.txt

$(BIN)/$(EXECUTABLE): $(SRC)/*
	$(CC) $(DBG) $(C_FLAGS) -I$(INCLUDE) -L$(LIB) $^ -o $@ $(LIBRARIES)