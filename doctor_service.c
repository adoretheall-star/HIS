// ==========================================
// 文件名: doctor_service.c
// 作用: 医生接诊相关业务层实现
// ==========================================
#define _CRT_SECURE_NO_WARNINGS

// 解决pragma pack编译问题
#pragma pack(push, 8)

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "global.h"
#include "list_ops.h"
#include "appointment.h"
#include "doctor_service.h"
#include "utils.h"

#pragma pack(pop)





// 查找医生匹配的最佳预约
static AppointmentNode* find_best_appointment_for_doctor(const char* patient_id, const DoctorNode* doctor)
{
    AppointmentNode* curr = NULL;
    AppointmentNode* last_any = NULL;
    AppointmentNode* last_matched = NULL;
    
    if (g_appointment_list == NULL || patient_id == NULL || patient_id[0] == '\0' || doctor == NULL)
    {
        return NULL;
    }
    
    curr = g_appointment_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->patient_id, patient_id) == 0)
        {
            // 记录该患者的最后一条任意预约
            last_any = curr;
            
            // 判断是否与当前医生匹配
            int is_matched = 0;
            
            // 如果 appoint_doctor == 当前医生编号，则算匹配
            if (strlen(curr->appoint_doctor) > 0 && strcmp(curr->appoint_doctor, doctor->id) == 0)
            {
                is_matched = 1;
            }
            // 或者 appoint_doctor 为空，但 appoint_dept == 当前医生科室，也算匹配
            else if (strlen(curr->appoint_doctor) == 0 && strlen(curr->appoint_dept) > 0 && strcmp(curr->appoint_dept, doctor->department) == 0)
            {
                is_matched = 1;
            }
            
            if (is_matched)
            {
                last_matched = curr;
            }
        }
        curr = curr->next;
    }
    
    // 如果找到和当前医生匹配的预约，就返回那条
    if (last_matched != NULL)
    {
        return last_matched;
    }
    
    // 如果没有找到匹配的，就返回该患者最后一条任意预约
    return last_any;
}

// 获取诊疗决策文字
static const char* get_decision_text(int decision)
{
    switch (decision)
    {
        case 1: return "结束就诊";
        case 2: return "开药";
        case 3: return "开检查";
        case 4: return "办理住院";
        default: return "未知决策";
    }
}

// 查找患者最新的接诊记录
static ConsultRecordNode* find_latest_consult_record_for_patient(const char* patient_id, const char* doctor_id)
{
    ConsultRecordNode* curr = NULL;
    ConsultRecordNode* latest = NULL;
    
    if (g_consult_record_list == NULL || patient_id == NULL || doctor_id == NULL)
        return NULL;
    
    curr = g_consult_record_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->patient_id, patient_id) == 0 && strcmp(curr->doctor_id, doctor_id) == 0)
        {
            latest = curr;
        }
        curr = curr->next;
    }
    
    return latest;
}

static void generate_consult_record_id(char* record_id, size_t size)
{
    time_t now;
    struct tm* local_time;
    char time_part[20];
    int max_suffix = 0;
    ConsultRecordNode* curr = NULL;

    if (record_id == NULL || size == 0)
    {
        return;
    }

    time(&now);
    local_time = localtime(&now);
    strftime(time_part, sizeof(time_part), "%Y%m%d%H%M%S", local_time);

    if (g_consult_record_list != NULL)
    {
        curr = g_consult_record_list->next;
        while (curr != NULL)
        {
            char existing_time_part[20];
            int suffix = 0;

            if (sscanf(curr->record_id, "CR-%19[^-]-%d", existing_time_part, &suffix) == 2 &&
                strcmp(existing_time_part, time_part) == 0 &&
                suffix > max_suffix)
            {
                max_suffix = suffix;
            }
            curr = curr->next;
        }
    }

    snprintf(record_id, size, "CR-%s-%02d", time_part, max_suffix + 1);
}

// 就诊状态转文字
static const char* get_med_status_text(MedStatus status)
{
    switch (status)
    {
        case STATUS_PENDING: return "待诊";
        case STATUS_EXAMINING: return "检查中";
        case STATUS_RECHECK_PENDING: return "检查后待复诊";
        case STATUS_UNPAID: return "已看诊待缴费";
        case STATUS_WAIT_MED: return "已缴费待取药";
        case STATUS_NEED_HOSPITALIZE: return "需住院待登记";
        case STATUS_HOSPITALIZED: return "住院中";
        case STATUS_COMPLETED: return "就诊结束";
        case STATUS_NO_SHOW: return "过号作废";
        default: return "未知状态";
    }
}

// 查找医生并做基础校验
static DoctorNode* get_doctor_by_id_checked(const char* doctor_id)
{
    DoctorNode* doctor = NULL;

    if (g_doctor_list == NULL)
    {
        printf("⚠️ 医生链表尚未初始化！\n");
        return NULL;
    }

    if (doctor_id == NULL || strlen(doctor_id) == 0)
    {
        printf("⚠️ 医生编号不能为空！\n");
        return NULL;
    }

    doctor = find_doctor_by_id(g_doctor_list, doctor_id);
    if (doctor == NULL)
    {
        printf("⚠️ 未找到对应医生，操作失败！\n");
        return NULL;
    }

    return doctor;
}

// 判断患者是否归属于当前医生
static int is_patient_match_doctor(const PatientNode* patient, const DoctorNode* doctor)
{
    AppointmentNode* curr = NULL;

    if (patient == NULL || doctor == NULL) return 0;

    if (strcmp(patient->doctor_id, doctor->id) == 0)
    {
        return 1;
    }

    if (strlen(patient->doctor_id) == 0 &&
        strlen(patient->target_dept) > 0 &&
        strcmp(patient->target_dept, doctor->department) == 0)
    {
        return 1;
    }

    if (g_appointment_list == NULL) return 0;

    curr = g_appointment_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->patient_id, patient->id) == 0 &&
            curr->appointment_status == CHECKED_IN)
        {
            if (strcmp(curr->appoint_doctor, doctor->id) == 0)
            {
                return 1;
            }

            if (strlen(curr->appoint_doctor) == 0 &&
                strcmp(curr->appoint_dept, doctor->department) == 0)
            {
                return 1;
            }
        }
        curr = curr->next;
    }

    return 0;
}

