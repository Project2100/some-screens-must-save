.PHONY: all clean

CC       = clang
CSFLAGS  = -Wall -Wpedantic -Wextra -Wshadow -Wformat=2 -Wundef -std=c17

all:
	$(CC) $(CSFLAGS) -o ssms.exe main.c

clean:
	rm ssms.exe application.log d3d.log
