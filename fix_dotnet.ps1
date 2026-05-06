$file = "admin_service.c"
$content = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8)
$content = $content.Replace('print_progress_bar_single("待诊患者",', 'print_progress_bar_single("待诊患者  ",')
$content = $content.Replace('print_progress_bar_single("急诊患者",', 'print_progress_bar_single("急诊患者  ",')
[System.IO.File]::WriteAllText($file, $content, [System.Text.Encoding]::UTF8)
Write-Host "Modified successfully!"