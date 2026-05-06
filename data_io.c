#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <time.h>
#include "global.h"
#include "list_ops.h"

// 调试开关，默认关闭（设�?显示详细调试信息�?
#define DATA_IO_DEBUG 0

#define DATA_DIR "data/"

static void ensure_data_dir() {
    _mkdir(DATA_DIR);
}

static void trim_newline(char* str) {
    if (str == NULL) return;
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len = strlen(str);
    }
}

static int split_line_by_delimiter(const char* line, char delimiter, char** fields, int max_fields) {
    if (line == NULL || fields == NULL || max_fields <= 0) return 0;
    
    static char line_copy[8192];
    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';
    
    int count = 0;
    char* start = line_copy;
    char* p = line_copy;
    
    while (*p && count < max_fields) {
        if (*p == delimiter) {
            *p = '\0';
            fields[count++] = start;
            start = p + 1;
        }
        p++;
    }
    fields[count++] = start;
    
    return count;
}



int save_patient_list(PatientNode* head) {
    if (head == NULL) return 0;
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "patients.txt", "w");
    if (fp == NULL) return 0;

    // 写入表头
    fprintf(fp, "患者编号|姓名|性别|年龄|身份证号|医保类型(0=�?1=甲类,2=乙类)|症状|目标科室|医生编号|就诊卡号|余额|状�?1=待诊,2=检查中,3=检查后待复�?4=已看诊待缴费,5=已缴费待取药,6=住院�?7=就诊结束,8=过号作废)|诊断结果|治疗意见|处方|处方数量|爽约时间1|爽约时间2|爽约时间3|爽约次数|黑名单过期时间|是否黑名�?0=�?1=�?|是否急诊(0=�?1=�?|排队时间|呼叫次数|急诊欠费|欠费时间\n");

    PatientNode* curr = head->next;
    while (curr != NULL) {
        char prescription_str[2048] = "";
        PrescriptionNode* p = curr->script_head;
        if (p != NULL) {
            char temp[128];
            strcpy(prescription_str, "");
            while (p != NULL) {
                snprintf(temp, sizeof(temp), "%s:%d", p->med_id, p->quantity);
                if (strlen(prescription_str) > 0) {
                    strcat(prescription_str, ",");
                }
                strcat(prescription_str, temp);
                p = p->next;
            }
        } else {
            strcpy(prescription_str, "EMPTY");
        }

        fprintf(fp, "%s|%s|%s|%d|%s|%d|%s|%s|%s|%s|%.2f|%d|%s|%s|%s|%d|%ld|%ld|%ld|%d|%ld|%d|%d|%ld|%d|%.2f|%ld\n",
            curr->id, curr->name, curr->gender, curr->age, curr->id_card, curr->m_type,
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
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%spatients.txt", DATA_DIR);
    #if DATA_IO_DEBUG
    printf("DEBUG: 正在读取患者文�? %s\n", file_path);
    #endif
    FILE* fp = fopen(file_path, "r");
    if (fp == NULL) {
        #if DATA_IO_DEBUG
        printf("DEBUG: 无法打开患者文�? %s\n", file_path);
        #endif
        return 0;
    }

    // 跳过表头
    char header_buffer[512];
    if (fgets(header_buffer, sizeof(header_buffer), fp) == NULL) {
        fclose(fp);
        return 0;
    }

    char line[4096];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char id[MAX_ID_LEN], name[MAX_NAME_LEN], gender[8], id_card[MAX_ID_LEN];
        char symptom[MAX_SYMPTOM_LEN], target_dept[MAX_NAME_LEN], doctor_id[MAX_ID_LEN], card_id[MAX_NAME_LEN];
        char diagnosis_text[MAX_RECORD_LEN], treatment_advice[MAX_RECORD_LEN], prescription_str[2048];
        int age, m_type, status;
        double balance;
        long missed_time_1, missed_time_2, missed_time_3, blacklist_expire, unpaid_time;
        int missed_count, is_blacklisted, is_emergency, call_count;
        double emergency_debt;
        long queue_time;

        char* fields[30];
        int field_count = split_line_by_delimiter(line, '|', fields, 30);
        
        if (field_count < 27) {
            #if DATA_IO_DEBUG
            printf("DEBUG: 字段数不足，跳过此行: %s (字段�? %d)\n", line, field_count);
            #endif
            continue;
        }

        strcpy(id, fields[0]);
        strcpy(name, fields[1]);
        strcpy(gender, fields[2]);
        age = atoi(fields[3]);
        strcpy(id_card, fields[4]);
        m_type = atoi(fields[5]);
        strcpy(symptom, fields[6]);
        strcpy(target_dept, fields[7]);
        strcpy(doctor_id, fields[8]);
        strcpy(card_id, fields[9]);
        balance = atof(fields[10]);
        status = atoi(fields[11]);
        strcpy(diagnosis_text, fields[12]);
        strcpy(treatment_advice, fields[13]);
        strcpy(prescription_str, fields[14]);
        (void)atoi(fields[15]);
        missed_time_1 = atol(fields[16]);
        missed_time_2 = atol(fields[17]);
        missed_time_3 = atol(fields[18]);
        missed_count = atoi(fields[19]);
        blacklist_expire = atol(fields[20]);
        is_blacklisted = atoi(fields[21]);
        is_emergency = atoi(fields[22]);
        queue_time = atol(fields[23]);
        call_count = atol(fields[24]);
        emergency_debt = atof(fields[25]);
        unpaid_time = atol(fields[26]);

        PatientNode* new_node = create_patient_node(id, name, age, gender, id_card);
        if (new_node == NULL) continue;

        strcpy(new_node->gender, gender);
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

    // 写入表头
    fprintf(fp, "预约编号|患者编号|预约日期|预约时段|医生编号|科室|状�?1=待确�?2=已确�?3=已完�?4=已取�?|挂号费|是否缴费(0=�?1=�?|是否现场挂号(0=�?1=�?\n");

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

    // 跳过表头
    char header_buffer[512];
    if (fgets(header_buffer, sizeof(header_buffer), fp) == NULL) {
        fclose(fp);
        return 1;
    }

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char appointment_id[MAX_ID_LEN], patient_id[MAX_ID_LEN], appointment_date[MAX_NAME_LEN];
        char appointment_slot[MAX_NAME_LEN], appoint_doctor[MAX_NAME_LEN], appoint_dept[MAX_NAME_LEN];
        int appointment_status, fee_paid, is_walk_in;
        double reg_fee;

        char* fields[15];
        int field_count = split_line_by_delimiter(line, '|', fields, 15);
        
        if (field_count < 10) continue;
        
        strcpy(appointment_id, fields[0]);
        strcpy(patient_id, fields[1]);
        strcpy(appointment_date, fields[2]);
        strcpy(appointment_slot, fields[3]);
        strcpy(appoint_doctor, fields[4]);
        strcpy(appoint_dept, fields[5]);
        appointment_status = atoi(fields[6]);
        reg_fee = atof(fields[7]);
        fee_paid = atoi(fields[8]);
        is_walk_in = atoi(fields[9]);

        AppointmentNode* new_node = create_appointment_node(
            appointment_id, patient_id, appointment_date, appointment_slot,
            appoint_doctor, appoint_dept, appointment_status);
        if (new_node == NULL) continue;

        new_node->reg_fee = reg_fee;
        new_node->fee_paid = fee_paid;
        new_node->is_walk_in = is_walk_in;
        
        // 设置显示用的医生姓名和科室名称
        strncpy(new_node->department, appoint_dept, MAX_NAME_LEN - 1);
        if (strlen(appoint_doctor) > 0)
        {
            DoctorNode* doctor = find_doctor_by_id(g_doctor_list, appoint_doctor);
            if (doctor != NULL)
            {
                strncpy(new_node->doctor_name, doctor->name, MAX_NAME_LEN - 1);
                strncpy(new_node->department, doctor->department, MAX_NAME_LEN - 1);
            }
            else
            {
                strncpy(new_node->doctor_name, appoint_doctor, MAX_NAME_LEN - 1);
            }
        }

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

    // 写入表头
    fprintf(fp, "医生编号|姓名|性别|科室|排队长度|是否值班(0=�?1=�?\n");

    DoctorNode* curr = head->next;
    while (curr != NULL) {
        fprintf(fp, "%s|%s|%s|%s|%d|%d\n",
            curr->id, curr->name, curr->gender, curr->department, curr->queue_length, curr->is_on_duty);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_doctor_list(DoctorNode** head) {
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "doctors.txt", "r");
    if (fp == NULL) return 1;

    // 跳过表头
    char header_buffer[512];
    if (fgets(header_buffer, sizeof(header_buffer), fp) == NULL) {
        fclose(fp);
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char id[MAX_ID_LEN], name[MAX_NAME_LEN], gender[8], department[MAX_NAME_LEN];
        int queue_length, is_on_duty;

        char* fields[10];
        int field_count = split_line_by_delimiter(line, '|', fields, 10);
        
        if (field_count < 6) continue;
        
        strcpy(id, fields[0]);
        strcpy(name, fields[1]);
        strcpy(gender, fields[2]);
        strcpy(department, fields[3]);
        queue_length = atoi(fields[4]);
        is_on_duty = atoi(fields[5]);

        DoctorNode* new_node = create_doctor_node(id, name, gender, department);
        if (new_node == NULL) continue;

        strcpy(new_node->gender, gender);
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

    // 写入表头
    fprintf(fp, "药品编号|商品名|别名|通用名|单价|库存|医保类型(0=自费,1=甲类,2=乙类)|有效期\n");

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

    // 跳过表头
    char header_buffer[512];
    if (fgets(header_buffer, sizeof(header_buffer), fp) == NULL) {
        fclose(fp);
        return 1;
    }

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char id[MAX_ID_LEN], name[MAX_MED_NAME_LEN], alias[MAX_ALIAS_LEN];
        char generic_name[MAX_GENERIC_NAME_LEN], expiry_date[MAX_DATE_LEN];
        double price;
        int stock, m_type;

        char* fields[10];
        int field_count = split_line_by_delimiter(line, '|', fields, 10);
        
        if (field_count < 8) continue;
        
        strcpy(id, fields[0]);
        strcpy(name, fields[1]);
        strcpy(alias, fields[2]);
        strcpy(generic_name, fields[3]);
        price = atof(fields[4]);
        stock = atoi(fields[5]);
        m_type = atoi(fields[6]);
        strcpy(expiry_date, fields[7]);

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

    // 写入表头
    fprintf(fp, "病房号|床位号|病房类型(1=普�?2=ICU,3=隔离病房,4=单人病房)|所属科室|是否占用(0=�?1=�?|患者编号\n");

    WardNode* curr = head->next;
    while (curr != NULL) {
        fprintf(fp, "%s|%s|%d|%s|%d|%s\n",
            curr->room_id, curr->bed_id, curr->ward_type, curr->dept, curr->is_occupied, curr->patient_id);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_ward_list(WardNode** head) {
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "wards.txt", "r");
    if (fp == NULL) return 1;

    // 跳过表头
    char header_buffer[512];
    if (fgets(header_buffer, sizeof(header_buffer), fp) == NULL) {
        fclose(fp);
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char room_id[MAX_ID_LEN], bed_id[MAX_ID_LEN], patient_id[MAX_ID_LEN], dept[MAX_NAME_LEN];
        int ward_type, is_occupied;

        char* fields[10];
        int field_count = split_line_by_delimiter(line, '|', fields, 10);
        
        if (field_count < 6) continue;
        
        strcpy(room_id, fields[0]);
        strcpy(bed_id, fields[1]);
        ward_type = atoi(fields[2]);
        strcpy(dept, fields[3]);
        is_occupied = atoi(fields[4]);
        strcpy(patient_id, fields[5]);

        WardNode* new_node = create_ward_node(room_id, bed_id, ward_type, dept);
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

    // 写入表头
    fprintf(fp, "用户名|密码|真实姓名|性别|角色(0=管理�?1=医生,2=护士,3=药师)|错误次数|锁定时间|是否值班(0=�?1=�?\n");

    AccountNode* curr = head->next;
    while (curr != NULL) {
        fprintf(fp, "%s|%s|%s|%s|%d|%d|%ld|%d\n",
            curr->username, curr->password, curr->real_name, curr->gender, curr->role,
            curr->error_count, (long)curr->lock_time, curr->is_on_duty);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_account_list(AccountNode** head) {
    ensure_data_dir();
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%saccounts.txt", DATA_DIR);
    
    #if DATA_IO_DEBUG
    printf("DEBUG: 正在读取账号文件: %s\n", file_path);
    #endif
    
    FILE* fp = fopen(file_path, "r");
    if (fp == NULL) {
        #if DATA_IO_DEBUG
        printf("DEBUG: 无法打开账号文件: %s\n", file_path);
        #endif
        return 0;
    }

    // 跳过表头
    char header_buffer[512];
    if (fgets(header_buffer, sizeof(header_buffer), fp) == NULL) {
        fclose(fp);
        return 0;
    }

    char line[512];
    int loaded_count = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char username[MAX_ID_LEN], password[MAX_ID_LEN], real_name[MAX_NAME_LEN], gender[8];
        int role, error_count, is_on_duty;
        long lock_time;

        char* fields[10];
        int field_count = split_line_by_delimiter(line, '|', fields, 10);
        
        if (field_count < 8) continue;
        
        strcpy(username, fields[0]);
        strcpy(password, fields[1]);
        strcpy(real_name, fields[2]);
        strcpy(gender, fields[3]);
        role = atoi(fields[4]);
        error_count = atoi(fields[5]);
        lock_time = atol(fields[6]);
        is_on_duty = atoi(fields[7]);

        AccountNode* new_node = create_account_node(username, password, real_name, gender, role);
        if (new_node == NULL) continue;

        strcpy(new_node->gender, gender);
        new_node->error_count = error_count;
        new_node->lock_time = (time_t)lock_time;
        new_node->is_on_duty = is_on_duty;

        insert_account_tail(*head, new_node);
        loaded_count++;
    }
    fclose(fp);
    
    #if DATA_IO_DEBUG
    printf("DEBUG: 成功加载 %d 个账号\n", loaded_count);
    #endif
    
    return 1;
}

int save_consult_record_list(ConsultRecordNode* head) {
    if (head == NULL) return 0;
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "consult_records.txt", "w");
    if (fp == NULL) return 0;

    // 写入表头
    fprintf(fp, "记录编号|患者编号|医生编号|预约编号|就诊时间|诊断结果|治疗意见|决策(0=未决�?1=门诊,2=住院,3=转院)|就诊前状�?0=未就�?1=待检�?2=待诊�?|就诊后状�?0=未完�?1=已完�?2=已转�?|星级评分(1-5�?0=未评�?|评价内容\n");

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

    // 跳过表头
    char header_buffer[512];
    if (fgets(header_buffer, sizeof(header_buffer), fp) == NULL) {
        fclose(fp);
        return 1;
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char record_id[MAX_ID_LEN], patient_id[MAX_ID_LEN], doctor_id[MAX_ID_LEN];
        char appointment_id[MAX_ID_LEN], consult_time[MAX_NAME_LEN];
        char diagnosis_text[MAX_RECORD_LEN], treatment_advice[MAX_RECORD_LEN];
        char feedback[MAX_RECORD_LEN];
        int decision, pre_status, post_status, star_rating;

        char* fields[15];
        int field_count = split_line_by_delimiter(line, '|', fields, 15);
        
        if (field_count < 12) continue;
        
        strcpy(record_id, fields[0]);
        strcpy(patient_id, fields[1]);
        strcpy(doctor_id, fields[2]);
        strcpy(appointment_id, fields[3]);
        strcpy(consult_time, fields[4]);
        strcpy(diagnosis_text, fields[5]);
        strcpy(treatment_advice, fields[6]);
        decision = atoi(fields[7]);
        pre_status = atoi(fields[8]);
        post_status = atoi(fields[9]);
        star_rating = atoi(fields[10]);
        strcpy(feedback, fields[11]);

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

    // 写入表头
    fprintf(fp, "检查项目编号|检查项目名称|所属科室|价格|医保类型(0=自费,1=甲类,2=乙类)\n");

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

    // 跳过表头
    char header_buffer[512];
    if (fgets(header_buffer, sizeof(header_buffer), fp) == NULL) {
        fclose(fp);
        return 1;
    }

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char item_id[MAX_ID_LEN], item_name[MAX_MED_NAME_LEN], dept[MAX_NAME_LEN];
        double price;
        int m_type;

        char* fields[10];
        int field_count = split_line_by_delimiter(line, '|', fields, 10);
        
        if (field_count < 5) continue;
        
        strcpy(item_id, fields[0]);
        strcpy(item_name, fields[1]);
        strcpy(dept, fields[2]);
        price = atof(fields[3]);
        m_type = atoi(fields[4]);

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

    // 写入表头
    fprintf(fp, "检查记录编号|患者编号|检查项目编号|检查项目名称|所属科室|检查时间|检查结果|是否完成(0=�?1=�?|是否缴费(0=�?1=�?\n");

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

    // 跳过表头
    char header_buffer[512];
    if (fgets(header_buffer, sizeof(header_buffer), fp) == NULL) {
        fclose(fp);
        return 1;
    }

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char record_id[MAX_ID_LEN], patient_id[MAX_ID_LEN], item_id[MAX_ID_LEN];
        char item_name[MAX_NAME_LEN], dept[MAX_NAME_LEN], check_time[MAX_NAME_LEN];
        char result[MAX_RECORD_LEN];
        int is_completed, is_paid;

        char* fields[15];
        int field_count = split_line_by_delimiter(line, '|', fields, 15);
        
        if (field_count < 9) continue;
        
        strcpy(record_id, fields[0]);
        strcpy(patient_id, fields[1]);
        strcpy(item_id, fields[2]);
        strcpy(item_name, fields[3]);
        strcpy(dept, fields[4]);
        strcpy(check_time, fields[5]);
        strcpy(result, fields[6]);
        is_completed = atoi(fields[7]);
        is_paid = atoi(fields[8]);

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

    // 写入表头
    fprintf(fp, "消息内容|时间\n");

    AlertNode* curr = head->next;
    while (curr != NULL) {
        // 将时间戳转换为可读格�?
        char time_str[20];
        struct tm* local_time = localtime(&curr->time);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
        fprintf(fp, "%s|%s\n", curr->message, time_str);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_alert_list(AlertNode** head) {
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "alerts.txt", "r");
    if (fp == NULL) return 0;

    // 跳过表头
    char header_buffer[512];
    if (fgets(header_buffer, sizeof(header_buffer), fp) == NULL) {
        fclose(fp);
        return 0;
    }

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char message[256];
        char time_str[20];
        time_t alert_time;

        char* last_pipe = strrchr(line, '|');
        if (last_pipe != NULL) {
            *last_pipe = '\0';
            strcpy(message, line);
            strcpy(time_str, last_pipe + 1);
            
            // 解析日期时间字符�?(格式: YYYY-MM-DD HH:MM:SS)
            struct tm tm = {0};
            if (sscanf(time_str, "%d-%d-%d %d:%d:%d", 
                       &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                       &tm.tm_hour, &tm.tm_min, &tm.tm_sec) == 6) {
                tm.tm_year -= 1900;  // 年份�?900开�?
                tm.tm_mon -= 1;      // 月份�?开�?
                alert_time = mktime(&tm);
            } else {
                // 解析失败，使用当前时�?
                alert_time = time(NULL);
            }
        } else {
            continue;
        }

        AlertNode* new_node = (AlertNode*)malloc(sizeof(AlertNode));
        if (new_node == NULL) continue;

        strncpy(new_node->message, message, 255);
        new_node->message[255] = '\0';
        new_node->time = alert_time;
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

    // 写入表头
    fprintf(fp, "投诉编号|患者编号|目标类型(1=医生,2=护士/前台,3=药师)|目标ID|目标名称|投诉内容|状�?0=待处�?1=已回�?|回复|提交时间\n");

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
    if (fp == NULL) return 0;

    // 跳过表头
    char header_buffer[512];
    if (fgets(header_buffer, sizeof(header_buffer), fp) == NULL) {
        fclose(fp);
        return 0;
    }

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

    fprintf(fp, "时间戳|操作|目标|描述\n");

    LogNode* curr = head->next;
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
    if (fp == NULL) return 0;

    *head = (LogNode*)malloc(sizeof(LogNode));
    if (*head == NULL) return 0;
    (*head)->next = NULL;

    char header_buffer[512];
    if (fgets(header_buffer, sizeof(header_buffer), fp) == NULL) {
        fclose(fp);
        return 0;
    }

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

        LogNode* curr = *head;
        while (curr->next != NULL) curr = curr->next;
        curr->next = new_node;
    }
    fclose(fp);
    return 1;
}

int save_inpatient_list(InpatientRecord* head) {
    if (head == NULL) return 0;
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "inpatients.txt", "w");
    if (fp == NULL) return 0;

    // 写入表头
    fprintf(fp, "住院记录编号|患者编号|床位号|初分床位号|病房类型(1=普�?2=ICU,3=隔离病房,4=单人病房)|推荐病房类型(1=普�?2=ICU,3=隔离病房,4=单人病房)|预计天数|已住天数|押金余额|是否活跃(0=�?1=�?|最近一次日结时间戳\n");

    InpatientRecord* curr = head->next;
    while (curr != NULL) {
        char time_str[30] = "";
        if (curr->last_settlement_time > 0) {
            time_t settlement_time = (time_t)curr->last_settlement_time;
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&settlement_time));
        } else {
            strcpy(time_str, "未日结");
        }
        fprintf(fp, "%s|%s|%s|%s|%d|%d|%d|%d|%.2f|%d|%s\n",
            curr->inpatient_id, curr->patient_id, curr->bed_id, curr->original_bed_id,
            curr->ward_type, curr->recommended_ward_type, curr->estimated_days,
            curr->days_stayed, curr->deposit_balance, curr->is_active,
            time_str);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_inpatient_list(InpatientRecord** head) {
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "inpatients.txt", "r");
    if (fp == NULL) return 0;

    // 跳过表头
    char header_buffer[512];
    if (fgets(header_buffer, sizeof(header_buffer), fp) == NULL) {
        fclose(fp);
        return 0;
    }

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        char inpatient_id[MAX_ID_LEN], patient_id[MAX_ID_LEN], bed_id[MAX_ID_LEN], original_bed_id[MAX_ID_LEN];
        int ward_type, recommended_ward_type, estimated_days, days_stayed, is_active;
        double deposit_balance;
        long last_settlement_time = 0;

        char* fields[15];
        int field_count = split_line_by_delimiter(line, '|', fields, 15);
        
        if (field_count < 11) continue;
        
        strcpy(inpatient_id, fields[0]);
        strcpy(patient_id, fields[1]);
        strcpy(bed_id, fields[2]);
        strcpy(original_bed_id, fields[3]);
        ward_type = atoi(fields[4]);
        recommended_ward_type = atoi(fields[5]);
        estimated_days = atoi(fields[6]);
        days_stayed = atoi(fields[7]);
        deposit_balance = atof(fields[8]);
        is_active = atoi(fields[9]);
        
        // 解析时间字段
        if (strcmp(fields[10], "未日结") != 0) {
            int year, month, day, hour, minute, second;
            if (sscanf(fields[10], "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6) {
                struct tm tm_time;
                tm_time.tm_year = year - 1900;  // 年份从1900开始
                tm_time.tm_mon = month - 1;      // 月份从0开始
                tm_time.tm_mday = day;
                tm_time.tm_hour = hour;
                tm_time.tm_min = minute;
                tm_time.tm_sec = second;
                tm_time.tm_isdst = -1;  // 自动检测夏令时
                last_settlement_time = (long)mktime(&tm_time);
            }
        }

        InpatientRecord* new_node = create_inpatient_record_node(
            inpatient_id, patient_id, bed_id, ward_type, recommended_ward_type,
            estimated_days, days_stayed, deposit_balance, is_active);
        if (new_node == NULL) continue;
        
        new_node->last_settlement_time = last_settlement_time;
        strncpy(new_node->original_bed_id, original_bed_id, MAX_ID_LEN - 1);
        new_node->original_bed_id[MAX_ID_LEN - 1] = '\0';

        insert_inpatient_record_tail(*head, new_node);
    }
    fclose(fp);
    return 1;
}

// 回收站列表持久化函数声明
int save_recycle_list(RecycleNode* head);
int load_recycle_list(RecycleNode** head);

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
    result &= save_recycle_list(g_recycle_list);
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
    result &= load_recycle_list(&g_recycle_list);
    return result;
}

// -----------------------------------------------------------------------------
// 保存回收站列�?
// -----------------------------------------------------------------------------
int save_recycle_list(RecycleNode* head)
{
    FILE* fp = fopen("data/recycle.txt", "w");
    if (fp == NULL)
    {
        return 0;
    }

    // 写入表头
    fprintf(fp, "回收站编号|类型|原编号|名称|删除时间|操作人|原因|是否恢复|备份数据\n");

    RecycleNode* curr = head->next;
    while (curr != NULL)
    {
        fprintf(fp, "%s|%d|%s|%s|%s|%s|%s|%d|",
                curr->recycle_id,
                curr->type,
                curr->source_id,
                curr->source_name,
                curr->delete_time,
                curr->deleted_by,
                curr->reason,
                curr->is_restored);

        // 根据类型写入不同的备份数据
        switch (curr->type)
        {
            case RECYCLE_MEDICINE:
                fprintf(fp, "%s|%s|%s|%s|%.2f|%d|%d|%s\n",
                        curr->medicine_backup.id,
                        curr->medicine_backup.name,
                        curr->medicine_backup.alias,
                        curr->medicine_backup.generic_name,
                        curr->medicine_backup.price,
                        curr->medicine_backup.stock,
                        curr->medicine_backup.m_type,
                        curr->medicine_backup.expiry_date);
                break;

            case RECYCLE_CHECK_ITEM:
                fprintf(fp, "%s|%s|%s|%.2f|%d\n",
                        curr->check_item_backup.item_id,
                        curr->check_item_backup.item_name,
                        curr->check_item_backup.dept,
                        curr->check_item_backup.price,
                        curr->check_item_backup.m_type);
                break;

            case RECYCLE_WARD:
                fprintf(fp, "%s|%s|%d|%s|%d|%s\n",
                        curr->ward_backup.room_id,
                        curr->ward_backup.bed_id,
                        curr->ward_backup.ward_type,
                        curr->ward_backup.dept,
                        curr->ward_backup.is_occupied,
                        curr->ward_backup.patient_id);
                break;

            case RECYCLE_ACCOUNT:
                fprintf(fp, "%s|%s|%s|%s|%d|%d\n",
                        curr->account_backup.username,
                        curr->account_backup.password,
                        curr->account_backup.real_name,
                        curr->account_backup.gender,
                        curr->account_backup.role,
                        curr->account_backup.is_on_duty);
                break;

            default:
                fprintf(fp, "\n");
                break;
        }

        curr = curr->next;
    }

    fclose(fp);
    return 1;
}

// -----------------------------------------------------------------------------
// 加载回收站列表
// -----------------------------------------------------------------------------
int load_recycle_list(RecycleNode** head)
{
    FILE* fp = fopen("data/recycle.txt", "r");
    if (fp == NULL)
    {
        return 1;
    }

    char line[4096];
    int line_num = 0;

    if (fgets(line, sizeof(line), fp) == NULL)
    {
        fclose(fp);
        return 1;
    }

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        line_num++;

        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }

        if (line[0] == '\0')
        {
            continue;
        }

        char* fields[20] = { NULL };
        int field_count = split_line_by_delimiter(line, '|', fields, 20);

        if (field_count < 8)
        {
            continue;
        }

        int type = atoi(fields[1]);

        if (strlen(fields[0]) == 0 || strlen(fields[2]) == 0)
        {
            continue;
        }

        RecycleNode* new_node = (RecycleNode*)malloc(sizeof(RecycleNode));
        if (new_node == NULL)
        {
            continue;
        }

        memset(new_node, 0, sizeof(RecycleNode));

        strncpy(new_node->recycle_id, fields[0], sizeof(new_node->recycle_id) - 1);
        new_node->recycle_id[sizeof(new_node->recycle_id) - 1] = '\0';

        new_node->type = type;

        strncpy(new_node->source_id, fields[2], sizeof(new_node->source_id) - 1);
        new_node->source_id[sizeof(new_node->source_id) - 1] = '\0';

        strncpy(new_node->source_name, fields[3], sizeof(new_node->source_name) - 1);
        new_node->source_name[sizeof(new_node->source_name) - 1] = '\0';

        strncpy(new_node->delete_time, fields[4], sizeof(new_node->delete_time) - 1);
        new_node->delete_time[sizeof(new_node->delete_time) - 1] = '\0';

        strncpy(new_node->deleted_by, fields[5], sizeof(new_node->deleted_by) - 1);
        new_node->deleted_by[sizeof(new_node->deleted_by) - 1] = '\0';

        strncpy(new_node->reason, fields[6], sizeof(new_node->reason) - 1);
        new_node->reason[sizeof(new_node->reason) - 1] = '\0';

        new_node->is_restored = atoi(fields[7]);

        switch (type)
        {
            case RECYCLE_MEDICINE:
                if (field_count < 16) { free(new_node); continue; }
                strncpy(new_node->medicine_backup.id, fields[8], sizeof(new_node->medicine_backup.id) - 1);
                new_node->medicine_backup.id[sizeof(new_node->medicine_backup.id) - 1] = '\0';
                strncpy(new_node->medicine_backup.name, fields[9], sizeof(new_node->medicine_backup.name) - 1);
                new_node->medicine_backup.name[sizeof(new_node->medicine_backup.name) - 1] = '\0';
                strncpy(new_node->medicine_backup.alias, fields[10], sizeof(new_node->medicine_backup.alias) - 1);
                new_node->medicine_backup.alias[sizeof(new_node->medicine_backup.alias) - 1] = '\0';
                strncpy(new_node->medicine_backup.generic_name, fields[11], sizeof(new_node->medicine_backup.generic_name) - 1);
                new_node->medicine_backup.generic_name[sizeof(new_node->medicine_backup.generic_name) - 1] = '\0';
                new_node->medicine_backup.price = atof(fields[12]);
                new_node->medicine_backup.stock = atoi(fields[13]);
                new_node->medicine_backup.m_type = atoi(fields[14]);
                strncpy(new_node->medicine_backup.expiry_date, fields[15], sizeof(new_node->medicine_backup.expiry_date) - 1);
                new_node->medicine_backup.expiry_date[sizeof(new_node->medicine_backup.expiry_date) - 1] = '\0';
                new_node->medicine_backup.prev = NULL;
                new_node->medicine_backup.next = NULL;
                break;

            case RECYCLE_CHECK_ITEM:
                if (field_count < 13) { free(new_node); continue; }
                strncpy(new_node->check_item_backup.item_id, fields[8], sizeof(new_node->check_item_backup.item_id) - 1);
                new_node->check_item_backup.item_id[sizeof(new_node->check_item_backup.item_id) - 1] = '\0';
                strncpy(new_node->check_item_backup.item_name, fields[9], sizeof(new_node->check_item_backup.item_name) - 1);
                new_node->check_item_backup.item_name[sizeof(new_node->check_item_backup.item_name) - 1] = '\0';
                strncpy(new_node->check_item_backup.dept, fields[10], sizeof(new_node->check_item_backup.dept) - 1);
                new_node->check_item_backup.dept[sizeof(new_node->check_item_backup.dept) - 1] = '\0';
                new_node->check_item_backup.price = atof(fields[11]);
                new_node->check_item_backup.m_type = atoi(fields[12]);
                new_node->check_item_backup.prev = NULL;
                new_node->check_item_backup.next = NULL;
                break;

            case RECYCLE_WARD:
                if (field_count < 14) { free(new_node); continue; }
                strncpy(new_node->ward_backup.room_id, fields[8], sizeof(new_node->ward_backup.room_id) - 1);
                new_node->ward_backup.room_id[sizeof(new_node->ward_backup.room_id) - 1] = '\0';
                strncpy(new_node->ward_backup.bed_id, fields[9], sizeof(new_node->ward_backup.bed_id) - 1);
                new_node->ward_backup.bed_id[sizeof(new_node->ward_backup.bed_id) - 1] = '\0';
                new_node->ward_backup.ward_type = atoi(fields[10]);
                strncpy(new_node->ward_backup.dept, fields[11], sizeof(new_node->ward_backup.dept) - 1);
                new_node->ward_backup.dept[sizeof(new_node->ward_backup.dept) - 1] = '\0';
                new_node->ward_backup.is_occupied = atoi(fields[12]);
                strncpy(new_node->ward_backup.patient_id, fields[13], sizeof(new_node->ward_backup.patient_id) - 1);
                new_node->ward_backup.patient_id[sizeof(new_node->ward_backup.patient_id) - 1] = '\0';
                new_node->ward_backup.prev = NULL;
                new_node->ward_backup.next = NULL;
                break;

            case RECYCLE_ACCOUNT:
                if (field_count < 14) { free(new_node); continue; }
                strncpy(new_node->account_backup.username, fields[8], sizeof(new_node->account_backup.username) - 1);
                new_node->account_backup.username[sizeof(new_node->account_backup.username) - 1] = '\0';
                strncpy(new_node->account_backup.password, fields[9], sizeof(new_node->account_backup.password) - 1);
                new_node->account_backup.password[sizeof(new_node->account_backup.password) - 1] = '\0';
                strncpy(new_node->account_backup.real_name, fields[10], sizeof(new_node->account_backup.real_name) - 1);
                new_node->account_backup.real_name[sizeof(new_node->account_backup.real_name) - 1] = '\0';
                strncpy(new_node->account_backup.gender, fields[11], sizeof(new_node->account_backup.gender) - 1);
                new_node->account_backup.gender[sizeof(new_node->account_backup.gender) - 1] = '\0';
                new_node->account_backup.role = atoi(fields[12]);
                new_node->account_backup.is_on_duty = atoi(fields[13]);
                new_node->account_backup.error_count = 0;
                new_node->account_backup.lock_time = 0;
                new_node->account_backup.prev = NULL;
                new_node->account_backup.next = NULL;
                break;

            default:
                free(new_node);
                continue;
        }

        new_node->prev = NULL;
        new_node->next = NULL;

        RecycleNode* tail = *head;
        while (tail->next != NULL)
        {
            tail = tail->next;
        }
        tail->next = new_node;
        new_node->prev = tail;
    }

    fclose(fp);
    return 1;
}