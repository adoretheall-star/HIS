// ==========================================
// 文件名: doctor_service.c
// 作用: 医生接诊相关业务层实现
// ==========================================
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "global.h"
#include "list_ops.h"
#include "doctor_service.h"
#include "utils.h"





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
        case STATUS_HOSPITALIZED: return "住院中";
        case STATUS_COMPLETED: return "就诊结束";
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
int get_waiting_patients_by_doctor(const char* doctor_id, PatientNode** patient_list)
{
    int count = 0;
    PatientNode* curr = NULL;
    DoctorNode* doctor = get_doctor_by_id_checked(doctor_id);

    if (doctor == NULL || patient_list == NULL) return 0;

    if (g_patient_list == NULL)
    {
        return 0;
    }

    curr = g_patient_list->next;
    while (curr != NULL)
    {
        if ((curr->status == STATUS_PENDING || curr->status == STATUS_RECHECK_PENDING) && is_patient_match_doctor(curr, doctor))
        {
            patient_list[count++] = curr;
        }
        curr = curr->next;
    }

    return count;
}

// 获取指定医生的已处理患者列表（返回患者数量）
int get_processed_patients_by_doctor(const char* doctor_id, PatientNode** patient_list)
{
    int count = 0;
    PatientNode* curr = NULL;

    if (doctor_id == NULL || patient_list == NULL) return 0;

    if (g_patient_list == NULL)
    {
        return 0;
    }

    curr = g_patient_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->doctor_id, doctor_id) == 0)
        {
            // 只把以下状态视为"已接诊患者/已处理患者"：
            // STATUS_EXAMINING, STATUS_UNPAID, STATUS_WAIT_MED, STATUS_HOSPITALIZED, STATUS_COMPLETED
            if (curr->status == STATUS_EXAMINING ||
                curr->status == STATUS_UNPAID ||
                curr->status == STATUS_WAIT_MED ||
                curr->status == STATUS_HOSPITALIZED ||
                curr->status == STATUS_COMPLETED)
            {
                patient_list[count++] = curr;
            }
        }
        curr = curr->next;
    }

    return count;
}

void show_waiting_patients_by_doctor(const char* doctor_id)
{
    int found = 0;
    PatientNode* curr = NULL;
    DoctorNode* doctor = get_doctor_by_id_checked(doctor_id);
    int index = 1;

    if (doctor == NULL) return;

    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化！\n");
        return;
    }

    printf("\n================ 医生待诊列表 ================\n");
    printf("医生编号: %s  医生姓名: %s  科室: %s\n",
        doctor->id, doctor->name, doctor->department);

    curr = g_patient_list->next;
    while (curr != NULL)
    {
        if ((curr->status == STATUS_PENDING || curr->status == STATUS_RECHECK_PENDING) && is_patient_match_doctor(curr, doctor))
        {
            const char* brief_text = strlen(curr->symptom) > 0 ? curr->symptom : curr->target_dept;
            const char* visit_type = curr->status == STATUS_PENDING ? "[初诊]" : "[复诊]";
            printf("[%d] %s 患者编号: %s | 姓名: %s | 年龄: %d | 症状/科室: %s | 状态: %s\n",
                index++,
                visit_type,
                curr->id,
                curr->name,
                curr->age,
                strlen(brief_text) > 0 ? brief_text : "暂无",
                get_med_status_text(curr->status));
            found = 1;
        }
        curr = curr->next;
    }

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
            patient->status = STATUS_COMPLETED;
            break;
        case 2:
            patient->status = STATUS_UNPAID;
            patient->unpaid_time = time(NULL); // ⏱️ 补丁：按下72小时作废的秒表！
            break;
        case 3:
            patient->status = STATUS_EXAMINING;
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
    
    if (doctor->queue_length > 0)
    {
        doctor->queue_length--;
    }

    // 更新患者的预约记录状态（联动更新）
    AppointmentNode* curr_appointment = NULL;
    AppointmentNode* latest_appointment = NULL;
    char appointment_id[MAX_ID_LEN] = {0};
    
    if (g_appointment_list != NULL)
    {
        curr_appointment = g_appointment_list->next;
        while (curr_appointment != NULL)
        {
            if (strcmp(curr_appointment->patient_id, patient_id) == 0)
            {
                latest_appointment = curr_appointment;
                strncpy(appointment_id, curr_appointment->appointment_id, MAX_ID_LEN - 1);
                appointment_id[MAX_ID_LEN - 1] = '\0';
            }
            curr_appointment = curr_appointment->next;
        }
        
        // 如果找到患者的预约记录，更新其状态
        if (latest_appointment != NULL)
        {
            // 根据诊疗决策更新预约状态
            switch (decision)
            {
                case 1: // 结束就诊
                    latest_appointment->appointment_status = CHECKED_IN; // 保持已签到状态
                    break;
                case 2: // 开药
                    latest_appointment->appointment_status = CHECKED_IN;
                    break;
                case 3: // 开检查
                    latest_appointment->appointment_status = CHECKED_IN;
                    break;
                case 4: // 办理住院
                    latest_appointment->appointment_status = CHECKED_IN;
                    break;
            }
            printf("\n预约记录已更新：%s\n", get_appointment_status_text(latest_appointment->appointment_status));
        }
    }

    // 生成接诊记录编号（课设简化版：使用当前时间戳的简单模拟）
    static int record_counter = 1;
    char record_id[MAX_ID_LEN] = {0};
    sprintf(record_id, "CR-%04d", record_counter++);
    
    // 生成接诊时间（课设简化版：使用字符串占位）
    char consult_time[MAX_NAME_LEN] = "2026-04-06 10:00:00";
    
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
    PatientNode* patient_list[100];
    int count = 0;
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
    count = get_processed_patients_by_doctor(doctor_id, patient_list);
    
    if (count == 0)
    {
        printf("暂无已处理患者记录\n");
        return;
    }
    
    // 显示已接诊患者列表
    for (int i = 0; i < count; i++)
    {
        printf("\n[%d] 患者编号：%s\n", index++, patient_list[i]->id);
        printf("姓名：%s\n", patient_list[i]->name);
        printf("年龄：%d\n", patient_list[i]->age);
        printf("当前状态：%s\n", get_med_status_text(patient_list[i]->status));
    
        // 获取最新接诊记录
        ConsultRecordNode* latest_record = find_latest_consult_record_for_patient(patient_list[i]->id, doctor_id);
        
        // 本次处理类型 / 诊疗决策文字
        if (latest_record != NULL)
            printf("本次处理类型：%s\n", get_decision_text(latest_record->decision));
        else
            printf("本次处理类型：暂无\n");
        
        // 最近一次诊断结论
        if (patient_list[i]->diagnosis_text[0] != '\0')
            printf("最近诊断结论：%s\n", patient_list[i]->diagnosis_text);
        else
            printf("最近诊断结论：暂无\n");
        
        // 最近一次处理意见
        if (patient_list[i]->treatment_advice[0] != '\0')
            printf("最近处理意见：%s\n", patient_list[i]->treatment_advice);
        else
            printf("最近处理意见：暂无\n");
        
        printf("----------------------------------------------\n");
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
        printf("最近诊断结论：暂无\n");
    
    // 最近一次处理意见
    if (patient->treatment_advice[0] != '\0')
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
        
        printf("预约状态：%s\n", get_appointment_status_text(latest_appointment->appointment_status));
    }
    else
    {
        printf("暂无预约记录\n");
    }
    printf("==============================================\n");
}

