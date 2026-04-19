#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include "global.h"
#include "list_ops.h"

#define DATA_DIR "data/"

static void ensure_data_dir() {
    _mkdir(DATA_DIR);
}

static void trim_newline(char* str) {
    if (str == NULL) return;
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

int save_patient_list(PatientNode* head) {
    if (head == NULL) return 0;
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "patients.txt", "w");
    if (fp == NULL) return 0;

    PatientNode* curr = head->next;
    while (curr != NULL) {
        char prescription_str[2048] = "";
        PrescriptionNode* p = curr->script_head;
        if (p != NULL) {
            char temp[128];
            strcpy(prescription_str, "");
            while (p != NULL) {
                sprintf(temp, "%s:%d", p->med_id, p->quantity);
                if (strlen(prescription_str) > 0) {
                    strcat(prescription_str, ",");
                }
                strcat(prescription_str, temp);
                p = p->next;
            }
        } else {
            strcpy(prescription_str, "EMPTY");
        }

        fprintf(fp, "%s|%s|%d|%s|%d|%s|%s|%s|%s|%.2f|%d|%s|%s|%s|%d|%ld|%ld|%ld|%d|%ld|%d|%d|%ld|%d|%.2f|%ld\n",
            curr->id, curr->name, curr->age, curr->id_card, curr->m_type,
            curr->symptom, curr->target_dept, curr->doctor_id, curr->card_id,
            curr->balance, curr->status, curr->diagnosis_text, curr->treatment_advice,
            prescription_str, curr->script_count,
            (long)curr->missed_time_1, (long)curr->missed_time_2, (long)curr->missed_time_3,
            curr->missed_count, (long)curr->blacklist_expire, curr->is_blacklisted,
            curr->is_emergency, (long)curr->queue_time, curr->call_count,
            curr->emergency_debt, (long)curr->unpaid_time);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_patient_list(PatientNode** head) {
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "patients.txt", "r");
    if (fp == NULL) return 1;

    char line[4096];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char id[MAX_ID_LEN], name[MAX_NAME_LEN], id_card[MAX_ID_LEN];
        char symptom[MAX_SYMPTOM_LEN], target_dept[MAX_NAME_LEN], doctor_id[MAX_ID_LEN], card_id[MAX_NAME_LEN];
        char diagnosis_text[MAX_RECORD_LEN], treatment_advice[MAX_RECORD_LEN], prescription_str[2048];
        int age, m_type, status, script_count;
        double balance;
        long missed_time_1, missed_time_2, missed_time_3, blacklist_expire, unpaid_time;
        int missed_count, is_blacklisted, is_emergency, call_count;
        double emergency_debt;
        long queue_time;

        char* fields[30];
        int field_count = 0;
        char* token = strtok(line, "|");
        while (token != NULL && field_count < 30) {
            fields[field_count++] = token;
            token = strtok(NULL, "|");
        }

        if (field_count < 25) {
            token = strtok(NULL, "");
            continue;
        }

        strcpy(id, fields[0]);
        strcpy(name, fields[1]);
        age = atoi(fields[2]);
        strcpy(id_card, fields[3]);
        m_type = atoi(fields[4]);
        strcpy(symptom, fields[5]);
        strcpy(target_dept, fields[6]);
        strcpy(doctor_id, fields[7]);
        strcpy(card_id, fields[8]);
        balance = atof(fields[9]);
        status = atoi(fields[10]);
        strcpy(diagnosis_text, fields[11]);
        strcpy(treatment_advice, fields[12]);
        strcpy(prescription_str, fields[13]);
        script_count = atoi(fields[14]);
        missed_time_1 = atol(fields[15]);
        missed_time_2 = atol(fields[16]);
        missed_time_3 = atol(fields[17]);
        missed_count = atoi(fields[18]);
        blacklist_expire = atol(fields[19]);
        is_blacklisted = atoi(fields[20]);
        is_emergency = atoi(fields[21]);
        queue_time = atol(fields[22]);
        call_count = atoi(fields[23]);
        emergency_debt = atof(fields[24]);
        unpaid_time = atol(fields[25]);

        PatientNode* new_node = create_patient_node(id, name, age, id_card);
        if (new_node == NULL) continue;

        new_node->m_type = m_type;
        strcpy(new_node->symptom, symptom);
        strcpy(new_node->target_dept, target_dept);
        strcpy(new_node->doctor_id, doctor_id);
        strcpy(new_node->card_id, card_id);
        new_node->balance = balance;
        new_node->status = status;
        strcpy(new_node->diagnosis_text, diagnosis_text);
        strcpy(new_node->treatment_advice, treatment_advice);
        new_node->missed_time_1 = (time_t)missed_time_1;
        new_node->missed_time_2 = (time_t)missed_time_2;
        new_node->missed_time_3 = (time_t)missed_time_3;
        new_node->missed_count = missed_count;
        new_node->blacklist_expire = (time_t)blacklist_expire;
        new_node->is_blacklisted = is_blacklisted;
        new_node->is_emergency = is_emergency;
        new_node->queue_time = (time_t)queue_time;
        new_node->call_count = call_count;
        new_node->emergency_debt = emergency_debt;
        new_node->unpaid_time = (time_t)unpaid_time;

        if (strcmp(prescription_str, "EMPTY") != 0 && strlen(prescription_str) > 0) {
            char* p_token = strtok(prescription_str, ",");
            while (p_token != NULL) {
                char med_id[MAX_ID_LEN];
                int quantity;
                sscanf(p_token, "%[^:]:%d", med_id, &quantity);
                add_prescription_to_patient(new_node, med_id, quantity);
                p_token = strtok(NULL, ",");
            }
        }

        insert_patient_tail(*head, new_node);
    }
    fclose(fp);
    return 1;
}

