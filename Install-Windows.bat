@echo off
REM Right-click -> Run as administrator
set DEST=%CommonProgramFiles%\VST3
xcopy "%~dp0Studio Bus Comp.vst3" "%DEST%\Studio Bus Comp.vst3\" /E /I /Y
echo.
echo Installed to %DEST%. In FL Studio: Options ^> Manage plugins ^> Find installed plugins.
pause
