TARGET_NAME:=main
OBJ_DIR=obj

CC=gcc

SRC_FILES=$(wildcard *.c)

OBJ:=$(foreach obj, $(SRC_FILES), $(OBJ_DIR)/$(obj).o)


.PHONY: default clean

default: $(TARGET_NAME)
	@echo "DONE."


$(OBJ_DIR)/%.c.o: %.c
	@echo "Compiling $(notdir $<)"
	@mkdir -p $(shell dirname $@)
	@$(CC) -c $< -o $@


$(TARGET_NAME): $(OBJ)
	@echo "Link $(notdir $@)"
	@$(CC) $(OBJ) -o $@

clean:
	rm -r $(TARGET_NAME) $(OBJ_DIR)
