REM Remove log files ('log' is no longer used. 'logs' contains current logs, these get automatically cleaned if they exceed 10mb).
rd /S /Q "%~dp0log"
rd /S /Q "%~dp0logs"
REM Remove old NavMesh files which where included as an attempt to debug/suppress warnings.
rd /S /Q "%~dp0..\maps"
rd /S /Q "%~dp0maps\graphs"
rd /S /Q "%~dp0maps\navmesh"
REM Remove deprecated binary and configuration files (these are no longer used).
del /Q "%~dp0..\gameinfo.txt"
del /Q "%~dp0..\banlist.config"
del /Q "%~dp0..\gui.config"
del /Q "%~dp0..\Run R5 Reloaded.exe"
del /Q "%~dp0..\r5reloaded.exe"
del /Q "%~dp0..\r5apexsdkd64.dll"
del /Q "%~dp0..\r5detours.dll"
del /Q "%~dp0..\r5dev.dll"
REM Remove deprecated pak files (these are no longer used).
del /Q "%~dp0..\paks\Win32\common_empty.rpak"
del /Q "%~dp0..\paks\Win32\common_sdk2.rpak"
