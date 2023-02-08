@echo off
set BAT_BASE_PATH=%~dp0
set PRJ_BASE_PATH=%BAT_BASE_PATH%\Client
set BIN_PATH=%PRJ_BASE_PATH%\bin
set PRJ_PATH=%PRJ_BASE_PATH%

RMDIR /s /q %BIN_PATH%
MD "%BIN_PATH%"
clang ^
    -I"%PRJ_PATH%\include" ^
    -l ws2_32 ^
    -o "%BIN_PATH%\Client.exe" "%PRJ_PATH%\src\*.c"

::    -I"%PRJ_PATH%\libraries\sdl2\include" ^
::    -I"%PRJ_PATH%\libraries\sdl2_image\include" ^
::    -L"%PRJ_PATH%\libraries\sdl2\lib\x64" ^
::    -L"%PRJ_PATH%\libraries\sdl2_image\lib\x64" ^
::    -lSDL2 ^
::    -lSDL2_image ^

::IF %ERRORLEVEL% EQU 0 (
::    COPY /Y "%PRJ_PATH%\libraries\sdl2\lib\x64\*.dll" "%BIN_PATH%"
::    COPY /Y "%PRJ_PATH%\libraries\sdl2_image\lib\x64\*.dll" "%BIN_PATH%"
::    XCOPY /E /Q /Y "%PRJ_PATH%\resources\" "%BIN_PATH%\resources\"
::)