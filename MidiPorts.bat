
echo off

:loop
cls

echo.
echo ins:

.\lsmidiins.exe

echo.
echo outs:

.\lsmidiouts.exe

pause > nul

goto loop
