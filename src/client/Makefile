client: main.o recvfile.o sendfile.o msg.o readline.o
	g++ -o client -lreadline main.o recvfile.o sendfile.o msg.o readline.o

main.o: file.h msg.h recvfile.h sendfile.h main.cpp

recvfile.o: file.h msg.h recvfile.h recvfile.cpp

sendfile.o: file.h msg.h sendfile.h sendfile.cpp

msg.o: msg.h msg.cpp

readline.o: readline.cpp

.PHONY: clean
clean:
	@rm -f client
	@echo Cleaned
