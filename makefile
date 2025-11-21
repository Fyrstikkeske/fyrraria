CC       := gcc
CFLAGS   := -std=c23 -Wall -g -Iclient/libraries -Iclient/include
LDFLAGS  := -lGL -lX11 -lXrandr -lm

SRC_DIR  := client/code
BUILD_DIR := build
EXE      := fyrraria

SRC      := main.c meshing.c worldgen.c utils.c shader.c 
OBJ      := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

all: $(EXE)

release: CFLAGS += -O3
release: clean $(EXE)
	@echo -e " \033[0;32m ### STARTING RELEASE BUILD\033[0m \n"
	./$(EXE)

small: CFLAGS = -Oz -s -flto -fdata-sections -ffunction-sections \
                 -fno-ident -fno-asynchronous-unwind-tables \
                 -fno-unwind-tables -fno-exceptions -fno-rtti \
                 -Iclient/libraries
small: LDFLAGS += -Wl,--gc-sections -flto
small: clean $(EXE)
	@strip --strip-all $(EXE)
	@echo -e " \033[0;34m ### SMALL OPTIMIZED BUILD COMPLETE \033[0m \n"

safe: CFLAGS = -std=c23 -Wall -Wextra -Wpedantic -Wshadow -Wcast-align \
               -Wstrict-prototypes -Wmissing-prototypes -Wuninitialized \
               -Wconversion -Wsign-conversion -Wno-unused-parameter \
               -fsanitize=undefined -fsanitize=address -fno-common -g -O0 \
               -isystem client/libraries -isystem client/include
safe: LDFLAGS = -lGL -lX11 -lXrandr -lm -fsanitize=undefined -fsanitize=address
safe: clean $(EXE)
	@echo -e " \033[0;33m ### SAFE BUILD COMPLETE \033[0m \n"
	./$(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(EXE)

bear: clean
	bear -- make

run: clean $(EXE)
	@echo -e " \033[0;32m ### Compilation is finished \033[0m \n"
	./$(EXE)

fast: $(EXE)
	@echo -e " \033[0;32m ### Fast Compilation finished\033[0m \n"
	./$(EXE)

.PHONY: all release small safe clean bear run fast