call "c:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
set INCLUDE=%INCLUDE%;c:\Program Files\Microsoft SDKs\Windows\v6.0A\Include
set LIB=%LIB%;c:\Program Files\Microsoft SDKs\Windows\v6.0A\lib
set PATH=%PATH%;C:\Program Files\Microsoft SDKs\Windows\v6.0A\bin
cmake .
cmake --build . --config release --target install --
