RD /S %~dp0out\build\x64-debug\src\StreamingServer\assets
xcopy /e /k /h /i %~dp0assets %~dp0\out\build\x64-debug\src\StreamingServer\assets