// 查看指定医生的待诊患者
// 获取指定医生的待诊患者列表（返回患者数量）
// 获取指定医生的待诊患者链表
PatientPtrNode* get_waiting_patients_by_doctor(const char* doctor_id)
{
    PatientPtrNode* head = NULL;
    PatientPtrNode* tail = NULL;
    DoctorNode* doctor = get_doctor_by_id_checked(doctor_id);
    if (!doctor || !g_patient_list) return NULL;
    int is_emergency_dept = (strcmp(doctor->department, "急诊科") == 0);

    // 1. 全量抓取符合条件的患者
    PatientNode* curr = g_patient_list->next;
    while (curr != NULL) {
        if ((curr->status == STATUS_PENDING || curr->status == STATUS_RECHECK_PENDING) &&
            is_patient_match_doctor(curr, doctor)) {
            // 创建新的患者指针节点
            PatientPtrNode* new_node = (PatientPtrNode*)malloc(sizeof(PatientPtrNode));
            if (new_node == NULL) {
                continue;
            }
            new_node->patient = curr;
            new_node->next = NULL;

            // 添加到链表
            if (head == NULL) {
                head = new_node;
                tail = new_node;
            } else {
                tail->next = new_node;
                tail = new_node;
            }
        }
        curr = curr->next;
    }

    // 2. 链表冒泡排序
    if (head != NULL) {
        int swapped;
        do {
            swapped = 0;
            PatientPtrNode* prev = NULL;
            PatientPtrNode* curr = head;
            PatientPtrNode* next = curr->next;

            while (next != NULL) {
                int swap = 0;
                if (is_emergency_dept) {
                    // 急诊科：急诊优先；同级别则比时间
                    if (curr->patient->is_emergency < next->patient->is_emergency) swap = 1;
                    else if (curr->patient->is_emergency == next->patient->is_emergency && 
                             curr->patient->queue_time > next->patient->queue_time) swap = 1;
                } else {
                    // 普通门诊：绝对按签到时间排队
                    if (curr->patient->queue_time > next->patient->queue_time) swap = 1;
                }

                if (swap) {
                    // 交换节点
                    if (prev == NULL) {
                        head = next;
                    } else {
                        prev->next = next;
                    }
                    curr->next = next->next;
                    next->next = curr;
                    
                    // 调整指针
                    prev = next;
                    next = curr->next;
                    swapped = 1;
                } else {
                    prev = curr;
                    curr = next;
                    next = next->next;
                }
            }
        } while (swapped);
    }

    return head;
}

// 获取指定医生的已处理患者列表（返回患者数量）
PatientPtrNode* get_processed_patients_by_doctor(const char* doctor_id)
{
    PatientPtrNode* head = NULL;
    PatientPtrNode* tail = NULL;
    PatientNode* curr = NULL;

    if (doctor_id == NULL)
    {
        return NULL;
    }

    if (g_patient_list == NULL)
    {
        return NULL;
    }

    curr = g_patient_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->doctor_id, doctor_id) == 0)
        {
            // 只把以下状态视为"已接诊患者/已处理患者"：
            // STATUS_EXAMINING, STATUS_UNPAID, STATUS_WAIT_MED, STATUS_NEED_HOSPITALIZE, STATUS_HOSPITALIZED, STATUS_COMPLETED
            if (curr->status == STATUS_EXAMINING ||
                curr->status == STATUS_UNPAID ||
                curr->status == STATUS_WAIT_MED ||
                curr->status == STATUS_NEED_HOSPITALIZE ||
                curr->status == STATUS_HOSPITALIZED ||
                curr->status == STATUS_COMPLETED)
            {
                // 创建新的患者指针节点
                PatientPtrNode* new_node = (PatientPtrNode*)malloc(sizeof(PatientPtrNode));
                if (new_node == NULL)
                {
                    continue;
                }
                new_node->patient = curr;
                new_node->next = NULL;

                // 添加到链表
                if (head == NULL)
                {
                    head = new_node;
                    tail = new_node;
                }
                else
                {
                    tail->next = new_node;
                    tail = new_node;
                }
            }
        }
        curr = curr->next;
    }

    return head;
}

// 释放患者指针链表
void free_patient_ptr_list(PatientPtrNode* head)
{
    PatientPtrNode* curr = head;
    while (curr != NULL)
    {
        PatientPtrNode* temp = curr;
        curr = curr->next;
        free(temp);
    }
}

// 获取患者指针链表的长度
int get_patient_ptr_list_count(PatientPtrNode* head)
{
    int count = 0;
    PatientPtrNode* curr = head;
    while (curr != NULL)
    {
        count++;
        curr = curr->next;
    }
    return count;
}

// 获取患者指针链表中第index个节点（从1开始）
PatientNode* get_nth_patient_from_ptr_list(PatientPtrNode* head, int index)
{
    if (index < 1) return NULL;
    
    int current = 1;
    PatientPtrNode* curr = head;
    while (curr != NULL && current < index)
    {
        curr = curr->next;
        current++;
    }
    
    return curr ? curr->patient : NULL;
}

// 在患者指针链表中查找指定患者的位置（从1开始）
int find_patient_position_in_ptr_list(PatientPtrNode* head, PatientNode* patient)
{
    int position = 1;
    PatientPtrNode* curr = head;
    while (curr != NULL)
    {
        if (curr->patient == patient)
        {
            return position;
        }
        curr = curr->next;
        position++;
    }
    return -1; // 未找到
}

// 获取患者指针链表中指定位置之后的第n个节点
PatientNode* get_patient_after_position_in_ptr_list(PatientPtrNode* head, int position, int n)
{
    int current = 1;
    PatientPtrNode* curr = head;
    
    // 先找到指定位置的节点
    while (curr != NULL && current < position)
    {
        curr = curr->next;
        current++;
    }
    
    // 再往后移动n个位置
    if (curr != NULL)
    {
        for (int i = 0; i < n && curr->next != NULL; i++)
        {
            curr = curr->next;
        }
        return curr->patient;
    }
    
    return NULL;
}

