rem @echo "Making the childrunner"
rem cl child_runner.c /IC:\WINDDK\2600.1106\inc\w2k /c
rem link /out:cr.exe child_runner.obj 
cls
@echo "Making humpty-dumpty"
@echo "***I hope you have all the devshit in your ENV***"
cl /nologo /w humpty_dumpty.c /IC:\WINDDK\2600.1106\inc\w2k /c
link /nologo /libpath:C:\WINDDK\2600.1106\lib\w2k\i386\ psapi.lib /out:hd.exe humpty_dumpty.obj
