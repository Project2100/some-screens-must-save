.PHONY: debug release run clean


debug: resource.res vertex.h pixel.h
	cl /nologo /W3 /std:c17 /c /DDEBUG debug.c dynamenger.c graphics.c main.c
	link /nologo /OUT:ssms.scr /DEF:ssms.def /subsystem:windows user32.lib comctl32.lib Advapi32.lib gdi32.lib ScrnSavW.lib resource.res debug.obj dynamenger.obj graphics.obj main.obj

release: resource.res vertex.h pixel.h
	cl /nologo /W3 /std:c17 /c dynamenger.c graphics.c main.c
	link /nologo /OUT:ssms.scr /DEF:ssms.def /subsystem:windows user32.lib comctl32.lib Advapi32.lib gdi32.lib ScrnSavW.lib resource.res dynamenger.obj graphics.obj main.obj

run:
	ssms.scr /s

config:
	ssms.scr /c

clean:
	del /Q ssms.scr *.res *.obj *.pdb *.log vertex.h pixel.h

resource.res: resource.rc controls.h
	rc /nologo /n resource.rc

vertex.h: vertex.hlsl
	fxc /T:vs_5_0 /Fh:vertex.h /E:VShader /nologo vertex.hlsl
	
pixel.h: pixel.hlsl
	fxc /T:ps_5_0 /Fh:pixel.h /E:PShader /nologo pixel.hlsl
