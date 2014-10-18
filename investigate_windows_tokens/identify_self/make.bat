rem HOLY SHIT when using cpp libs and headers (such as msi.h in this case
rem our actual file name (ident_self) has to have the .cpp extension
rem or cl.exe parses it differently or something... if named .c it doesnt compile
rem when named .cpp it does.

cl /nologo /Yd /Z7 /IC:\WINDDK\2600.1106\inc\w2k ident_self.cpp /c
link /nologo advapi32.lib msi.lib user32.lib /out:ident_self.exe ident_self.obj