int save_appointment_list(AppointmentNode* head) {
    if (head == NULL) return 0;
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "appointments.txt", "w");
    if (fp == NULL) return 0;

    AppointmentNode* curr = head->next;
    while (curr != NULL) {
        fprintf(fp, "%s|%s|%s|%s|%s|%s|%d|%.2f|%d|%d\n",
            curr->appointment_id, curr->patient_id, curr->appointment_date,
            curr->appointment_slot, curr->appoint_doctor, curr->appoint_dept,
            curr->appointment_status, curr->reg_fee, curr->fee_paid, curr->is_walk_in);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_appointment_list(AppointmentNode** head) {
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "appointments.txt", "r");
    if (fp == NULL) return 1;

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char appointment_id[MAX_ID_LEN], patient_id[MAX_ID_LEN], appointment_date[MAX_NAME_LEN];
        char appointment_slot[MAX_NAME_LEN], appoint_doctor[MAX_NAME_LEN], appoint_dept[MAX_NAME_LEN];
        int appointment_status, fee_paid, is_walk_in;
        double reg_fee;

        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%d|%lf|%d|%d",
            appointment_id, patient_id, appointment_date, appointment_slot,
            appoint_doctor, appoint_dept, &appointment_status, &reg_fee, &fee_paid, &is_walk_in);

        AppointmentNode* new_node = create_appointment_node(
            appointment_id, patient_id, appointment_date, appointment_slot,
            appoint_doctor, appoint_dept, appointment_status);
        if (new_node == NULL) continue;

        new_node->reg_fee = reg_fee;
        new_node->fee_paid = fee_paid;
        new_node->is_walk_in = is_walk_in;

        insert_appointment_tail(*head, new_node);
    }
    fclose(fp);
    return 1;
}

