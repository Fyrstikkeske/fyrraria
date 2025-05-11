CC = gcc
CFLAGS = -std=c23 -Wall -g -Iclient/libraries
LDFLAGS = -lGL -lX11 -lXrandr -lm
SRC = client/main.c
OBJ = $(SRC:.c=.o)
EXE = fyrraria

all: $(EXE)

release: CFLAGS += -O3
release: clean $(EXE) 
	@echo -e " \033[0;32m ### STARING RELEASE BUILD\033[0m \n"
	./$(EXE)

small: CFLAGS = -Os -s -Iclient/libraries
small: LDFLAGS += -s
small: clean $(EXE)
	@strip $(EXE)
	@echo -e " \033[0;34m ### SMALL OPTIMIZED BUILD COMPLETE \033[0m \n"


$(EXE): $(OBJ)
	$(CC) $(OBJ) -o $(EXE) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXE)

run: $(EXE)
	@echo -e " \033[0;32m ### Compilation is finished \033[0m \n"
	./$(EXE)
