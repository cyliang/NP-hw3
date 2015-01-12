client: main.o recvfile.o sendfile.o msg.o
	g++ -o client -lreadline main.o recvfile.o sendfile.o msg.o

main.o: file.h msg.h recvfile.h sendfile.h main.cpp

recvfile.o: file.h msg.h recvfile.h recvfile.cpp

sendfile.o: file.h msg.h sendfile.h sendfile.cpp

msg.o: msg.h msg.cpp

.PHONY: clean
clean:
	@rm -f client
	@echo Cleaned
