client.o: client.h user.h client.cpp

user.o: client.h user.h user.cpp

file.o: transfer.h file.h file.cpp

transfer.o: transfer.h transfer.cpp

.PHONY: clean
clean:
	@rm -f client.o user.o file.o transfer.o
	@echo Cleaned.
