.PHONY: all run clean

CC       = clang
CSFLAGS  = -Wall -Wpedantic -Wextra -Wshadow -Wformat=2 -Wundef -std=c17

all: #resource.res
	$(CC) $(CSFLAGS) -o ssms.scr resource.res main.c

run:
	ssms.scr /s

clean:
	rm ssms.scr resource.res application.log d3d.log

# resource.res: resource.rc
# 	rc resource.rc
