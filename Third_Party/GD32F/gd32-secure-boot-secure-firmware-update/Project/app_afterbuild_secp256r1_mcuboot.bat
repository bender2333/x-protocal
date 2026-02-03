@echo off
set TZEN=%1
set SIP=%2
set AESK=%~3
set DEBUGGER=%4
set OUTPUT_PATH=%5
set TOOLKIT=%6
set TOOLKIT_PATH=%7
set "OUTPUT_PATH=%OUTPUT_PATH:/=\%"
echo TOOLKIT=%TOOLKIT%
echo TOOLKIT_PATH=%TOOLKIT_PATH%
echo OUTPUT_PATH=%OUTPUT_PATH%


set INDEX=0
set ROOT=%OUTPUT_PATH%\..\..\..\..\..\..
set OUTPUT_IMAGE_PATH=%ROOT%\scripts\images
set CERT_PATH=%ROOT%\scripts\certs
@REM set ROTPK=%ROOT%\scripts\certs\ec_secp256r1_prikey.pem
set CONFIG_FILE=%ROOT%\config\config_gdm32.h
set SYSTOOL=%ROOT%\scripts\imgtool\dist\sysset.exe
@REM set IMGTOOL=%ROOT%\scripts\imgtool\dist\imgtool.exe
set HEXTOOL=%ROOT%\scripts\imgtool\dist\hextool.exe
set GENTOOL=%ROOT%\scripts\imgtool\dist\gentool.exe
set SREC_CAT=%ROOT%\scripts\imgtool\srec_cat.exe
set OUTPUT_FILE=app
set KEY_PASSPHRASE="12345678"

set IMGTOOL=%ROOT%\scripts\scripts_mcuboot\imgtool.py
set ROTPK=%ROOT%\scripts\certs\ec_secp256r1_prikey.pem

set ALGO_HASH=SHA256
set ALGO_SIGN=ECDSA256

:: Save Orignal Working Path
set WORK_PATH=%CD%


if "%AESK%" NEQ "" (
    set AES_SUFFIX=-aes
) else (
    set AES_SUFFIX=
)

IF EXIST %OUTPUT_PATH%\..\app*     del %OUTPUT_PATH%\..\app*

if "%TOOLKIT%" == "KEIL" (
    :: Generate txt for debug
     %TOOLKIT_PATH%/ARM\ARMCC\bin\fromelf.exe --text -c -d --output=%OUTPUT_PATH%\..\%OUTPUT_FILE%.txt %OUTPUT_PATH%\%OUTPUT_FILE%.axf

    :: Generate binary image
     %TOOLKIT_PATH%/ARM\ARMCC\bin\fromelf.exe --bin --8x1 --bincombined --output=%OUTPUT_PATH%\..\app.bin %OUTPUT_PATH%\%OUTPUT_FILE%.axf
)
if "%TOOLKIT%" == "IAR" (
    :: Generate ASM file
    %TOOLKIT_PATH%\bin\ielfdumparm.exe %OUTPUT_PATH%\%OUTPUT_FILE%.axf  --output %OUTPUT_PATH%\..\%OUTPUT_FILE%.asm --code

    :: Generate binary image
    %TOOLKIT_PATH%\bin\ielftool.exe %OUTPUT_PATH%\%OUTPUT_FILE%.axf  --bin %OUTPUT_PATH%\..\%OUTPUT_FILE%.bin
)

if "%TOOLKIT%" == "GCC" (
    arm-none-eabi-objdump -xS  %OUTPUT_PATH%\%OUTPUT_FILE%.axf > %OUTPUT_PATH%\%OUTPUT_FILE%.txt
    arm-none-eabi-objcopy -Obinary %OUTPUT_PATH%\%OUTPUT_FILE%.axf %OUTPUT_PATH%\..\%OUTPUT_FILE%.bin
)

::echo %CONFIG_FILE%
:: Get image version
set cmd="python %IMGTOOL% printver  --config %CONFIG_FILE% -t "APP""
FOR /F "tokens=*" %%i IN (' %cmd% ') DO SET IMG_VERSION=%%i

IF EXIST %OUTPUT_IMAGE_PATH%\app-sign.bin     del %OUTPUT_IMAGE_PATH%\app-sign.bin

@REM :: Add image header, ptlvs and concatenate the cert
@REM %IMGTOOL% sign --config %CONFIG_FILE% ^
@REM          -k %ROTPK% ^
@REM          -P %KEY_PASSPHRASE% ^
@REM          -t "APP" ^
@REM          --cert %CERT_PATH%\arot-cert.pem ^
@REM         --algo_hash "%ALGO_HASH%" ^
@REM          --algo_sig "%ALGO_SIGN%" ^
@REM          %OUTPUT_PATH%\..\app.bin ^
@REM          %OUTPUT_IMAGE_PATH%\app-sign.bin
@REM @if not exist %OUTPUT_IMAGE_PATH%\app-sign.bin goto error

@REM if "%AESK%" NEQ "" (
@REM     %AESTOOL% -c %CONFIG_FILE% ^
@REM         -t "IMG_%INDEX%_NSPE" ^
@REM         -i %OUTPUT_IMAGE_PATH%\app-sign.bin ^
@REM         -o %OUTPUT_IMAGE_PATH%\app-sign"%AES_SUFFIX%".bin ^
@REM         -k %AESK%
@REM     echo Encrypted!
@REM )

@REM :: Convert to Intel HEX for debug
@REM %HEXTOOL% --c %CONFIG_FILE% ^
@REM         -t "IMG_%INDEX%_APP" ^
@REM         -e %SREC_CAT% ^
@REM         %OUTPUT_IMAGE_PATH%\app-sign%AES_SUFFIX%.bin ^
@REM         %OUTPUT_IMAGE_PATH%\app-sign%AES_SUFFIX%.hex

python %IMGTOOL% sign --align 4 -v "1.0.1" ^
    -k %ROTPK% ^
    -H 0x200 --pad-header ^
    -S 0x100000 --public-key-format hash --max-align 8 --pad-sig ^
    %OUTPUT_PATH%\..\app.bin ^
    %OUTPUT_IMAGE_PATH%\app-sign.bin

:: Convert to Intel HEX for debug
%HEXTOOL% --c %CONFIG_FILE% ^
        -t "IMG_%INDEX%_APP" ^
        -e %SREC_CAT% ^
        %OUTPUT_IMAGE_PATH%\app-sign%AES_SUFFIX%.bin ^
        %OUTPUT_IMAGE_PATH%\app-sign%AES_SUFFIX%.hex

:: Convert to Intel HEX for debug
%HEXTOOL% --c %CONFIG_FILE% ^
        -t "IMG_1_APP" ^
        -e %SREC_CAT% ^
        %OUTPUT_IMAGE_PATH%\app-sign%AES_SUFFIX%.bin ^
        %OUTPUT_IMAGE_PATH%\app-sign1%AES_SUFFIX%.hex

:: Generate images for mass production or upgrade
del %OUTPUT_IMAGE_PATH%\image*.bin          /q
del %OUTPUT_IMAGE_PATH%\image*.hex          /q

copy %OUTPUT_IMAGE_PATH%\app-sign.hex %OUTPUT_PATH%\app.hex
copy %OUTPUT_IMAGE_PATH%\app-sign1.hex %OUTPUT_PATH%\app1.hex
