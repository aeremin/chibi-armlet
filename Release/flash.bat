:start
"C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-LINK_CLI.exe" -c SWD -P ChArmletBasic.hex 0x08000000 -Run

@REM pause

@ping 10.25.11.254 -n 1 -w 3000 > nul

goto start