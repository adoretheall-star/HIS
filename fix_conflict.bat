@echo off
setlocal enabledelayedexpansion

set "inputFile=c:\Users\LENOVO\Desktop\HIS\main.c"
set "outputFile=c:\Users\LENOVO\Desktop\HIS\main_fixed.c"

del "%outputFile%" 2>nul

for /f "delims=" %%a in ('type "%inputFile%"') do (
    set "line=%%a"
    if "!line:~0,7!"=="<<<<<<< " (
        set "inConflict=1"
        set "keepHead=1"
    ) else if "!line:~0,7!"=="=======" (
        set "keepHead=0"
    ) else if "!line:~0,8!"==">>>>>>> " (
        set "inConflict=0"
    ) else if "!inConflict!"=="0" (
        echo !line!>>"%outputFile%"
    ) else if "!inConflict!"=="1" and "!keepHead!"=="1" (
        echo !line!>>"%outputFile%"
    )
)

copy "%outputFile%" "%inputFile%"
del "%outputFile%"

echo 冲突标记已移除