void show_waiting_patients_by_doctor(const char* doctor_id)
{
    int found = 0;
    PatientPtrNode* waiting_list = NULL;
    DoctorNode* doctor = get_doctor_by_id_checked(doctor_id);
    int index = 1;

    if (doctor == NULL) return;

    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化！\n");
        return;
    }

    printf("\n================ 医生待诊列表 ================\n");
    printf("医生编号: %s  医生姓名: %s  性别: %s  科室: %s\n",
        doctor->id, doctor->name, strlen(doctor->gender) > 0 ? doctor->gender : "未设置", doctor->department);

    // 使用 get_waiting_patients_by_doctor 获取排序后的待诊患者链表
    waiting_list = get_waiting_patients_by_doctor(doctor_id);
    
    PatientPtrNode* curr = waiting_list;
    while (curr != NULL)
    {
        PatientNode* patient = curr->patient;
        if ((patient->status == STATUS_PENDING || patient->status == STATUS_RECHECK_PENDING) && is_patient_match_doctor(patient, doctor))
        {
            const char* brief_text = strlen(patient->symptom) > 0 ? patient->symptom : patient->target_dept;
            const char* visit_type = patient->status == STATUS_PENDING ? "[初诊]" : "[复诊]";
            printf("[%d] %s 患者编号: %s | 姓名: %s | 性别: %s | 年龄: %d | 症状/科室: %s | 状态: %s\n",
                index++,
                visit_type,
                patient->id,
                patient->name,
                patient->gender,
                patient->age,
                strlen(brief_text) > 0 ? brief_text : "暂无",
                get_med_status_text(patient->status));
            found = 1;
        }
        curr = curr->next;
    }

    // 释放临时链表
    free_patient_ptr_list(waiting_list);

    if (!found)
    {
        printf("ℹ️ 当前没有可接诊的待诊患者。\n");
    }
}

// 医生接诊并做最小诊疗决策
int doctor_consult_patient(
    const char* doctor_id,
    const char* patient_id,
    int decision,
    const char* diagnosis_text,
    const char* treatment_advice
)
{
    DoctorNode* doctor = NULL;
    PatientNode* patient = NULL;

    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化，无法接诊！\n");
        return 0;
    }

    doctor = get_doctor_by_id_checked(doctor_id);
    if (doctor == NULL) return 0;

    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("⚠️ 患者编号不能为空！\n");
        return 0;
    }

    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到对应患者，无法接诊！\n");
        return 0;
    }

    if (patient->status != STATUS_PENDING && patient->status != STATUS_RECHECK_PENDING)
    {
        printf("⚠️ 当前患者状态为[%s]，不属于待诊状态！\n", get_med_status_text(patient->status));
        return 0;
    }

    if (!is_patient_match_doctor(patient, doctor))
    {
        printf("⚠️ 当前患者未分配给该医生，或预约目标与该医生不匹配！\n");
        return 0;
    }

    // 保存接诊前状态
    MedStatus pre_status = patient->status;

    if (diagnosis_text != NULL)
    {
        strncpy(patient->diagnosis_text, diagnosis_text, MAX_RECORD_LEN - 1);
        patient->diagnosis_text[MAX_RECORD_LEN - 1] = '\0';
    }

    if (treatment_advice != NULL)
    {
        strncpy(patient->treatment_advice, treatment_advice, MAX_RECORD_LEN - 1);
        patient->treatment_advice[MAX_RECORD_LEN - 1] = '\0';
    }

    switch (decision)
    {
        case 1:
            if (patient->script_head != NULL)
            {
                patient->status = STATUS_WAIT_MED;
            }
            else
            {
                patient->status = STATUS_COMPLETED;
            }
            break;
        case 2:
            patient->status = STATUS_UNPAID;
            patient->unpaid_time = time(NULL); // ⏱️ 补丁：按下72小时作废的秒表！
            break;
        case 3:
            patient->status = STATUS_UNPAID;
            patient->unpaid_time = time(NULL);
            break;
        case 4:
            patient->status = STATUS_HOSPITALIZED;
            break;
        default:
            printf("⚠️ 无效的诊疗决策，操作取消！\n");
            return 0;
    }

    // 详细展示接诊结果
    printf("\n================ 接诊结果 ================\n");
    printf("医生信息：%s（编号：%s）\n", doctor->name, doctor->id);
    printf("患者信息：%s（编号：%s）\n", patient->name, patient->id);
    printf("本次诊断结论：%s\n", diagnosis_text != NULL && strlen(diagnosis_text) > 0 ? diagnosis_text : "暂无");
    printf("本次处理意见：%s\n", treatment_advice != NULL && strlen(treatment_advice) > 0 ? treatment_advice : "暂无");
    
    // 诊疗决策对应的文字
    const char* decision_text = "";
    switch (decision)
    {
        case 1: decision_text = "结束就诊"; break;
        case 2: decision_text = "开药"; break;
        case 3: decision_text = "开检查"; break;
        case 4: decision_text = "办理住院"; break;
    }
    printf("诊疗决策：%s\n", decision_text);
    printf("患者新状态：%s\n", get_med_status_text(patient->status));
    printf("========================================\n");

    // 方案A：先将患者分配给当前医生，确保队列人数正确减少
    strncpy(patient->doctor_id, doctor->id, MAX_ID_LEN - 1);
    patient->doctor_id[MAX_ID_LEN - 1] = '\0';
    patient->call_count = 0;
    
    if (doctor->queue_length > 0)
    {
        doctor->queue_length--;
    }

    // 更新患者的预约记录状态（联动更新）
    // 优先匹配：当前已签到 + 当前医生匹配或当前科室匹配
    AppointmentNode* curr_appointment = NULL;
    AppointmentNode* matched_appointment = NULL;
    AppointmentNode* latest_valid_appointment = NULL;
    char appointment_id[MAX_ID_LEN] = {0};
    
    if (g_appointment_list != NULL)
    {
        curr_appointment = g_appointment_list->next;
        while (curr_appointment != NULL)
        {
            if (strcmp(curr_appointment->patient_id, patient_id) == 0)
            {
                // 记录最新的有效预约（不管状态）
                latest_valid_appointment = curr_appointment;
                
                // 优先匹配：已签到 + 医生或科室匹配
                if (curr_appointment->appointment_status == CHECKED_IN)
                {
                    int doctor_match = (strlen(curr_appointment->appoint_doctor) > 0 && 
                                       strcmp(curr_appointment->appoint_doctor, doctor->id) == 0);
                    int dept_match = (strlen(curr_appointment->appoint_dept) > 0 && 
                                     strcmp(curr_appointment->appoint_dept, doctor->department) == 0);
                    
                    if (doctor_match || dept_match)
                    {
                        matched_appointment = curr_appointment;
                        break;
                    }
                }
            }
            curr_appointment = curr_appointment->next;
        }
        
        // 如果没找到匹配的，使用已签到的最新预约
        if (matched_appointment == NULL && latest_valid_appointment != NULL)
        {
            if (latest_valid_appointment->appointment_status == CHECKED_IN)
            {
                matched_appointment = latest_valid_appointment;
            }
        }
        
        // 如果找到了匹配的预约，更新其状态
        if (matched_appointment != NULL)
        {
            strncpy(appointment_id, matched_appointment->appointment_id, MAX_ID_LEN - 1);
            appointment_id[MAX_ID_LEN - 1] = '\0';
            
            // 根据诊疗决策更新预约状态
            switch (decision)
            {
                case 1: // 结束就诊
                    matched_appointment->appointment_status = CHECKED_IN;
                    break;
                case 2: // 开药
                    matched_appointment->appointment_status = CHECKED_IN;
                    break;
                case 3: // 开检查
                    matched_appointment->appointment_status = CHECKED_IN;
                    break;
                case 4: // 办理住院
                    matched_appointment->appointment_status = CHECKED_IN;
                    break;
            }
            
            // 根据是否现场挂号显示不同提示
            if (matched_appointment->is_walk_in == 1)
            {
                printf("\n✅ 接诊成功！患者为现场挂号\n");
            }
            else
            {
                printf("\n✅ 接诊成功！患者为预约挂号\n");
            }
        }
        else
        {
            // 没有找到匹配的预约，仍然允许接诊但不绑定预约
            if (patient->status == STATUS_RECHECK_PENDING)
            {
                printf("\nℹ️ 患者为检查后待复诊，已完成接诊\n");
            }
            else if (patient->is_emergency == 1)
            {
                printf("\nℹ️ 患者为急诊通道就诊，已完成接诊\n");
            }
            else
            {
                printf("\nℹ️ 患者无预约记录，已完成接诊\n");
            }
        }
    }

    char record_id[MAX_ID_LEN] = {0};
    generate_consult_record_id(record_id, sizeof(record_id));
    
    // 生成真实接诊时间
    char consult_time[MAX_NAME_LEN] = {0};
    time_t consult_now = time(NULL);
    struct tm* consult_tm = localtime(&consult_now);
    strftime(consult_time, sizeof(consult_time), "%Y-%m-%d %H:%M:%S", consult_tm);
    
    // 创建并写入历史接诊记录
    ConsultRecordNode* record = create_consult_record_node(
        record_id,
        patient_id,
        doctor_id,
        appointment_id[0] != '\0' ? appointment_id : NULL,
        consult_time,
        diagnosis_text,
        treatment_advice,
        decision,
        pre_status,
        patient->status
    );
    
    if (record != NULL && g_consult_record_list != NULL)
    {
        insert_consult_record_tail(g_consult_record_list, record);
        printf("\n历史接诊记录已保存：%s\n", record_id);
    }

    return 1;
}

