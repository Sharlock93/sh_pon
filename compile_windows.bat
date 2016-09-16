@echo off
pushd buildwin

del *.pdb 2> NUL
call "C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/vcvarsall.bat" x64

set CommonLinkers=/INCREMENTAL:NO /OPT:REF 
set CommonCompilerFlags=/MT /nologo /Gm- /GR- /EHsc- /EHa- /Od /Oi /W4 /wd4505 /wd4305 /wd4244 /wd4456 /wd4201 /wd4100 /wd4189 /wd4310 /wd4245 /FC /Zi

set Libs=opengl32.lib shar.lib sh_circle.lib sh_rect.lib sh_line.lib sharfunc.lib sharinit.lib glfw3dll.lib glew32.lib user32.lib

set TIME_STAMP=%date:~10,4%%date:~4,2%%date:~7,2%_%time:~3,2%%time:~6,2%%time:~9,2%

cl %CommonCompilerFlags% ..\src\game_logic.cpp %Libs% /LD /link /PDB:game_logic%TIME_STAMP%.pdb %CommonLinkers%

cl %CommonCompilerFlags% ..\src\main.cpp %Libs% /link %CommonLinkers% 

popd

