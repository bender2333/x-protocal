cd

@REM echo %cd%

@REM %cd%/../imgtool/dist/imgtool.exe keygen -k secp256r1.pem -t  ecdsa-p256
set IMGTOOL=%cd%/../scripts_mcuboot/imgtool.py 
python %IMGTOOL% keygen -k mcuboot_secp256r1.pem -t  ecdsa-p256


