server: main.o client.o user.o file.o transfer.o
	g++ -o server main.o client.o user.o file.o transfer.o

main.o: client.h transfer.h file.h main.cpp

client.o: client.h user.h transfer.h file.h client.cpp

user.o: client.h user.h file.h user.cpp

file.o: transfer.h file.h file.cpp

transfer.o: transfer.h transfer.cpp

.PHONY: clean
clean:
	@rm -f server main.o client.o user.o file.o transfer.o
	@echo Cleaned.