int save_doctor_list(DoctorNode* head) {
    if (head == NULL) return 0;
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "doctors.txt", "w");
    if (fp == NULL) return 0;

    DoctorNode* curr = head->next;
    while (curr != NULL) {
        fprintf(fp, "%s|%s|%s|%d|%d\n",
            curr->id, curr->name, curr->department, curr->queue_length, curr->is_on_duty);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_doctor_list(DoctorNode** head) {
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "doctors.txt", "r");
    if (fp == NULL) return 1;

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char id[MAX_ID_LEN], name[MAX_NAME_LEN], department[MAX_NAME_LEN];
        int queue_length, is_on_duty;

        sscanf(line, "%[^|]|%[^|]|%[^|]|%d|%d", id, name, department, &queue_length, &is_on_duty);

        DoctorNode* new_node = create_doctor_node(id, name, department);
        if (new_node == NULL) continue;

        new_node->queue_length = queue_length;
        new_node->is_on_duty = is_on_duty;

        insert_doctor_tail(*head, new_node);
    }
    fclose(fp);
    return 1;
}

int save_medicine_list(MedicineNode* head) {
    if (head == NULL) return 0;
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "medicines.txt", "w");
    if (fp == NULL) return 0;

    MedicineNode* curr = head->next;
    while (curr != NULL) {
        fprintf(fp, "%s|%s|%s|%s|%.2f|%d|%d|%s\n",
            curr->id, curr->name, curr->alias, curr->generic_name,
            curr->price, curr->stock, curr->m_type, curr->expiry_date);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_medicine_list(MedicineNode** head) {
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "medicines.txt", "r");
    if (fp == NULL) return 1;

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char id[MAX_ID_LEN], name[MAX_MED_NAME_LEN], alias[MAX_ALIAS_LEN];
        char generic_name[MAX_GENERIC_NAME_LEN], expiry_date[MAX_DATE_LEN];
        double price;
        int stock, m_type;

        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%lf|%d|%d|%[^|]",
            id, name, alias, generic_name, &price, &stock, &m_type, expiry_date);

        MedicineNode* new_node = create_medicine_node(
            id, name, alias, generic_name, price, stock, m_type, expiry_date);
        if (new_node == NULL) continue;

        insert_medicine_tail(*head, new_node);
    }
    fclose(fp);
    return 1;
}

int save_ward_list(WardNode* head) {
    if (head == NULL) return 0;
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "wards.txt", "w");
    if (fp == NULL) return 0;

    WardNode* curr = head->next;
    while (curr != NULL) {
        fprintf(fp, "%s|%s|%d|%d|%s\n",
            curr->room_id, curr->bed_id, curr->ward_type, curr->is_occupied, curr->patient_id);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_ward_list(WardNode** head) {
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "wards.txt", "r");
    if (fp == NULL) return 1;

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char room_id[MAX_ID_LEN], bed_id[MAX_ID_LEN], patient_id[MAX_ID_LEN];
        int ward_type, is_occupied;

        sscanf(line, "%[^|]|%[^|]|%d|%d|%[^|]", room_id, bed_id, &ward_type, &is_occupied, patient_id);

        WardNode* new_node = create_ward_node(room_id, bed_id, ward_type);
        if (new_node == NULL) continue;

        new_node->is_occupied = is_occupied;
        strcpy(new_node->patient_id, patient_id);

        insert_ward_tail(*head, new_node);
    }
    fclose(fp);
    return 1;
}

int save_account_list(AccountNode* head) {
    if (head == NULL) return 0;
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "accounts.txt", "w");
    if (fp == NULL) return 0;

    AccountNode* curr = head->next;
    while (curr != NULL) {
        fprintf(fp, "%s|%s|%s|%d|%d|%ld|%d\n",
            curr->username, curr->password, curr->real_name, curr->role,
            curr->error_count, (long)curr->lock_time, curr->is_on_duty);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_account_list(AccountNode** head) {
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "accounts.txt", "r");
    if (fp == NULL) return 1;

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char username[MAX_ID_LEN], password[MAX_ID_LEN], real_name[MAX_NAME_LEN];
        int role, error_count, is_on_duty;
        long lock_time;

        sscanf(line, "%[^|]|%[^|]|%[^|]|%d|%d|%ld|%d",
            username, password, real_name, &role, &error_count, &lock_time, &is_on_duty);

        AccountNode* new_node = create_account_node(username, password, real_name, role);
        if (new_node == NULL) continue;

        new_node->error_count = error_count;
        new_node->lock_time = (time_t)lock_time;
        new_node->is_on_duty = is_on_duty;

        insert_account_tail(*head, new_node);
    }
    fclose(fp);
    return 1;
}

