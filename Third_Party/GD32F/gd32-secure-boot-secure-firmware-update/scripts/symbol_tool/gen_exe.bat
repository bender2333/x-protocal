cd %~dp0

pyinstaller -F -w symbol_generate_name.py
rd /s /q build
cd dist
IF EXIST symbol_generate.exe    del symbol_generate.exe
ren symbol_generate_name.exe symbol_generate.exe
del symbol_generate_name.exe
