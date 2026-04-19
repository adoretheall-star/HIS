@echo off
chcp 65001 > nul

:: 2. 核心修改：加上 -static 让程序自带底层运行库，彻底解决 0xc000007b 报错！
gcc main.c utils.c list_ops.c appointment.c patient_service.c doctor_service.c medicine_service.c pharmacy_service.c admin_service.c inpatient_service.c -o his_system.exe -fexec-charset=UTF-8 -static

if %errorlevel% neq 0 (
    echo.
    echo ❌ 糟糕！代码编译失败了，请回 Trae 检查报错信息！
    pause
    exit
)

cls
echo.
echo ✅ 编译成功，正在启动系统...
echo.

his_system.exe
pause