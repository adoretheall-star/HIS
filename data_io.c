#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include "global.h"
#include "list_ops.h"

// 调试开关，开启
#define DATA_IO_DEBUG 1

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
    fprintf(fp, "患者编号|姓名|性别|年龄|身份证号|医保类型(0=无,1=甲类,2=乙类)|症状|目标科室|医生编号|就诊卡号|余额|状态(1=待诊,2=检查中,3=检查后待复诊,4=已看诊待缴费,5=已缴费待取药,6=住院中,7=就诊结束,8=过号作废)|诊断结果|治疗意见|处方|处方数量|爽约时间1|爽约时间2|爽约时间3|爽约次数|黑名单过期时间|是否黑名单(0=否,1=是)|是否急诊(0=否,1=是)|排队时间|呼叫次数|急诊欠费|欠费时间\n");

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
    printf("DEBUG: 正在读取患者文件: %s\n", file_path);
    FILE* fp = fopen(file_path, "r");
    if (fp == NULL) {
        printf("DEBUG: 无法打开患者文件: %s\n", file_path);
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
            printf("DEBUG: 字段数不足，跳过此行: %s (字段数: %d)\n", line, field_count);
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
        missed_time_1 = atol(fields[15]);
        missed_time_2 = atol(fields[16]);
        missed_time_3 = atol(fields[17]);
        missed_count = atoi(fields[18]);
        blacklist_expire = atol(fields[19]);
        is_blacklisted = atoi(fields[20]);
        is_emergency = atoi(fields[21]);
        queue_time = atol(fields[22]);
        call_count = atol(fields[23]);
        emergency_debt = atof(fields[24]);
        unpaid_time = atol(fields[25]);

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
    fprintf(fp, "预约编号|患者编号|预约日期|预约时段|医生编号|科室|状态(1=待确认,2=已确认,3=已完成,4=已取消)|挂号费|是否缴费(0=否,1=是)|是否现场挂号(0=否,1=是)\n");

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
    fprintf(fp, "医生编号|姓名|性别|科室|排队长度|是否值班(0=否,1=是)\n");

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
    fprintf(fp, "病房号|床位号|病房类型(1=普通,2=ICU,3=隔离病房,4=单人病房)|是否占用(0=否,1=是)|患者编号\n");

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

        char room_id[MAX_ID_LEN], bed_id[MAX_ID_LEN], patient_id[MAX_ID_LEN];
        int ward_type, is_occupied;

        char* fields[10];
        int field_count = split_line_by_delimiter(line, '|', fields, 10);
        
        if (field_count < 5) continue;
        
        strcpy(room_id, fields[0]);
        strcpy(bed_id, fields[1]);
        ward_type = atoi(fields[2]);
        is_occupied = atoi(fields[3]);
        strcpy(patient_id, fields[4]);

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

    // 写入表头
    fprintf(fp, "用户名|密码|真实姓名|性别|角色(0=管理员,1=医生,2=护士,3=药师)|错误次数|锁定时间|是否值班(0=否,1=是)\n");

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
    fprintf(fp, "记录编号|患者编号|医生编号|预约编号|就诊时间|诊断结果|治疗意见|决策(0=未决定,1=门诊,2=住院,3=转院)|就诊前状态(0=未就诊,1=待检查,2=待诊断)|就诊后状态(0=未完成,1=已完成,2=已转诊)|星级评分(1-5星,0=未评价)|评价内容\n");

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

        char item_id[MAX_ID_LEN], item_name[MAX_NAME_LEN], dept[MAX_NAME_LEN];
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
    fprintf(fp, "检查记录编号|患者编号|检查项目编号|检查项目名称|所属科室|检查时间|检查结果|是否完成(0=否,1=是)|是否缴费(0=否,1=是)\n");

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
        // 将时间戳转换为可读格式
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

        char message[256];
        char time_str[20];
        time_t alert_time;

        char* last_pipe = strrchr(line, '|');
        if (last_pipe != NULL) {
            *last_pipe = '\0';
            strcpy(message, line);
            strcpy(time_str, last_pipe + 1);
            
            // 解析日期时间字符串 (格式: YYYY-MM-DD HH:MM:SS)
            struct tm tm = {0};
            if (sscanf(time_str, "%d-%d-%d %d:%d:%d", 
                       &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                       &tm.tm_hour, &tm.tm_min, &tm.tm_sec) == 6) {
                tm.tm_year -= 1900;  // 年份从1900开始
                tm.tm_mon -= 1;      // 月份从0开始
                alert_time = mktime(&tm);
            } else {
                // 解析失败，使用当前时间
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
    fprintf(fp, "投诉编号|患者编号|目标类型(1=医生,2=护士/前台,3=药师)|目标ID|目标名称|投诉内容|状态(0=待处理,1=已回复)|回复|提交时间\n");

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
    if (head == NULL) return 1;
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "logs.txt", "w");
    if (fp == NULL) return 0;

    // 写入表头
    fprintf(fp, "时间戳|操作|目标|描述\n");

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

    // 写入表头
    fprintf(fp, "住院记录编号|患者编号|床位号|病房类型(1=普通,2=ICU,3=隔离病房,4=单人病房)|推荐病房类型(1=普通,2=ICU,3=隔离病房,4=单人病房)|预计天数|已住天数|押金余额|是否活跃(0=否,1=是)\n");

    InpatientRecord* curr = head->next;
    while (curr != NULL) {
        fprintf(fp, "%s|%s|%s|%d|%d|%d|%d|%.2f|%d|%ld\n",
            curr->inpatient_id, curr->patient_id, curr->bed_id,
            curr->ward_type, curr->recommended_ward_type, curr->estimated_days,
            curr->days_stayed, curr->deposit_balance, curr->is_active,
            (long)curr->last_settlement_time);
        curr = curr->next;
    }
    fclose(fp);
    return 1;
}

int load_inpatient_list(InpatientRecord** head) {
    ensure_data_dir();
    FILE* fp = fopen(DATA_DIR "inpatients.txt", "r");
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

        char inpatient_id[MAX_ID_LEN], patient_id[MAX_ID_LEN], bed_id[MAX_ID_LEN];
        int ward_type, recommended_ward_type, estimated_days, days_stayed, is_active;
        double deposit_balance;
        long last_settlement_time;

        char* fields[15];
        int field_count = split_line_by_delimiter(line, '|', fields, 15);
        
        if (field_count < 10) continue;
        
        strcpy(inpatient_id, fields[0]);
        strcpy(patient_id, fields[1]);
        strcpy(bed_id, fields[2]);
        ward_type = atoi(fields[3]);
        recommended_ward_type = atoi(fields[4]);
        estimated_days = atoi(fields[5]);
        days_stayed = atoi(fields[6]);
        deposit_balance = atof(fields[7]);
        is_active = atoi(fields[8]);
        last_settlement_time = atol(fields[9]);

        InpatientRecord* new_node = create_inpatient_record_node(
            inpatient_id, patient_id, bed_id, ward_type, recommended_ward_type,
            estimated_days, days_stayed, deposit_balance, is_active);
        if (new_node == NULL) continue;
        
        new_node->last_settlement_time = last_settlement_time;

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