int save_consult_record_list(ConsultRecordNode* head) {
    if (head == NULL) return 0;
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "consult_records.txt", "w");
    if (fp == NULL) return 0;

    ConsultRecordNode* curr = head->next;
    while (curr != NULL) {
        fprintf(fp, "%s|%s|%s|%s|%s|%s|%s|%d|%d|%d|%d|%s\n",
            curr->record_id, curr->patient_id, curr->doctor_id, curr->appointment_id,
            curr->consult_time, curr->diagnosis_text, curr->treatment_advice,
            curr->decision, curr->pre_status, curr->post_status, curr->star_rating, curr->feedback);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_consult_record_list(ConsultRecordNode** head) {
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "consult_records.txt", "r");
    if (fp == NULL) return 1;

    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char record_id[MAX_ID_LEN], patient_id[MAX_ID_LEN], doctor_id[MAX_ID_LEN];
        char appointment_id[MAX_ID_LEN], consult_time[MAX_NAME_LEN];
        char diagnosis_text[MAX_RECORD_LEN], treatment_advice[MAX_RECORD_LEN];
        char feedback[MAX_RECORD_LEN];
        int decision, pre_status, post_status, star_rating;

        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%d|%d|%d|%d|%[^|]",
            record_id, patient_id, doctor_id, appointment_id, consult_time,
            diagnosis_text, treatment_advice, &decision, &pre_status, &post_status,
            &star_rating, feedback);

        ConsultRecordNode* new_node = create_consult_record_node(
            record_id, patient_id, doctor_id, appointment_id, consult_time,
            diagnosis_text, treatment_advice, decision, pre_status, post_status);
        if (new_node == NULL) continue;

        new_node->star_rating = star_rating;
        strcpy(new_node->feedback, feedback);

        insert_consult_record_tail(*head, new_node);
    }
    fclose(fp);
    return 1;
}

int save_check_item_list(CheckItemNode* head) {
    if (head == NULL) return 0;
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "check_items.txt", "w");
    if (fp == NULL) return 0;

    CheckItemNode* curr = head->next;
    while (curr != NULL) {
        fprintf(fp, "%s|%s|%s|%.2f|%d\n",
            curr->item_id, curr->item_name, curr->dept, curr->price, curr->m_type);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_check_item_list(CheckItemNode** head) {
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "check_items.txt", "r");
    if (fp == NULL) return 1;

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char item_id[MAX_ID_LEN], item_name[MAX_NAME_LEN], dept[MAX_NAME_LEN];
        double price;
        int m_type;

        sscanf(line, "%[^|]|%[^|]|%[^|]|%lf|%d", item_id, item_name, dept, &price, &m_type);

        CheckItemNode* new_node = create_check_item_node(item_id, item_name, dept, price, m_type);
        if (new_node == NULL) continue;

        insert_check_item_tail(*head, new_node);
    }
    fclose(fp);
    return 1;
}

