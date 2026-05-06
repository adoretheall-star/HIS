@echo off
title HIS-UTF8
chcp 65001
cd /d "%~dp0"

if exist "HIS.exe" (
    HIS.exe
) else if exist "x64\Debug\HIS.exe" (
    "x64\Debug\HIS.exe"
) else if exist "x64\Release\HIS.exe" (
    "x64\Release\HIS.exe"
) else if exist "Debug\HIS.exe" (
    "Debug\HIS.exe"
) else (
    echo 未找到 HIS.exe，请检查 exe 路径。
)

pause