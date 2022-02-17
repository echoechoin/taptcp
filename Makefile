CC := gcc

CLI := tcp_stack
C_SOURCE := $(wildcard *.c)
OBJ := $(C_SOURCE:.c=.o)
INCLUDE := -I./include

CFLAGS := -std=c99
# CFLAGS := -Wall -Wextra -Werror -std=c99

$(CLI): $(OBJ)
	$(CC)  $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -f $(OBJ) $(CLI)