int save_check_record_list(CheckRecordNode* head) {
    if (head == NULL) return 0;
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "check_records.txt", "w");
    if (fp == NULL) return 0;

    CheckRecordNode* curr = head->next;
    while (curr != NULL) {
        fprintf(fp, "%s|%s|%s|%s|%s|%s|%s|%d|%d\n",
            curr->record_id, curr->patient_id, curr->item_id, curr->item_name,
            curr->dept, curr->check_time, curr->result, curr->is_completed, curr->is_paid);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_check_record_list(CheckRecordNode** head) {
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "check_records.txt", "r");
    if (fp == NULL) return 1;

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char record_id[MAX_ID_LEN], patient_id[MAX_ID_LEN], item_id[MAX_ID_LEN];
        char item_name[MAX_NAME_LEN], dept[MAX_NAME_LEN], check_time[MAX_NAME_LEN];
        char result[MAX_RECORD_LEN];
        int is_completed, is_paid;

        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%d|%d",
            record_id, patient_id, item_id, item_name, dept, check_time, result,
            &is_completed, &is_paid);

        CheckRecordNode* new_node = create_check_record_node(
            record_id, patient_id, item_id, item_name, dept, check_time, result,
            is_completed, is_paid);
        if (new_node == NULL) continue;

        insert_check_record_tail(*head, new_node);
    }
    fclose(fp);
    return 1;
}

int save_alert_list(AlertNode* head) {
    if (head == NULL) return 0;
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "alerts.txt", "w");
    if (fp == NULL) return 0;

    AlertNode* curr = head->next;
    while (curr != NULL) {
        fprintf(fp, "%s|%ld\n", curr->message, (long)curr->time);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_alert_list(AlertNode** head) {
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "alerts.txt", "r");
    if (fp == NULL) return 1;

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char message[256];
        long alert_time;

        char* last_pipe = strrchr(line, '|');
        if (last_pipe != NULL) {
            *last_pipe = '\0';
            strcpy(message, line);
            alert_time = atol(last_pipe + 1);
        } else {
            continue;
        }

        AlertNode* new_node = (AlertNode*)malloc(sizeof(AlertNode));
        if (new_node == NULL) continue;

        strncpy(new_node->message, message, 255);
        new_node->message[255] = '\0';
        new_node->time = (time_t)alert_time;
        new_node->prev = NULL;
        new_node->next = NULL;

        AlertNode* curr = *head;
        while (curr->next != NULL) curr = curr->next;
        curr->next = new_node;
        new_node->prev = curr;
    }
    fclose(fp);
    return 1;
}

int save_complaint_list(ComplaintNode* head) {
    if (head == NULL) return 0;
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "complaints.txt", "w");
    if (fp == NULL) return 0;

    ComplaintNode* curr = head->next;
    while (curr != NULL) {
        fprintf(fp, "%s|%s|%d|%s|%s|%s|%d|%s|%s\n",
            curr->complaint_id, curr->patient_id, curr->target_type, curr->target_id,
            curr->target_name, curr->content, curr->status, curr->response, curr->submit_time);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_complaint_list(ComplaintNode** head) {
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "complaints.txt", "r");
    if (fp == NULL) return 1;

    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char complaint_id[MAX_ID_LEN], patient_id[MAX_ID_LEN], target_id[MAX_ID_LEN];
        char target_name[MAX_NAME_LEN], content[MAX_RECORD_LEN], response[MAX_RECORD_LEN];
        char submit_time[MAX_NAME_LEN];
        int target_type, status;

        sscanf(line, "%[^|]|%[^|]|%d|%[^|]|%[^|]|%[^|]|%d|%[^|]|%[^|]",
            complaint_id, patient_id, &target_type, target_id, target_name,
            content, &status, response, submit_time);

        ComplaintNode* new_node = create_complaint_node(
            complaint_id, patient_id, target_type, target_id, target_name,
            content, status, response, submit_time);
        if (new_node == NULL) continue;

        insert_complaint_tail(*head, new_node);
    }
    fclose(fp);
    return 1;
}

