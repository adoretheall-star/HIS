#ifndef DATA_IO_H
#define DATA_IO_H

#include "global.h"

int save_patient_list(PatientNode* head);
int load_patient_list(PatientNode** head);

int save_appointment_list(AppointmentNode* head);
int load_appointment_list(AppointmentNode** head);

int save_doctor_list(DoctorNode* head);
int load_doctor_list(DoctorNode** head);

int save_medicine_list(MedicineNode* head);
int load_medicine_list(MedicineNode** head);

int save_ward_list(WardNode* head);
int load_ward_list(WardNode** head);

int save_account_list(AccountNode* head);
int load_account_list(AccountNode** head);

int save_consult_record_list(ConsultRecordNode* head);
int load_consult_record_list(ConsultRecordNode** head);

int save_check_item_list(CheckItemNode* head);
int load_check_item_list(CheckItemNode** head);

int save_check_record_list(CheckRecordNode* head);
int load_check_record_list(CheckRecordNode** head);

int save_alert_list(AlertNode* head);
int load_alert_list(AlertNode** head);

int save_complaint_list(ComplaintNode* head);
int load_complaint_list(ComplaintNode** head);

int save_log_list(LogNode* head);
int load_log_list(LogNode** head);

int save_inpatient_list(InpatientRecord* head);
int load_inpatient_list(InpatientRecord** head);

int save_all_data();
int load_all_data();

#endif