// 医生查看已处理患者
void doctor_view_processed_patients(const char* doctor_id)
{
    DoctorNode* doctor = NULL;
    PatientPtrNode* patient_list = NULL;
    PatientPtrNode* curr = NULL;
    int index = 1;
    
    // 检查患者链表是否初始化
    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化，无法查询！\n");
        return;
    }
    
    // 检查医生编号是否为空
    if (doctor_id == NULL || strlen(doctor_id) == 0)
    {
        printf("⚠️ 医生编号不能为空！\n");
        return;
    }
    
    // 获取医生信息
    doctor = get_doctor_by_id_checked(doctor_id);
    if (doctor == NULL)
        return;
    
    printf("\n================ 已处理患者列表 ================\n");
    printf("医生：%s（编号：%s）\n", doctor->name, doctor->id);
    printf("----------------------------------------------\n");
    
    // 获取已处理患者列表
    patient_list = get_processed_patients_by_doctor(doctor_id);
    
    if (patient_list == NULL)
    {
        printf("暂无已处理患者记录\n");
        return;
    }
    
    // 显示已接诊患者列表
    curr = patient_list;
    while (curr != NULL)
    {
        printf("\n[%d] 患者编号：%s\n", index++, curr->patient->id);
        printf("姓名：%s\n", curr->patient->name);
        printf("性别：%s\n", curr->patient->gender);
        printf("年龄：%d\n", curr->patient->age);
        printf("当前状态：%s\n", get_med_status_text(curr->patient->status));
    
        // 获取最新接诊记录
        ConsultRecordNode* latest_record = find_latest_consult_record_for_patient(curr->patient->id, doctor_id);
        
        // 本次处理类型 / 诊疗决策文字
        if (latest_record != NULL)
            printf("本次处理类型：%s\n", get_decision_text(latest_record->decision));
        else
            printf("本次处理类型：暂无\n");
        
        // 最近一次诊断结论
        if (latest_record != NULL && latest_record->diagnosis_text[0] != '\0')
            printf("最近诊断结论：%s\n", latest_record->diagnosis_text);
        else if (curr->patient->diagnosis_text[0] != '\0')
            printf("最近诊断结论：%s\n", curr->patient->diagnosis_text);
        else
            printf("最近诊断结论：暂无\n");
        
        // 最近一次处理意见
        if (latest_record != NULL && latest_record->treatment_advice[0] != '\0')
            printf("最近处理意见：%s\n", latest_record->treatment_advice);
        else if (curr->patient->treatment_advice[0] != '\0')
            printf("最近处理意见：%s\n", curr->patient->treatment_advice);
        else
            printf("最近处理意见：暂无\n");
        
        printf("----------------------------------------------\n");
        curr = curr->next;
    }
    
    // 释放链表
    curr = patient_list;
    while (curr != NULL)
    {
        PatientPtrNode* temp = curr;
        curr = curr->next;
        free(temp);
    }
}