// 检查完成回诊登记
int complete_exam_and_return_to_doctor(const char* patient_id)
{
    PatientNode* patient = NULL;
    DoctorNode* doctor = NULL;
    
    // 参数校验
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("⚠️ 患者编号不能为空！\n");
        return 0;
    }
    
    // 查找患者
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到对应患者！\n");
        return 0;
    }
    
    // 检查患者当前状态是否为检查中
    if (patient->status != STATUS_EXAMINING)
    {
        printf("⚠️ 当前患者状态为[%s]，不是检查中状态，无法办理回诊登记！\n", get_med_status_text(patient->status));
        return 0;
    }
    
    // 检查患者是否有对应的医生
    if (patient->doctor_id[0] == '\0')
    {
        printf("⚠️ 当前患者未分配医生，无法办理回诊登记！\n");
        return 0;
    }
    
    // 查找对应的医生
    doctor = find_doctor_by_id(g_doctor_list, patient->doctor_id);
    
    // 更新患者状态为检查后待复诊
    patient->status = STATUS_RECHECK_PENDING;
    
    // 输出提示信息
    printf("\n✅ 检查完成回诊登记成功！\n");
    printf("患者：%s（编号：%s）已完成检查\n", patient->name, patient->id);
    printf("已回到原医生待复诊队列\n");
    printf("对应医生：%s（编号：%s）\n", doctor ? doctor->name : "未知", patient->doctor_id);
    
    return 1;
}

// 获取患者姓名
const char* get_patient_name_by_id(const char* patient_id)
{
    if (patient_id == NULL || g_patient_list == NULL)
        return "未知";
    
    PatientNode* patient = find_patient_by_id(g_patient_list, patient_id);
    return (patient != NULL) ? patient->name : "未知";
}

// 获取医生姓名（辅助函数）
static const char* get_doctor_name_by_id(const char* doctor_id)
{
    if (doctor_id == NULL || g_doctor_list == NULL)
        return "未知";
    
    DoctorNode* doctor = find_doctor_by_id(g_doctor_list, doctor_id);
    return (doctor != NULL) ? doctor->name : "未知";
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
        
        printf("预约状态：%s\n", get_appointment_status_text(latest_appointment->appointment_status));
    }
    else
    {
        printf("暂无预约记录\n");
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

            // 更新检查结果
            strncpy(curr->result, result, MAX_RECORD_LEN - 1);
            curr->is_completed = 1;
            
            // 设置检查完成时间
            time_t now = time(NULL);
            struct tm* t = localtime(&now);
            strftime(curr->check_time, MAX_NAME_LEN, "%Y-%m-%d %H:%M:%S", t);

            printf("✅ 检查结果录入成功！\n");
            printf("检查记录: %s\n", curr->record_id);
            printf("患者姓名: %s\n", get_patient_name_by_id(curr->patient_id));
            printf("检查项目: %s\n", curr->item_name);
            printf("检查结果: %s\n", curr->result);
            printf("完成时间: %s\n", curr->check_time);
            return 1;
        }
        curr = curr->next;
    }

    printf("⚠️ 未找到指定的检查记录！\n");
    return 0;
}