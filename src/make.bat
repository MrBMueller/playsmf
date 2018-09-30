
echo off

del .\*.obj ..\bin\playsmf.exe

cl /nologo /I. /c .\midifile.c
cl /nologo /I. /c .\playsmf.c
cl /nologo /Fe..\bin\playsmf.exe .\playsmf.obj .\midifile.obj winmm.lib

del .\*.obj