// 医生查看已处理患者详情
void doctor_view_processed_patient_detail(const char* doctor_id, const char* patient_id)
{
    DoctorNode* doctor = NULL;
    PatientNode* patient = NULL;
    AppointmentNode* latest_appointment = NULL;
    char masked_id[MAX_ID_LEN] = {0};
    
    // 检查患者链表是否初始化
    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化，无法查询！\n");
        return;
    }
    
    // 检查医生编号是否为空
    if (doctor_id == NULL || strlen(doctor_id) == 0)
    {
        printf("⚠️ 医生编号不能为空！\n");
        return;
    }
    
    // 检查患者编号是否为空
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("⚠️ 患者编号不能为空！\n");
        return;
    }
    
    // 获取医生信息
    doctor = get_doctor_by_id_checked(doctor_id);
    if (doctor == NULL)
        return;
    
    // 根据患者编号查找患者
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到对应患者！\n");
        return;
    }
    
    // 检查患者是否归属于当前医生且已接诊
    if (strcmp(patient->doctor_id, doctor_id) != 0 || 
        patient->status == STATUS_PENDING || 
        patient->status == STATUS_RECHECK_PENDING)
    {
        printf("⚠️ 当前患者不属于该医生或未被接诊！\n");
        return;
    }
    
    // 获取最佳预约信息（优先匹配当前医生）
    latest_appointment = find_best_appointment_for_doctor(patient_id, doctor);
    
    // 显示患者详情
    printf("\n================ 已处理患者详情 ================\n");
    printf("患者编号：%s\n", patient->id);
    printf("姓名：%s\n", patient->name);
    printf("性别：%s\n", patient->gender);
    printf("年龄：%d\n", patient->age);
    
    // 脱敏身份证号
    if (strlen(patient->id_card) > 0)
    {
        mask_id_card(patient->id_card, masked_id);
        printf("身份证号：%s\n", masked_id);
    }
    else
    {
        printf("身份证号：暂无\n");
    }
    
    // 症状描述
    if (patient->symptom[0] != '\0')
        printf("症状描述：%s\n", patient->symptom);
    else
        printf("症状描述：暂无\n");
    
    // 目标科室
    if (patient->target_dept[0] != '\0')
        printf("目标科室：%s\n", patient->target_dept);
    else
        printf("目标科室：暂无\n");
    
    // 当前状态
    printf("当前状态：%s\n", get_med_status_text(patient->status));
    
    // 获取最新接诊记录
    ConsultRecordNode* latest_record = find_latest_consult_record_for_patient(patient_id, doctor_id);
    
    // 当前就诊类型
    if (latest_record != NULL)
    {
        if (latest_record->pre_status == STATUS_RECHECK_PENDING)
            printf("当前就诊类型：复诊\n");
        else if (latest_record->pre_status == STATUS_PENDING)
            printf("当前就诊类型：初诊\n");
    }
    else
    {
        printf("当前就诊类型：未知\n");
    }
    
    // 本次处理类型 / 诊疗决策文字
    if (latest_record != NULL)
        printf("本次处理类型：%s\n", get_decision_text(latest_record->decision));
    else
        printf("本次处理类型：暂无\n");
    
    // 最近一次诊断结论
    if (latest_record != NULL && latest_record->diagnosis_text[0] != '\0')
        printf("最近诊断结论：%s\n", latest_record->diagnosis_text);
    else if (patient->diagnosis_text[0] != '\0')
        printf("最近诊断结论：%s\n", patient->diagnosis_text);
    else
        printf("最近诊断结论：暂无\n");
    
    // 最近一次处理意见
    if (latest_record != NULL && latest_record->treatment_advice[0] != '\0')
        printf("最近处理意见：%s\n", latest_record->treatment_advice);
    else if (patient->treatment_advice[0] != '\0')
        printf("最近处理意见：%s\n", patient->treatment_advice);
    else
        printf("最近处理意见：暂无\n");
    
    // 最近一次预约信息
    printf("\n【最近一次预约信息】\n");
    if (latest_appointment != NULL)
    {
        printf("预约编号：%s\n", latest_appointment->appointment_id);
        printf("预约日期：%s\n", latest_appointment->appointment_date);
        printf("预约时段：%s\n", latest_appointment->appointment_slot);
        
        if (strlen(latest_appointment->appoint_doctor) > 0)
        {
            DoctorNode* appoint_doctor = find_doctor_by_id(g_doctor_list, latest_appointment->appoint_doctor);
            if (appoint_doctor != NULL)
                printf("预约医生：%s\n", appoint_doctor->name);
            else
                printf("预约医生：未知\n");
        }
        else if (strlen(latest_appointment->appoint_dept) > 0)
        {
            printf("预约科室：%s\n", latest_appointment->appoint_dept);
        }
        else
        {
            printf("预约科室/医生：暂无\n");
        }
        
        printf("预约状态：%s\n", get_appointment_display_status(latest_appointment));
    }
    else
    {
        printf("暂无预约记录\n");
    }
    
    // 显示历史接诊记录
    printf("\n【历史接诊记录】\n");
    ConsultRecordNode* curr_record = NULL;
    ConsultRecordNode* last_match = NULL;
    int record_count = 0;
    
    if (g_consult_record_list != NULL)
    {
        // 先找到最后一条匹配的记录（最新的）
        curr_record = g_consult_record_list->next;
        while (curr_record != NULL)
        {
            if (strcmp(curr_record->patient_id, patient_id) == 0)
            {
                last_match = curr_record;
            }
            curr_record = curr_record->next;
        }
        
        // 从最后一条匹配记录开始向前遍历（倒序显示）
        curr_record = last_match;
        while (curr_record != NULL && curr_record != g_consult_record_list)
        {
            if (strcmp(curr_record->patient_id, patient_id) == 0)
            {
                record_count++;
                printf("\n记录 %d：%s\n", record_count, curr_record->record_id);
                printf("接诊时间：%s\n", curr_record->consult_time);
                // 直接查找医生姓名
                DoctorNode* consult_doctor = find_doctor_by_id(g_doctor_list, curr_record->doctor_id);
                printf("接诊医生：%s\n", consult_doctor != NULL ? consult_doctor->name : "未知");
                printf("诊断结论：%s\n", curr_record->diagnosis_text[0] != '\0' ? curr_record->diagnosis_text : "暂无");
                printf("处理意见：%s\n", curr_record->treatment_advice[0] != '\0' ? curr_record->treatment_advice : "暂无");
                printf("诊疗决策：%s\n", get_decision_text(curr_record->decision));
                printf("----------------------------------------------\n");
            }
            curr_record = curr_record->prev;
        }
    }
    
    if (record_count == 0)
    {
        printf("暂无历史接诊记录\n");
    }
    
    printf("==============================================\n");
}

// 获取医生姓名（辅助函数）
static const char* get_doctor_name_by_id(const char* doctor_id)
{
    if (doctor_id == NULL || g_doctor_list == NULL)
        return "未知";
    
    DoctorNode* doctor = find_doctor_by_id(g_doctor_list, doctor_id);
    return (doctor != NULL) ? doctor->name : "未知";
}

// 获取患者姓名
const char* get_patient_name_by_id(const char* patient_id)
{
    if (patient_id == NULL || g_patient_list == NULL)
        return "未知";
    
    PatientNode* patient = find_patient_by_id(g_patient_list, patient_id);
    return (patient != NULL) ? patient->name : "未知";
}

