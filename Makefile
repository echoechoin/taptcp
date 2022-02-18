CC := gcc

SERVER := taptcp_server
CLIENT := taptcp_client

C_SOURCE := $(filter-out main.c server.c client.c,  $(wildcard *.c))
OBJ := $(C_SOURCE:.c=.o)
OBJ_ALL := $(OBJ) main_server.o main_client.o server.o client.o
INCLUDE := -I./include

C_FLAGS := -std=c99
# C_FLAGS := -Wall -Wextra -Werror -std=c99

LD_FLAGS := -levent -levent_core -lpthread

all: $(SERVER) $(CLIENT)

$(SERVER): $(OBJ) main_server.o server.o
	$(CC) -o $@ $^ $(LD_FLAGS)
$(CLIENT): $(OBJ) main_client.o client.o
	$(CC) -o $@ $^ $(LD_FLAGS)
main_server.o: main.c
	$(CC) $(C_FLAGS) $(INCLUDE) -c -o $@ $< -DTUN_SERVER
main_client.o: main.c
	$(CC) $(C_FLAGS) $(INCLUDE) -c -o $@ $< -DTUN_CLIENT
server.o: server.c
	$(CC) $(C_FLAGS) $(INCLUDE) -c -o $@ $<
client.o: client.c
	$(CC) $(C_FLAGS) $(INCLUDE) -c -o $@ $<
%.o: %.c
	$(CC) $(C_FLAGS) $(INCLUDE) -c -o $@ $<

clean:
	rm -rf $(OBJ_ALL)
	rm -rf $(CLIENT)
	rm -rf $(SERVER)
