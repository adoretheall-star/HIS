# Fix the corrupted appointment.c file
$filePath = $PSScriptRoot + "\appointment.c"
$content = Get-Content -Path $filePath -Raw

# Fix the escaped asterisks and parentheses
$content = $content -replace 'DoctorNode\\\* assign_doctor_by_department\\\(const char\\\* dept_name\\\)', 'DoctorNode* assign_doctor_by_department(const char* dept_name)'

Set-Content -Path $filePath -Value $content -Encoding UTF8
Write-Host "Fixed escaped characters"