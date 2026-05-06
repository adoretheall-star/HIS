$content = Get-Content -Path 'admin_service.c' -Raw -Encoding UTF8

$content = $content -replace 'print_progress_bar_single\("待诊患者",', 'print_progress_bar_single("待诊患者  ",'
$content = $content -replace 'print_progress_bar_single\("急诊患者",', 'print_progress_bar_single("急诊患者  ",'
$content = $content -replace 'print_progress_bar_single\("待缴费患者",', 'print_progress_bar_single("待缴费患者",'
$content = $content -replace 'print_progress_bar_single\("待取药患者",', 'print_progress_bar_single("待取药患者",'

$content | Set-Content -Path 'admin_service.c' -Encoding UTF8 -NoNewline

Write-Host "Done!"