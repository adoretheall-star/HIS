// ==========================================
// 文件名: appointment.c
// 作用: 预约管理相关业务层实现
// ==========================================
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "list_ops.h"
#include "appointment.h"
#include "patient_service.h"
// 预约状态转文字
static const char* get_appointment_status_text(AppointmentStatus status)
{
    switch (status)
    {
        case RESERVED: return "已预约";
        case CHECKED_IN: return "已签到";
        case CANCELLED: return "已取消";
        case MISSED: return "已过号";
        default: return "未知状态";
    }
}
// 生成下一个预约编号
static void generate_appointment_id(char* new_id)
{
    int max_no = 0;
    AppointmentNode* curr = NULL;
    if (new_id == NULL) return;
    if (g_appointment_list != NULL)
    {
        curr = g_appointment_list->next;
        while (curr != NULL)
        {
            if (strncmp(curr->appointment_id, "A-", 2) == 0)
            {
                int current_no = atoi(curr->appointment_id + 2);
                if (current_no > max_no)
                {
                    max_no = current_no;
                }
            }
            curr = curr->next;
        }
    }
    snprintf(new_id, MAX_ID_LEN, "A-%03d", max_no + 1);
}
// 打印单条预约信息
static void print_appointment_info(const AppointmentNode* appointment)
{
    const char* target_text = NULL;
    if (appointment == NULL) return;
    if (strlen(appointment->appoint_doctor) > 0)
    {
        target_text = appointment->appoint_doctor;
        printf("预约编号：%s | 日期：%s | 时段：%s | 医生：%s | 状态：%s\n",
            appointment->appointment_id,
            appointment->appointment_date,
            appointment->appointment_slot,
            target_text,
            get_appointment_status_text(appointment->appointment_status));
    }
    else
    {
        target_text = appointment->appoint_dept;
        printf("预约编号：%s | 日期：%s | 时段：%s | 科室：%s | 状态：%s\n",
            appointment->appointment_id,
            appointment->appointment_date,
            appointment->appointment_slot,
            target_text,
            get_appointment_status_text(appointment->appointment_status));
    }
}
// 预约登记
AppointmentNode* register_appointment(
    const char* patient_id,
    const char* appointment_date,
    const char* appointment_slot,
    const char* appoint_doctor,
    const char* appoint_dept
) {
    char new_id[MAX_ID_LEN];
    AppointmentNode* new_appointment = NULL;
    PatientNode* patient = NULL;
    int has_doctor = 0;
    int has_dept = 0;
    if (g_patient_list == NULL || g_appointment_list == NULL)
    {
        printf("⚠️ 基础链表尚未初始化，无法登记预约！\n");
        return NULL;
    }
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("⚠️ 患者编号不能为空！\n");
        return NULL;
    }
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到对应患者，预约登记失败！\n");
        return NULL;
    }
    if (appointment_date == NULL || strlen(appointment_date) == 0)
    {
        printf("⚠️ 预约日期不能为空！\n");
        return NULL;
    }
    if (appointment_slot == NULL || strlen(appointment_slot) == 0)
    {
        printf("⚠️ 预约时段不能为空！\n");
        return NULL;
    }
    has_doctor = (appoint_doctor != NULL && strlen(appoint_doctor) > 0);
    has_dept = (appoint_dept != NULL && strlen(appoint_dept) > 0);
    if (!has_doctor && !has_dept)
    {
        printf("⚠️ 预约医生和预约科室至少要填写一个！\n");
        return NULL;
    }
    generate_appointment_id(new_id);
    new_appointment = create_appointment_node(
        new_id,
        patient_id,
        appointment_date,
        appointment_slot,
        has_doctor ? appoint_doctor : "",
        has_dept ? appoint_dept : "",
        RESERVED
    );
    if (new_appointment == NULL)
    {
        printf("⚠️ 预约节点创建失败！\n");
        return NULL;
    }
    insert_appointment_tail(g_appointment_list, new_appointment);
    printf("✅ 预约登记成功！预约编号：%s，患者编号：%s\n", new_appointment->appointment_id, patient->id);
    return new_appointment;
}
// 按患者编号查询预约
void query_appointments_by_patient_id(const char* patient_id)
{
    int found = 0;
    AppointmentNode* curr = NULL;
    if (g_appointment_list == NULL)
    {
        printf("⚠️ 预约链表尚未初始化！\n");
        return;
    }
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("⚠️ 患者编号不能为空！\n");
        return;
    }
    curr = g_appointment_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->patient_id, patient_id) == 0)
        {
            print_appointment_info(curr);
            found = 1;
        }
        curr = curr->next;
    }
    if (!found)
    {
        printf("ℹ️ 未查询到该患者的预约记录。\n");
    }
}
// 按身份证号查询预约
void query_appointments_by_id_card(const char* id_card)
{
    PatientNode* patient = NULL;
    if (id_card == NULL || strlen(id_card) == 0)
    {
        printf("⚠️ 身份证号不能为空！\n");
        return;
    }
    patient = find_patient_by_id_card(id_card);
    if (patient == NULL)
    {
        printf("ℹ️ 未找到对应患者，无法查询预约。\n");
        return;
    }
    query_appointments_by_patient_id(patient->id);
}
// 预约取消
int cancel_appointment(const char* appointment_id)
{
    AppointmentNode* appointment = NULL;
    if (g_appointment_list == NULL)
    {
        printf("⚠️ 预约链表尚未初始化！\n");
        return 0;
    }
    appointment = find_appointment_by_id(g_appointment_list, appointment_id);
    if (appointment == NULL)
    {
        printf("⚠️ 未找到对应预约记录！\n");
        return 0;
    }
    if (appointment->appointment_status != RESERVED)
    {
        printf("⚠️ 当前预约状态为[%s]，不允许取消！\n",
            get_appointment_status_text(appointment->appointment_status));
        return 0;
    }
    appointment->appointment_status = CANCELLED;
    printf("✅ 预约取消成功！预约编号：%s\n", appointment->appointment_id);
    return 1;
}
// 预约签到转挂号
int check_in_appointment(const char* appointment_id)
{
    AppointmentNode* appointment = NULL;
    PatientNode* patient = NULL;
    DoctorNode* doctor = NULL;
    if (g_appointment_list == NULL || g_patient_list == NULL)
    {
        printf("⚠️ 基础链表尚未初始化，无法签到！\n");
        return 0;
    }
    appointment = find_appointment_by_id(g_appointment_list, appointment_id);
    if (appointment == NULL)
    {
        printf("⚠️ 未找到对应预约记录！\n");
        return 0;
    }
    if (appointment->appointment_status != RESERVED)
    {
        printf("⚠️ 当前预约状态为[%s]，不能执行签到！\n",
            get_appointment_status_text(appointment->appointment_status));
        return 0;
    }
    patient = find_patient_by_id(g_patient_list, appointment->patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 对应患者不存在，签到失败！\n");
        return 0;
    }
    appointment->appointment_status = CHECKED_IN;
    patient->status = STATUS_PENDING;
    if (strlen(appointment->appoint_dept) > 0)
    {
        strncpy(patient->target_dept, appointment->appoint_dept, MAX_NAME_LEN - 1);
        patient->target_dept[MAX_NAME_LEN - 1] = '\0';
    }
    if (strlen(appointment->appoint_doctor) > 0 && g_doctor_list != NULL)
    {
        doctor = find_doctor_by_id(g_doctor_list, appointment->appoint_doctor);
        if (doctor != NULL)
        {
            doctor->queue_length++;
            strncpy(patient->doctor_id, doctor->id, MAX_ID_LEN - 1);
            patient->doctor_id[MAX_ID_LEN - 1] = '\0';
            if (strlen(patient->target_dept) == 0)
            {
                strncpy(patient->target_dept, doctor->department, MAX_NAME_LEN - 1);
                patient->target_dept[MAX_NAME_LEN - 1] = '\0';
            }
        }
    }
    printf("✅ 预约签到成功！预约编号：%s，患者已转入待诊状态。\n", appointment->appointment_id);
    return 1;
}
