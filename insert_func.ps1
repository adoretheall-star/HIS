$filePath = $PSScriptRoot + "\appointment.c"
$content = Get-Content -Path $filePath -Encoding Default -Raw

# Try to find and insert before the function
$pattern = "DoctorNode\* assign_doctor_by_department\(const char\* dept_name\)"

$newFunction = @"

// check for duplicate appointment (early validation)
// return 1 if duplicate exists, 0 if no duplicate
int check_duplicate_appointment_early(
    const char* patient_id,
    const char* appointment_date,
    const char* appointment_slot,
    char* existing_appointment_id,
    size_t id_size
) {
    if (patient_id == NULL || appointment_date == NULL || appointment_slot == NULL)
        return 0;
    if (g_appointment_list == NULL)
        return 0;

    AppointmentNode* curr = g_appointment_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->patient_id, patient_id) == 0 &&
            strcmp(curr->appointment_date, appointment_date) == 0 &&
            strcmp(curr->appointment_slot, appointment_slot) == 0 &&
            (curr->appointment_status == RESERVED || curr->appointment_status == CHECKED_IN))
        {
            if (existing_appointment_id != NULL && id_size > 0)
            {
                strncpy(existing_appointment_id, curr->appointment_id, id_size - 1);
                existing_appointment_id[id_size - 1] = 0;
            }
            return 1;
        }
        curr = curr->next;
    }
    return 0;
}

"@

if ($content -match $pattern) {
    $content = $content -replace ($pattern), ($newFunction + $pattern)
    Set-Content -Path $filePath -Value $content -Encoding Default
    Write-Host "Inserted successfully"
} else {
    Write-Host "Pattern not found"
}