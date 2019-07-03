if "%1" equ "" (
  echo Error: Lustra ID argument not specified
  exit /b 1
)

@set /A dec = %1
:: Convert decimal input tp hex required for CLI
:convert
@call cmd /c exit /b %dec%
@set hex=%=exitcode%
::echo %hex%

:start

"c:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe" -c SWD -P FalloutDevice.hex 0x08000000 -V
::"c:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe" -rst
::timeout 6
::"c:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe" -Halt
"c:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe" -c SWD -w32 0x08080000 %hex%
"c:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe" -c SWD -w32 0x08080004 FFFFFF9C
"c:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe" -c SWD -w32 0x08080008 8
"c:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe" -rst


@CHOICE /C NAX /M "Flash [N]ext ID or try [A]gain, or e[X]it?"
IF %ERRORLEVEL% EQU 3 goto exit
IF %ERRORLEVEL% EQU 2 goto start
IF %ERRORLEVEL% EQU 1 goto next

:next
set /A dec=dec+1
@echo %dec%
goto convert

::pause
::@ping 10.25.11.254 -n 1 -w 2000 > nul

@goto start

:exit
@echo Bye!