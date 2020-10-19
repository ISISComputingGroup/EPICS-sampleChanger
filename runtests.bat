@echo on
setlocal
set "ARCH=%1"
set "TESTPATH=%~dp0sampleChangerApp/src/O.%ARCH%"
if exist "%TESTPATH%\runner.exe" (
    call %TESTPATH%\dllPath.bat
    %TESTPATH%\runner.exe --gtest_output=xml:./test-reports/TEST-samplechanger.xml
) else (
    @echo No tests to run
)
