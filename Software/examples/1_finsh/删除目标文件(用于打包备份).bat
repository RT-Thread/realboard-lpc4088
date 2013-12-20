del /Q build
del /Q *.dblite
del /Q *.Administrator
del /Q project.uvopt
del /Q project.uvproj
del /Q *.dep
del /Q *.bak
del /Q *.bin
@echo off
for /f "delims=" %%a in ('dir . /b /ad /s ^|sort /r' ) do rd /q "%%a" 2>nul


