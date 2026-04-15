@echo off
:: 1. 强制当前黑框框使用 UTF-8 编码
chcp 65001 > nul

:: 2. 核心修改：加上 -static 让程序自带底层运行库，彻底解决 0xc000007b 报错！
<<<<<<< HEAD
gcc main.c utils.c list_ops.c appointment.c patient_service.c doctor_service.c -o his_system.exe -fexec-charset=UTF-8 -static
=======
gcc main.c utils.c list_ops.c appointment.c patient_service.c medicine_service.c pharmacy_service.c admin_service.c -o his_system.exe -fexec-charset=UTF-8 -static
>>>>>>> medicine

:: 3. 拦截编译错误
if %errorlevel% neq 0 (
    echo.
    echo ❌ 糟糕！代码编译失败了，请回 Trae 检查报错信息！
    pause
    exit
)

:: 4. 清屏并启动
cls
his_system.exe

:: 5. 防止闪退
pause
