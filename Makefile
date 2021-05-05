SERVER=src/server.c

CLIENT=src/client.c

COMPILER=gcc

FLAGS=-Wall


build/client: $(CLIENT)
	$(COMPILER) -o $@ $< $(FLAGS)
build/server: $(SERVER)
	$(COMPILER) -o $@ $< $(FLAGS)
clean: 
	rm build/client build/server 
