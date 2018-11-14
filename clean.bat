@echo off

echo 正在清除……

del /f /s /q .\mdk\obj\*.o
del /f /s /q .\mdk\obj\*.axf
del /f /s /q .\mdk\obj\*.crf
del /f /s /q .\mdk\obj\*.bin
del /f /s /q .\mdk\obj\*.plg
del /f /s /q .\mdk\obj\*.lst
del /f /s /q .\mdk\obj\*.lnp
del /f /s /q .\mdk\obj\*.map
del /f /s /q .\mdk\obj\*.tra
del /f /s /q .\mdk\obj\*.d
del /f /s /q .\mdk\obj\*.__i
del /f /s /q .\mdk\obj\*.hex
del /f /s /q .\mdk\obj\*.htm
del /f /s /q .\mdk\obj\*.txt
del /f /s /q .\mdk\obj\*.iex
del /f /s /q .\*.bak

echo 清除完成！
echo. & pause

