#ifndef PHARMACY_SERVICE_H
#define PHARMACY_SERVICE_H

#include "medicine_service.h"

void show_paid_patients_waiting_for_dispense(void);
int dispense_medicine_for_patient(const char* patient_id);

#endif // PHARMACY_SERVICE_H