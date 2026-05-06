@echo off
title HIS UTF-8 Fixed
cd /d "%~dp0"
chcp 65001 >nul

echo Building fixed HIS system...
gcc -finput-charset=UTF-8 -fexec-charset=UTF-8 main.c utils.c list_ops.c appointment.c patient_service.c doctor_service.c medicine_service.c pharmacy_service.c admin_service.c inpatient_service.c data_io.c -o his_system.exe -static

if errorlevel 1 (
    echo Build failed. Send the red error screenshot to ChatGPT.
    pause
    exit /b 1
)

cls
echo Build OK. Starting HIS...
his_system.exe
pause
