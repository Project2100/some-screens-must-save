.PHONY: all run clean

all: resource.res graphics.obj
	cl /nologo /W3 /std:c17 /Fe:ssms.scr resource.res main.c graphics.obj

run:
	ssms.scr /s

clean:
	rm -f ssms.scr *.res *.obj *.pdb *.log

resource.res: resource.rc
	rc /nologo resource.rc

.c.obj:
	cl /nologo /W3 /std:c17 /c $<
