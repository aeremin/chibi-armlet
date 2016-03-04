:start

@REM Erase EE
"C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe" -SE ed1

IF %ERRORLEVEL% EQU 0 (
    "C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe" -c SWD -P ChArmletBasic.hex 0x08000000 -Run
)

@echo Insert Connector

@REM pause
@ping 10.25.11.254 -n 1 -w 2000 > nul

goto start