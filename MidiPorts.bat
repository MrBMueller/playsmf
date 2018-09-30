
echo off

:loop
cls

echo.
echo ins:

.\bin\lsmidiins.exe

echo.
echo outs:

.\bin\lsmidiouts.exe

pause > nul

goto loop
