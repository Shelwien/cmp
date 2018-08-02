@echo off

set ICLCFG=icl.cfg

del cmp.exe

perl txt2inc.pl help.txt help.inc

:call c:\vc71\c2.bat cmp.cpp /I..\Lib
:call C:\IntelB1054\bin\ia32\icl.bat cmp.cpp 
:call C:\IntelG0815\bin-ia32\icl1.bat cmp.cpp 

set LIB=C:\IntelB1054\lib\ia32

call C:\IntelJ0070\bin-ia32\icl.bat /Qm32 cmp.cpp /Fecmp.exe
rem call C:\IntelI0124\bin-ia32\icl.bat /Qm32 cmp.cpp /Fecmp.exe

c:\vc\clr_header cmp.exe 

rem c:\vc71\link.exe /edit /subsystem:console cmp.exe

del *.obj
