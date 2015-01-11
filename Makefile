.PHONY: all clean server.exe client.exe

export
CXXFLAGS = 


all: server.exe client.exe
	@echo -e '\n' \
		'\033[1;33m*******************************\033[m\n' \
		'\033[1;33m/ Make performed successfully /\033[m\n' \
		'\033[1;33m*******************************\033[m'

server.exe:
	rm -f server.exe
	$(MAKE) -C src/server server
	ln -s src/server/server server.exe

client.exe:
	rm -f client.exe
	$(MAKE) -C src/client client
	ln -s src/client/client client.exe

clean:
	$(MAKE) -C src/server clean
	$(MAKE) -C src/client clean
	@rm -f server.exe client.exe
	@echo Cleaned
