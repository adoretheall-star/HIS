$content = Get-Content -Path 'utils.c' -Raw -Encoding UTF8
$old = 'printf("%3d 人  [", queue);'
$new = '{ char num_buf[32]; sprintf(num_buf, "%d人", queue); print_padded_text(num_buf, 8); } printf("  [");'
$content = $content -replace [regex]::Escape($old), $new
$content | Set-Content -Path 'utils.c' -Encoding UTF8 -NoNewline