// 查看历史接诊记录
void doctor_view_consult_history(const char* doctor_id, const char* patient_id)
{
    ConsultRecordNode* curr = NULL;
    ConsultRecordNode* last_match = NULL;
    int found = 0;
    
    // 检查接诊记录链表是否初始化
    if (g_consult_record_list == NULL)
    {
        printf("⚠️ 接诊记录链表尚未初始化，无法查询！\n");
        return;
    }
    
    printf("\n================ 历史接诊记录 ================\n");
    
    // 先从前往后遍历，找到最后一条匹配的记录
    curr = g_consult_record_list->next;
    while (curr != NULL)
    {
        int match = 1;
        
        // 如果提供了医生编号，检查是否匹配
        if (doctor_id != NULL && strlen(doctor_id) > 0)
        {
            if (strcmp(curr->doctor_id, doctor_id) != 0)
            {
                match = 0;
            }
        }
        
        // 如果提供了患者编号，检查是否匹配
        if (patient_id != NULL && strlen(patient_id) > 0)
        {
            if (strcmp(curr->patient_id, patient_id) != 0)
            {
                match = 0;
            }
        }
        
        if (match)
        {
            last_match = curr;
            found = 1;
        }
        
        curr = curr->next;
    }
    
    if (!found)
    {
        printf("暂无匹配的历史接诊记录\n");
        return;
    }
    
    // 从最后一条匹配记录开始，向前遍历显示（最新在前）
    curr = last_match;
    while (curr != NULL && curr != g_consult_record_list)
    {
        int match = 1;
        
        // 如果提供了医生编号，检查是否匹配
        if (doctor_id != NULL && strlen(doctor_id) > 0)
        {
            if (strcmp(curr->doctor_id, doctor_id) != 0)
            {
                match = 0;
            }
        }
        
        // 如果提供了患者编号，检查是否匹配
        if (patient_id != NULL && strlen(patient_id) > 0)
        {
            if (strcmp(curr->patient_id, patient_id) != 0)
            {
                match = 0;
            }
        }
        
        if (match)
        {
            printf("\n【接诊记录：%s】\n", curr->record_id);
            printf("患者编号：%s\n", curr->patient_id);
            printf("患者姓名：%s\n", get_patient_name_by_id(curr->patient_id));
            printf("医生编号：%s\n", curr->doctor_id);
            printf("医生姓名：%s\n", get_doctor_name_by_id(curr->doctor_id));
            
            // 显示诊断结论
            if (curr->diagnosis_text[0] != '\0')
                printf("诊断结论：%s\n", curr->diagnosis_text);
            else
                printf("诊断结论：暂无\n");
            
            // 显示处理意见
            if (curr->treatment_advice[0] != '\0')
                printf("处理意见：%s\n", curr->treatment_advice);
            else
                printf("处理意见：暂无\n");
            
            // 显示诊疗决策
            printf("诊疗决策：%s\n", get_decision_text(curr->decision));
            
            // 显示接诊前后状态
            printf("接诊前状态：%s\n", get_med_status_text(curr->pre_status));
            printf("接诊后状态：%s\n", get_med_status_text(curr->post_status));
            
            // 如果有预约编号，显示预约编号
            if (curr->appointment_id[0] != '\0')
                printf("预约编号：%s\n", curr->appointment_id);
            
            // 如果有接诊时间，显示接诊时间
            if (curr->consult_time[0] != '\0')
                printf("接诊时间：%s\n", curr->consult_time);
            
            printf("----------------------------------------------\n");
        }
        
        curr = curr->prev;
    }
}

// 医生查看患者接诊前概览
void doctor_view_patient_overview(const char* doctor_id, const char* patient_id)
{
    PatientNode* patient = NULL;
    DoctorNode* doctor = NULL;
    AppointmentNode* latest_appointment = NULL;
    char masked_id[MAX_ID_LEN] = {0};
    
    // 检查患者链表是否初始化
    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化，无法查询！\n");
        return;
    }
    
    // 检查医生编号是否为空
    if (doctor_id == NULL || strlen(doctor_id) == 0)
    {
        printf("⚠️ 医生编号不能为空！\n");
        return;
    }
    
    // 检查患者编号是否为空
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("⚠️ 患者编号不能为空！\n");
        return;
    }
    
    // 获取医生信息
    doctor = get_doctor_by_id_checked(doctor_id);
    if (doctor == NULL)
        return;
    
    // 根据患者编号查找患者
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到对应患者！\n");
        return;
    }
    
    // 检查患者是否归属于当前医生
    if (!is_patient_match_doctor(patient, doctor))
    {
        printf("⚠️ 当前患者未分配给该医生，或预约目标与该医生不匹配！\n");
        return;
    }
    
    // 获取最佳预约信息（优先匹配当前医生）
    latest_appointment = find_best_appointment_for_doctor(patient_id, doctor);
    
    // 显示患者接诊前概览
    printf("\n================ 患者接诊前概览 ================\n");
    printf("患者编号：%s\n", patient->id);
    printf("姓名：%s\n", patient->name);
    printf("性别：%s\n", patient->gender);
    printf("年龄：%d\n", patient->age);
    
    // 症状描述
    if (patient->symptom[0] != '\0')
        printf("症状描述：%s\n", patient->symptom);
    else
        printf("症状描述：暂无\n");
    
    // 目标科室
    if (patient->target_dept[0] != '\0')
        printf("目标科室：%s\n", patient->target_dept);
    else
        printf("目标科室：暂无\n");
    
    // 当前状态
    printf("当前状态：%s\n", get_med_status_text(patient->status));
    
    // 当前就诊类型
    if (patient->status == STATUS_RECHECK_PENDING)
        printf("当前就诊类型：复诊\n");
    else if (patient->status == STATUS_PENDING)
        printf("当前就诊类型：初诊\n");
    
    // 获取最新接诊记录
    ConsultRecordNode* latest_record = find_latest_consult_record_for_patient(patient_id, doctor_id);
    
    // 本次处理类型 / 诊疗决策文字
    if (latest_record != NULL)
        printf("本次处理类型：%s\n", get_decision_text(latest_record->decision));
    else
        printf("本次处理类型：暂无\n");
    
    // 最近一次诊断结论
    if (patient->diagnosis_text[0] != '\0')
        printf("最近诊断结论：%s\n", patient->diagnosis_text);
    else
        printf("最近诊断结论：暂无诊断记录\n");
    
    // 最近一次处理意见
    if (patient->treatment_advice[0] != '\0')
        printf("最近处理意见：%s\n", patient->treatment_advice);
    else
        printf("最近处理意见：暂无处理意见\n");
    
    // 脱敏身份证号（医生侧不需要显示完整身份证号）
    if (strlen(patient->id_card) > 0)
    {
        mask_id_card(patient->id_card, masked_id);
        printf("身份证号：%s\n", masked_id);
    }
    else
    {
        printf("身份证号：暂无\n");
    }
    
    // 最近一次预约信息
    printf("\n【最近一次预约信息】\n");
    if (latest_appointment != NULL)
    {
        printf("预约编号：%s\n", latest_appointment->appointment_id);
        printf("预约日期：%s\n", latest_appointment->appointment_date);
        printf("预约时段：%s\n", latest_appointment->appointment_slot);
        
        if (strlen(latest_appointment->appoint_doctor) > 0)
        {
            DoctorNode* doctor = find_doctor_by_id(g_doctor_list, latest_appointment->appoint_doctor);
            if (doctor != NULL)
                printf("预约医生：%s\n", doctor->name);
            else
                printf("预约医生：未知\n");
        }
        else if (strlen(latest_appointment->appoint_dept) > 0)
        {
            printf("预约科室：%s\n", latest_appointment->appoint_dept);
        }
        else
        {
            printf("预约科室/医生：暂无\n");
        }
        
        printf("预约状态：%s\n", get_appointment_display_status(latest_appointment));
    }
    else
    {
        printf("暂无预约记录\n");
    }
    
    // 显示检查记录
    printf("\n【检查记录】\n");
    CheckRecordNode* curr_check = g_check_record_list->next;
    int check_count = 0;
    
    while (curr_check != NULL && curr_check != g_check_record_list)
    {
        if (strcmp(curr_check->patient_id, patient_id) == 0)
        {
            check_count++;
            printf("\n检查记录 %d：%s\n", check_count, curr_check->record_id);
            printf("检查项目：%s\n", curr_check->item_name);
            printf("所属科室：%s\n", curr_check->dept);
            printf("检查状态：%s\n", curr_check->is_completed == 1 ? "已完成" : "待检查");
            printf("缴费状态：%s\n", curr_check->is_paid == 1 ? "已缴费" : "未缴费");
            if (curr_check->is_completed == 1)
            {
                printf("检查时间：%s\n", curr_check->check_time);
                printf("检查结果：%s\n", curr_check->result);
            }
            printf("----------------------------------------------\n");
        }
        curr_check = curr_check->next;
    }
    
    if (check_count == 0)
    {
        printf("暂无检查记录\n");
    }
}

