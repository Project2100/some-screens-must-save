.PHONY: debug release run clean


debug: resource.res vertex.h pixel.h
	cl /nologo /W3 /std:c17 /c debug.c
	cl /DDEBUG /nologo /W3 /std:c17 /c dynamenger.c
	cl /DDEBUG /nologo /W3 /std:c17 /c graphics.c
	cl /DDEBUG /nologo /W3 /std:c17 /Fe:ssms.scr resource.res main.c graphics.obj dynamenger.obj debug.obj

release: resource.res vertex.h pixel.h
	cl /nologo /W3 /std:c17 /c dynamenger.c
	cl /nologo /W3 /std:c17 /c graphics.c
	cl /nologo /W3 /std:c17 /Fe:ssms.scr resource.res main.c graphics.obj dynamenger.obj

run:
	ssms.scr /s

config:
	ssms.scr /c

clean:
	del /Q ssms.scr *.res *.obj *.pdb *.log vertex.h pixel.h

resource.res: resource.rc controls.h
	rc /nologo /n resource.rc

# .c.obj:
# 	cl /DDEBUG /nologo /W3 /std:c17 /c $<

vertex.h: vertex.hlsl
	fxc /T:vs_5_0 /Fh:vertex.h /E:VShader /nologo vertex.hlsl
	
pixel.h: pixel.hlsl
	fxc /T:ps_5_0 /Fh:pixel.h /E:PShader /nologo pixel.hlsl
