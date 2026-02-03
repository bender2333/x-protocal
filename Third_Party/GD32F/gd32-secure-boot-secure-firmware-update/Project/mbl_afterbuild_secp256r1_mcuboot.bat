@echo off

set SIP=%1
set AESK=%~2
set DEBUGGER=%3
set OUTPUT_PATH=%4
set OUTPUT_NAME=%5
set TOOLKIT=%6
set TOOLKIT_DIR=%7

set "OUTPUT_PATH=%OUTPUT_PATH:/=\%"

set ROOT=%OUTPUT_PATH%\..\..\..\..\..\..

set KEY_PASSPHRASE="12345678"

set ROTPK=%ROOT%\scripts\certs\ec_secp256r1_prikey.pem
set MBL_CERT=%ROOT%\scripts\certs\mbl-cert.pem
set CONFIG_FILE=%ROOT%\config\config_gdm32.h
@REM set SYSTOOL=%ROOT%\scripts\imgtool\dist\sysset.exe
@REM set IMGTOOL=%ROOT%\scripts\imgtool\dist\imgtool.exe
set HEXTOOL=%ROOT%\scripts\imgtool\dist\hextool.exe
set GENTOOL=%ROOT%\scripts\imgtool\dist\gentool.exe
set SREC_CAT=%ROOT%\scripts\imgtool\srec_cat.exe
set OUTPUT_HEX_PATH=%ROOT%\scripts\images\
set OUTPUT_FILE=%OUTPUT_PATH%\..\%OUTPUT_NAME%

set SYSTOOL=%ROOT%\scripts\scripts_mcuboot\sysset.py
set IMGTOOL=%ROOT%\scripts\scripts_mcuboot\imgtool.py
set MBLSYMBOLTOOL=%ROOT%\scripts\symbol_tool\symbol_generate_name_mbl.py

set ALGO_HASH=SHA256
set ALGO_SIGN=ECDSA256

:: Save Orignal Working Path
set WORK_PATH=%CD%

if "%SIP%" == "SIP" (
    set ALGO=GD32W51x_S
) else (
    set ALGO=GD32W51x_Q_S
)

if "%AESK%" NEQ "" (
    set AES_SUFFIX=-aes
) else (
    set AES_SUFFIX=
)

:: Generate system setting hex
python "%SYSTOOL%" -t "SYS_SET" -c %CONFIG_FILE%  %OUTPUT_HEX_PATH%\sysset.bin

if "%TOOLKIT%" == "KEIL" (
    :: Generate txt for debug
    %TOOLKIT_DIR%\ARM\ARMCC\bin\fromelf.exe --text -c -d --output=%OUTPUT_FILE%.txt %OUTPUT_PATH%\%OUTPUT_NAME%.axf

    :: Generate binary image
    %TOOLKIT_DIR%\ARM\ARMCC\bin\fromelf.exe --bin --8x1  --bincombined --output=%OUTPUT_FILE%.bin %OUTPUT_PATH%\%OUTPUT_NAME%.axf
)
if "%TOOLKIT%" == "IAR" (
    :: Generate ASM file
    %TOOLKIT_DIR%\bin\ielfdumparm.exe %OUTPUT_PATH%\%OUTPUT_NAME%.axf  --output %OUTPUT_FILE%.asm --code

    :: Generate binary image
    %TOOLKIT_DIR%\bin\ielftool.exe %OUTPUT_PATH%\%OUTPUT_NAME%.axf  --bin %OUTPUT_FILE%.bin
)

if "%TOOLKIT%" == "GCC" (
    arm-none-eabi-objdump -xS  %OUTPUT_PATH%/%OUTPUT_NAME%.axf > %OUTPUT_FILE%.txt
    arm-none-eabi-objcopy -Obinary %OUTPUT_PATH%/%OUTPUT_NAME%.axf %OUTPUT_FILE%.bin
)

if exist %OUTPUT_HEX_PATH%\mbl-sign.bin  del %OUTPUT_HEX_PATH%\mbl-sign.bin
:: Add image header, ptlvs and concatenate the cert
@REM %IMGTOOL% sign --config %CONFIG_FILE% ^
@REM                       -k %ROTPK% ^
@REM                       -P %KEY_PASSPHRASE% ^
@REM                       -t "MBL" ^
@REM                       --cert %MBL_CERT% ^
@REM                       --algo_hash "%ALGO_HASH%" ^
@REM                       --algo_sig "%ALGO_SIGN%" ^
@REM                       %OUTPUT_FILE%.bin %OUTPUT_HEX_PATH%\mbl-sign.bin

