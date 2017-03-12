@echo off

set ICLCFG=icl.cfg

del cmp.exe

perl txt2inc.pl help.txt help.inc

:call c:\vc71\c2.bat cmp.cpp /I..\Lib
:call C:\IntelB1054\bin\ia32\icl.bat cmp.cpp 
rem call C:\IntelG0815\bin-ia32\icl1.bat cmp.cpp /Fecmp

rem call C:\IntelH0721\bin-intel64\icl.bat cmp.cpp /Fecmp

call C:\IntelH0721\bin-ia32\icl1.bat cmp.cpp icon.obj icnewfeat32.lib /Fecmp

del *.obj