int save_log_list(LogNode* head) {
    if (head == NULL) return 0;
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "logs.txt", "w");
    if (fp == NULL) return 0;

    LogNode* curr = head;
    while (curr != NULL) {
        fprintf(fp, "%s|%s|%s|%s\n", curr->timestamp, curr->operation, curr->target, curr->description);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_log_list(LogNode** head) {
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "logs.txt", "r");
    if (fp == NULL) return 1;

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char timestamp[20], operation[50], target[50], description[200];

        sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]", timestamp, operation, target, description);

        LogNode* new_node = (LogNode*)malloc(sizeof(LogNode));
        if (new_node == NULL) continue;

        strncpy(new_node->timestamp, timestamp, 19);
        new_node->timestamp[19] = '\0';
        strncpy(new_node->operation, operation, 49);
        new_node->operation[49] = '\0';
        strncpy(new_node->target, target, 49);
        new_node->target[49] = '\0';
        strncpy(new_node->description, description, 199);
        new_node->description[199] = '\0';
        new_node->next = NULL;

        if (*head == NULL) {
            *head = new_node;
        } else {
            LogNode* curr = *head;
            while (curr->next != NULL) curr = curr->next;
            curr->next = new_node;
        }
    }
    fclose(fp);
    return 1;
}

int save_inpatient_list(InpatientRecord* head) {
    if (head == NULL) return 0;
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "inpatients.txt", "w");
    if (fp == NULL) return 0;

    InpatientRecord* curr = head->next;
    while (curr != NULL) {
        fprintf(fp, "%s|%s|%s|%d|%d|%d|%d|%.2f|%d\n",
            curr->inpatient_id, curr->patient_id, curr->bed_id,
            curr->ward_type, curr->recommended_ward_type, curr->estimated_days,
            curr->days_stayed, curr->deposit_balance, curr->is_active);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_inpatient_list(InpatientRecord** head) {
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "inpatients.txt", "r");
    if (fp == NULL) return 1;

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char inpatient_id[MAX_ID_LEN], patient_id[MAX_ID_LEN], bed_id[MAX_ID_LEN];
        int ward_type, recommended_ward_type, estimated_days, days_stayed, is_active;
        double deposit_balance;

        sscanf(line, "%[^|]|%[^|]|%[^|]|%d|%d|%d|%d|%lf|%d",
            inpatient_id, patient_id, bed_id, &ward_type, &recommended_ward_type,
            &estimated_days, &days_stayed, &deposit_balance, &is_active);

        InpatientRecord* new_node = create_inpatient_record_node(
            inpatient_id, patient_id, bed_id, ward_type, recommended_ward_type,
            estimated_days, days_stayed, deposit_balance, is_active);
        if (new_node == NULL) continue;

        insert_inpatient_record_tail(*head, new_node);
    }
    fclose(fp);
    return 1;
}

int save_all_data() {
    int result = 1;
    result &= save_patient_list(g_patient_list);
    result &= save_appointment_list(g_appointment_list);
    result &= save_doctor_list(g_doctor_list);
    result &= save_medicine_list(g_medicine_list);
    result &= save_ward_list(g_ward_list);
    result &= save_account_list(g_account_list);
    result &= save_consult_record_list(g_consult_record_list);
    result &= save_check_item_list(g_check_item_list);
    result &= save_check_record_list(g_check_record_list);
    result &= save_alert_list(g_alert_list);
    result &= save_complaint_list(g_complaint_list);
    result &= save_log_list(g_log_list);
    result &= save_inpatient_list(g_inpatient_list);
    return result;
}

int load_all_data() {
    int result = 1;
    result &= load_patient_list(&g_patient_list);
    result &= load_appointment_list(&g_appointment_list);
    result &= load_doctor_list(&g_doctor_list);
    result &= load_medicine_list(&g_medicine_list);
    result &= load_ward_list(&g_ward_list);
    result &= load_account_list(&g_account_list);
    result &= load_consult_record_list(&g_consult_record_list);
    result &= load_check_item_list(&g_check_item_list);
    result &= load_check_record_list(&g_check_record_list);
    result &= load_alert_list(&g_alert_list);
    result &= load_complaint_list(&g_complaint_list);
    result &= load_log_list(&g_log_list);
    result &= load_inpatient_list(&g_inpatient_list);
    return result;
}