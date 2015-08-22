@echo off
if not exist %PROJECT%\build mkdir %PROJECT%\build
pushd %PROJECT%\build 
set COMMON_FILES=
set TEST_COMMON_FILES=
set FLAGS=-nologo -DSDL_ASSERT_LEVEL=2 -Oi -GR- -Gm- -MP -EHac -WX -W4 /FC /Zi /MD /wd4100 /wd4201 /wd4127 /wd4244 /wd4996 /wd4189 /wd4702 /wd4512
set INCLUDE_FILES=-I%PROJECT%\include -IC:\game_dev\3rd\sdl\dev\SDL2-2.0.3\include -IC:\game_dev\3rd\sdl\dev\SDL2_ttf-2.0.12\include -IC:\game_dev\3rd\sdl\dev\SDL2_mixer-2.0.0\include
REM set LIBRARY_FILES=C:\game_dev\3rd\sdl\dev\sdl2-2.0.3\lib\x64\sdl2.lib C:\game_dev\3rd\sdl\dev\sdl2-2.0.3\lib\x64\sdl2main.lib C:\game_dev\3rd\sdl\dev\sdl2_image-2.0.0\lib\x64\sdl2_image.lib C:\game_dev\3rd\glew-1.12.0\lib\release\x64\glew32.lib "C:\program files (x86)\microsoft sdks\windows\v7.1a\lib\x64\glu32.lib" "C:\program files (x86)\microsoft sdks\windows\v7.1a\lib\x64\opengl32.lib" "C:\game_dev\3rd\freetype\freetype-2.5.5\cmake-build\Release\freetype.lib" "C:\game_dev\3rd\SQUIRREL3\x64\Release\squirrel.lib" "C:\game_dev\3rd\SQUIRREL3\x64\Release\sqstdlib.lib"
set LIBRARY_FILES=C:\game_dev\3rd\sdl\dev\sdl2-2.0.3\lib\x64\sdl2.lib C:\game_dev\3rd\sdl\dev\sdl2-2.0.3\lib\x64\sdl2main.lib C:\game_dev\3rd\sdl\dev\SDL2_ttf-2.0.12\lib\x64\SDL2_ttf.lib C:\game_dev\3rd\sdl\dev\SDL2_mixer-2.0.0\lib\x64\SDL2_mixer.lib
cl %FLAGS% %INCLUDE_FILES% ../src/template.cpp %COMMON_FILES% %LIBRARY_FILES% 
popd
