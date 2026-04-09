// ==========================================
// 文件名: doctor_service.c
// 作用: 医生接诊相关业务层实现
// ==========================================
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "list_ops.h"
#include "doctor_service.h"

// 就诊状态转文字
static const char* get_med_status_text(MedStatus status)
{
    switch (status)
    {
        case STATUS_PENDING: return "待诊";
        case STATUS_EXAMINING: return "检查中";
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
void show_waiting_patients_by_doctor(const char* doctor_id)
{
    int found = 0;
    PatientNode* curr = NULL;
    DoctorNode* doctor = get_doctor_by_id_checked(doctor_id);

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
        if (curr->status == STATUS_PENDING && is_patient_match_doctor(curr, doctor))
        {
            const char* brief_text = strlen(curr->symptom) > 0 ? curr->symptom : curr->target_dept;
            printf("患者编号: %s | 姓名: %s | 年龄: %d | 症状/科室: %s | 状态: %s\n",
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

    if (patient->status != STATUS_PENDING)
    {
        printf("⚠️ 当前患者状态为[%s]，不属于待诊状态！\n", get_med_status_text(patient->status));
        return 0;
    }

    if (!is_patient_match_doctor(patient, doctor))
    {
        printf("⚠️ 当前患者未分配给该医生，或预约目标与该医生不匹配！\n");
        return 0;
    }

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
            printf("✅ 接诊完成，患者已结束就诊。\n");
            break;
        case 2:
            patient->status = STATUS_UNPAID;
            printf("✅ 已完成开药处理，患者状态变为“已看诊待缴费”。\n");
            break;
        case 3:
            patient->status = STATUS_EXAMINING;
            printf("✅ 已完成开检查处理，患者状态变为“检查中”。\n");
            break;
        case 4:
            patient->status = STATUS_HOSPITALIZED;
            printf("✅ 已完成住院办理占位处理，患者状态变为“住院中”。\n");
            break;
        default:
            printf("⚠️ 无效的诊疗决策，操作取消！\n");
            return 0;
    }

    // 方案A：先将患者分配给当前医生，确保队列人数正确减少
    strncpy(patient->doctor_id, doctor->id, MAX_ID_LEN - 1);
    patient->doctor_id[MAX_ID_LEN - 1] = '\0';
    
    if (doctor->queue_length > 0)
    {
        doctor->queue_length--;
    }

    return 1;
}