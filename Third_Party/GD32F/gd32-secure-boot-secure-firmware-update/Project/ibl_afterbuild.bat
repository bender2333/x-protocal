@echo off

set OUTPUT_PATH=%1
set OUTPUT_NAME=%2
set TOOLKIT=%3
set TOOLKIT_DIR=%4

set "OUTPUT_PATH=%OUTPUT_PATH:/=\%"

set ROOT=%OUTPUT_PATH%\..\..\..\..\..\..

set SREC_CAT=%ROOT%\scripts\imgtool\srec_cat.exe
set OUTPUT_HEX_PATH=%ROOT%\scripts\images\
set OUTPUT_FILE=%OUTPUT_PATH%\..\%OUTPUT_NAME%

cd ../../../../scripts/symbol_tool/dist
symbol_generate.exe symbol_gen -m %OUTPUT_PATH%/../Listings/ibl.map -o ../../../Export/symbol/ibl_symbol_mbedtls

if "%TOOLKIT%" == "KEIL" (
    :: Generate txt for debug
    %TOOLKIT_DIR%\ARM\ARMCC\bin\fromelf.exe --text -c -d --output=%OUTPUT_FILE%.txt %OUTPUT_PATH%\%OUTPUT_NAME%.axf

    :: Generate binary image
    %TOOLKIT_DIR%\ARM\ARMCC\bin\fromelf.exe --bin --8x1  --bincombined --output=%OUTPUT_FILE%.bin %OUTPUT_PATH%\%OUTPUT_NAME%.axf
)

IF EXIST %OUTPUT_HEX_PATH%%OUTPUT_FILE%.bin    del %OUTPUT_HEX_PATH%%OUTPUT_FILE%.bin
copy %OUTPUT_FILE%.bin %OUTPUT_HEX_PATH%ibl.bin