// ==========================================
// 检查科室医生功能
// ==========================================

// 显示检查科室医生的待检查列表（与医生待诊列表逻辑相同）
void show_waiting_checks_by_dept(const char* doctor_id)
{
    int found = 0;
    CheckRecordNode* curr = NULL;
    DoctorNode* doctor = get_doctor_by_id_checked(doctor_id);
    int index = 1;

    if (doctor == NULL) return;

    if (g_check_record_list == NULL)
    {
        printf("⚠️ 检查记录链表尚未初始化！\n");
        return;
    }

    printf("\n================ 待检查列表 ================\n");
    printf("医生编号: %s  医生姓名: %s  科室: %s\n",
        doctor->id, doctor->name, doctor->department);
    printf("--------------------------------------------\n");

    curr = g_check_record_list->next;
    while (curr != NULL && curr != g_check_record_list)
    {
        // 筛选出未完成且科室匹配的检查记录
        if (curr->is_completed == 0 && strcmp(curr->dept, doctor->department) == 0)
        {
            const char* patient_name = get_patient_name_by_id(curr->patient_id);
            printf("[%d] 检查记录: %s | 患者编号: %s | 患者姓名: %s | 检查项目: %s\n",
                index++,
                curr->record_id,
                curr->patient_id,
                patient_name != NULL ? patient_name : "未知",
                curr->item_name);
            found = 1;
        }
        curr = curr->next;
    }

    if (!found)
    {
        printf("ℹ️ 当前没有待检查的记录。\n");
    }
}

// 获取待检查列表（用于选择）
int get_waiting_checks_by_dept(const char* doctor_id, CheckRecordNode** check_list)
{
    int count = 0;
    CheckRecordNode* curr = NULL;
    DoctorNode* doctor = get_doctor_by_id_checked(doctor_id);

    if (doctor == NULL || check_list == NULL) return 0;

    if (g_check_record_list == NULL)
    {
        return 0;
    }

    curr = g_check_record_list->next;
    while (curr != NULL && curr != g_check_record_list)
    {
        if (curr->is_completed == 0 && strcmp(curr->dept, doctor->department) == 0)
        {
            check_list[count++] = curr;
        }
        curr = curr->next;
    }

    return count;
}

// 显示检查记录详情
void show_check_record_detail(CheckRecordNode* record)
{
    if (record == NULL) return;

    printf("\n================ 检查记录详情 ================\n");
    printf("检查记录编号：%s\n", record->record_id);
    printf("患者编号：%s\n", record->patient_id);
    printf("患者姓名：%s\n", get_patient_name_by_id(record->patient_id));
    printf("检查项目：%s\n", record->item_name);
    printf("检查项目编号：%s\n", record->item_id);
    printf("所属科室：%s\n", record->dept);
    printf("检查状态：%s\n", record->is_completed == 1 ? "已完成" : "待检查");
    printf("缴费状态：%s\n", record->is_paid == 1 ? "已缴费" : "未缴费");
    if (record->is_completed == 1)
    {
        printf("检查时间：%s\n", record->check_time);
        printf("检查结果：%s\n", record->result);
    }
    printf("--------------------------------------------\n");
}