@REM @REM python %GENTOOL% --config %CONFIG_FILE% ^
@REM %GENTOOL% --config %CONFIG_FILE% ^
@REM                  --sys_set %OUTPUT_HEX_PATH%\sysset.bin ^
@REM                  --mbl %OUTPUT_HEX_PATH%\mbl-sign.bin ^
@REM                  -o %OUTPUT_HEX_PATH%\mbl-sys.bin
@REM IF EXIST %OUTPUT_HEX_PATH%\sysset.bin del %OUTPUT_HEX_PATH%\sysset.bin
@REM @REM IF EXIST %OUTPUT_HEX_PATH%\mbl-sign.bin del %OUTPUT_HEX_PATH%\mbl-sign.bin

@REM if "%AESK%" == ""  (
@REM     %HEXTOOL% -c %CONFIG_FILE% ^
@REM             -t "SYS_SET" ^
@REM             -e %SREC_CAT% ^
@REM             %OUTPUT_HEX_PATH%\mbl-sys.bin ^
@REM             %OUTPUT_HEX_PATH%\mbl-sys.hex
@REM )  else (
@REM     python %IMGTOOL% pad -s 0x8000 ^
@REM                          %OUTPUT_HEX_PATH%\mbl-sys.bin %OUTPUT_HEX_PATH%\mbl-sys-pad.bin
@REM     python %AESTOOL% --c %CONFIG_FILE%   ^
@REM             -t "SYS_SET" ^
@REM             -i %OUTPUT_HEX_PATH%\mbl-sys-pad.bin ^
@REM             -o %OUTPUT_HEX_PATH%\mbl-sys%AES_SUFFIX%.bin ^
@REM             -k %AESK%
@REM     python %HEXTOOL% -c %CONFIG_FILE% ^
@REM             -t "SYS_SET" ^
@REM             -e %SREC_CAT% ^
@REM             %OUTPUT_HEX_PATH%\mbl-sys%AES_SUFFIX%.bin ^
@REM             %OUTPUT_HEX_PATH%\mbl-sys.hex
@REM     del %OUTPUT_HEX_PATH%\mbl-sys-pad.bin
@REM     echo Encrypted!
@REM )

@REM copy %OUTPUT_HEX_PATH%\mbl-sys.hex %OUTPUT_PATH%\mbl.hex

:: Add image header, ptlvs and concatenate the cert
python %IMGTOOL% sign --align 4 -v "1.0.5" ^
                      -k %ROTPK% ^
                      -H 0x200 --pad-header ^
                      -S 0x7000 --public-key-format hash --max-align 8 --pad-sig ^
                      %OUTPUT_FILE%.bin %OUTPUT_HEX_PATH%\mbl-sign.bin

@REM python %GENTOOL% --config %CONFIG_FILE% ^
%GENTOOL% --config %CONFIG_FILE% ^
                 --sys_set %OUTPUT_HEX_PATH%\sysset.bin ^
                 --mbl %OUTPUT_HEX_PATH%\mbl-sign.bin ^
                 -o %OUTPUT_HEX_PATH%\mbl-sys.bin
IF EXIST %OUTPUT_HEX_PATH%\sysset.bin del %OUTPUT_HEX_PATH%\sysset.bin
@REM IF EXIST %OUTPUT_HEX_PATH%\mbl-sign.bin del %OUTPUT_HEX_PATH%\mbl-sign.bin

if "%AESK%" == ""  (
    %HEXTOOL% -c %CONFIG_FILE% ^
            -t "SYS_SET" ^
            -e %SREC_CAT% ^
            %OUTPUT_HEX_PATH%\mbl-sys.bin ^
            %OUTPUT_HEX_PATH%\mbl-sys.hex
)  else (
    python %IMGTOOL% pad -s 0x8000 ^
                         %OUTPUT_HEX_PATH%\mbl-sys.bin %OUTPUT_HEX_PATH%\mbl-sys-pad.bin
    python %AESTOOL% --c %CONFIG_FILE%   ^
            -t "SYS_SET" ^
            -i %OUTPUT_HEX_PATH%\mbl-sys-pad.bin ^
            -o %OUTPUT_HEX_PATH%\mbl-sys%AES_SUFFIX%.bin ^
            -k %AESK%
    python %HEXTOOL% -c %CONFIG_FILE% ^
            -t "SYS_SET" ^
            -e %SREC_CAT% ^
            %OUTPUT_HEX_PATH%\mbl-sys%AES_SUFFIX%.bin ^
            %OUTPUT_HEX_PATH%\mbl-sys.hex
    del %OUTPUT_HEX_PATH%\mbl-sys-pad.bin
    echo Encrypted!
)

copy %OUTPUT_HEX_PATH%\mbl-sys.hex %OUTPUT_PATH%\mbl.hex

python %MBLSYMBOLTOOL% symbol_gen -m %OUTPUT_PATH%/../Listings/mbl.map -o %ROOT%/Export/symbol/mbl_symbol