// 录入检查结果
int doctor_update_check_result(const char* doctor_id, const char* record_id, const char* result)
{
    DoctorNode* doctor = get_doctor_by_id_checked(doctor_id);
    CheckRecordNode* curr = NULL;

    if (doctor == NULL) return 0;
    if (record_id == NULL || strlen(record_id) == 0)
    {
        printf("⚠️ 检查记录编号不能为空！\n");
        return 0;
    }
    if (result == NULL || strlen(result) == 0)
    {
        printf("⚠️ 检查结果不能为空！\n");
        return 0;
    }

    if (g_check_record_list == NULL)
    {
        printf("⚠️ 检查记录链表尚未初始化！\n");
        return 0;
    }

    curr = g_check_record_list->next;
    while (curr != NULL && curr != g_check_record_list)
    {
        if (strcmp(curr->record_id, record_id) == 0)
        {
            // 检查科室是否匹配
            if (strcmp(curr->dept, doctor->department) != 0)
            {
                printf("⚠️ 该检查记录不属于您的科室！\n");
                return 0;
            }
            
            // 检查是否已完成
            if (curr->is_completed == 1)
            {
                printf("⚠️ 该检查记录已完成，无法修改！\n");
                return 0;
            }
            
            // 检查是否已缴费
            if (curr->is_paid == 0)
            {
                printf("⚠️ 该检查记录尚未缴费，不能录入结果！请先完成缴费。\n");
                return 0;
            }

            // 更新检查结果
            strncpy(curr->result, result, MAX_RECORD_LEN - 1);
            curr->is_completed = 1;
            
            // 设置检查完成时间
            time_t now = time(NULL);
            struct tm* t = localtime(&now);
            strftime(curr->check_time, MAX_NAME_LEN, "%Y-%m-%d %H:%M:%S", t);
            
            // 自动识别患者并拨转状态
            PatientNode* patient = find_patient_by_id(g_patient_list, curr->patient_id);
            if (patient != NULL)
            {
                patient->status = STATUS_RECHECK_PENDING;
                patient->queue_time = time(NULL) - 7200; // 拿CT/验血报告回来的复诊患者，虚拟时间前移2小时，优先看诊
                
                // 为检查后复诊的患者生成现场挂号记录
                if (g_appointment_list != NULL && strlen(patient->target_dept) > 0) {
                    char appointment_id[MAX_ID_LEN];
                    // 生成预约编号
                    int max_no = 0;
                    AppointmentNode* appt_curr = NULL;
                    if (g_appointment_list != NULL)
                    {
                        appt_curr = g_appointment_list->next;
                        while (appt_curr != NULL)
                        {
                            if (strncmp(appt_curr->appointment_id, "A-", 2) == 0)
                            {
                                int current_no = atoi(appt_curr->appointment_id + 2);
                                if (current_no > max_no)
                                {
                                    max_no = current_no;
                                }
                            }
                            appt_curr = appt_curr->next;
                        }
                    }
                    snprintf(appointment_id, MAX_ID_LEN, "A-%03d", max_no + 1);
                    
                    // 获取当前日期
                    char current_date[MAX_NAME_LEN];
                    time_t now = time(NULL);
                    struct tm* tm_now = localtime(&now);
                    strftime(current_date, sizeof(current_date), "%Y-%m-%d", tm_now);
                    
                    // 随机分配上午或下午
                    const char* slot = (rand() % 2 == 0) ? "上午" : "下午";
                    
                    // 创建预约记录
                    AppointmentNode* new_appointment = create_appointment_node(
                        appointment_id,
                        patient->id,
                        current_date,
                        slot,
                        patient->doctor_id,
                        patient->target_dept,
                        CHECKED_IN
                    );
                    
                    if (new_appointment != NULL) {
                        new_appointment->is_walk_in = 1; // 检查后复诊视为现场挂号
                        new_appointment->reg_fee = 10.0; // 现场号10元
                        new_appointment->fee_paid = 1; // 假设已缴费
                        insert_appointment_tail(g_appointment_list, new_appointment);
                    }
                }
            }

            printf("✅ 检查结果录入成功！\n");
            printf("检查记录: %s\n", curr->record_id);
            printf("患者姓名: %s\n", get_patient_name_by_id(curr->patient_id));
            printf("检查项目: %s\n", curr->item_name);
            printf("检查结果: %s\n", curr->result);
            printf("完成时间: %s\n", curr->check_time);
            printf("🔄 [系统联动] 该患者状态已自动更新为：检查后待复诊 (STATUS_RECHECK_PENDING)，已重新回到原接诊医生队列。\n");
            return 1;
        }
        curr = curr->next;
    }

    printf("⚠️ 未找到指定的检查记录！\n");
    return 0;
}

void show_check_records_by_patient_id(const char* patient_id)
{
    if (is_blank_string(patient_id))
    {
        printf("提示：患者编号不能为空。\n");
        return;
    }

    if (g_check_record_list == NULL || g_check_record_list->next == NULL)
    {
        printf("当前暂无可用的检查记录数据。\n");
        return;
    }

    CheckRecordPtrNode* records = get_check_records_by_patient(g_check_record_list, patient_id);
    if (records == NULL)
    {
        printf("\n⚠️ 患者 %s 暂无检查记录。\n", patient_id);
        return;
    }

    const char* p_name = get_patient_name_by_id(patient_id);

    printf("\n==============================================================\n");
    printf("         患者检查记录查询\n");
    printf("==============================================================\n");
    printf("患者编号：%s\n", patient_id);
    printf("患者姓名：%s\n", p_name ? p_name : "未知");
    printf("--------------------------------------------------------------\n");
    print_col("记录编号", 12);
    print_col("检查项目", 20);
    print_col("科室", 12);
    print_col("完成时间", 22);
    print_col("完成", 8);
    print_col("缴费", 6);
    printf("\n");
    printf("--------------------------------------------------------------\n");

    CheckRecordPtrNode* curr = records;
    int count = 0;
    while (curr != NULL)
    {
        CheckRecordNode* r = curr->record;
        print_col(r->record_id, 12);
        print_col(r->item_name, 20);
        print_col(r->dept, 12);
        print_col(r->is_completed ? r->check_time : "——", 22);
        print_col(r->is_completed ? "✅" : "待查", 8);
        print_col(r->is_paid ? "✅" : "未缴", 6);
        printf("\n");
        count++;
        curr = curr->next;
    }

    printf("--------------------------------------------------------------\n");
    printf("共 %d 条检查记录\n", count);

    curr = records;
    count = 0;
    while (curr != NULL)
    {
        count++;
        CheckRecordNode* r = curr->record;
        printf("\n[记录 %d] %s\n", count, r->record_id);
        printf("  检查项目：%s（%s）\n", r->item_name, r->item_id);
        printf("  所属科室：%s\n", r->dept);
        printf("  完成状态：%s\n", r->is_completed ? "已完成" : "待检查");
        printf("  缴费状态：%s\n", r->is_paid ? "已缴费" : "未缴费");
        if (r->is_completed)
        {
            printf("  检查时间：%s\n", r->check_time);
            printf("  检查结果：%s\n", r->result);
        }
        curr = curr->next;
    }

    printf("\n==============================================================\n");

    free_check_record_ptr_list(records);
}

void show_check_record_by_id(const char* record_id)
{
    if (is_blank_string(record_id))
    {
        printf("提示：检查记录编号不能为空。\n");
        return;
    }

    if (g_check_record_list == NULL)
    {
        printf("提示：检查记录链表尚未初始化。\n");
        return;
    }

    CheckRecordNode* record = find_check_record_by_id(g_check_record_list, record_id);
    if (record == NULL)
    {
        printf("\n⚠️ 未找到检查记录 %s。\n", record_id);
        return;
    }

    show_check_record_detail(record);
}



