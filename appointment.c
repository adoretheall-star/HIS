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
/**
 * @brief 根据科室过滤并显示医生列表（强化按科室选医生功能）
 * @param dept_name 科室名称
 * @return 该科室的医生数量
 */
int display_doctors_by_dept(const char* dept_name)
{
    int count = 0;
    
    if (g_doctor_list == NULL)
    {
        printf("⚠️ 医生链表尚未初始化！\n");
        return 0;
    }
    
    if (dept_name == NULL || strlen(dept_name) == 0)
    {
        printf("⚠️ 科室名称不能为空！\n");
        return 0;
    }
    
    printf("\n🏥 【%s】科室医生列表：\n", dept_name);
    printf("--------------------------------------------------\n");
    printf("  编号\t姓名\t\t排队人数\n");
    printf("--------------------------------------------------\n");
    
    DoctorNode* curr = g_doctor_list->next;
    while (curr != NULL)
    {
        // 匹配科室
        if (strcmp(curr->department, dept_name) == 0)
        {
            count++;
            printf("  %s\t%s\t\t%d\n", 
                   curr->id, 
                   curr->name, 
                   curr->queue_length);
        }
        curr = curr->next;
    }
    
    printf("--------------------------------------------------\n");
    
    if (count == 0)
    {
        printf("😔 该科室暂无医生值班，请选择其他科室\n");
    }
    else
    {
        printf("📝 该科室共有 %d 位医生\n", count);
    }
    
    return count;
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
    
    // 特权穿透判定：直接判断患者是否为急诊
    if (patient->is_emergency == 1)
    {
        // 如果是急诊，跳过所有黑名单拦截
        if (patient->is_blacklisted != 0)
        {
            printf("🚨 [生命通道] 发现黑名单人员突发危重急症，已强制放行并报备相关部门！\n");
            // 添加急诊特权穿透预警
            char alert_msg[256];
            snprintf(alert_msg, sizeof(alert_msg), "🚨 急诊强制放行：患者 %s（黑名单人员）突发危重急症，已强制放行，请相关部门跟进！", patient->name);
            push_system_alert(alert_msg);
        }
    }
    else
    {
        // 非急诊情况，执行黑名单拦截
        
        // 逻辑 C：逃单拦截
        if (patient->is_blacklisted == 2)
        {
            printf("⛔ [黑名单拦截] 您的症状经预检分诊判定为非危重急症，拒绝提供常规诊疗服务，请先补缴欠费！\n");
            // 添加永久黑名单拦截预警
            char alert_msg[256];
            snprintf(alert_msg, sizeof(alert_msg), "⛔ 恶意挂号被拦截：患者 %s（永久黑名单）试图挂号，已拦截！", patient->name);
            push_system_alert(alert_msg);
            return NULL;
        }
        // 逻辑 B：爽约拦截
        else if (patient->is_blacklisted == 1)
        {
            time_t current_time = time(NULL);
            if (current_time > patient->blacklist_expire)
            {
                // 90天惩罚期已过，自动解禁
                patient->is_blacklisted = 0;
                patient->missed_times[0] = 0;
                patient->missed_times[1] = 0;
                patient->missed_times[2] = 0;
                patient->blacklist_expire = 0;
                printf("✅ 该患者的90天惩罚期已过，已自动解禁！\n");
            }
            else
            {
                // 还在惩罚期内
                // 检查是否为当天挂号（现场挂号）
                char today[20];
                time_t now = time(NULL);
                struct tm* local_time = localtime(&now);
                strftime(today, sizeof(today), "%Y-%m-%d", local_time);
                
                if (strcmp(appointment_date, today) != 0)
                {
                    // 不是当天，即提前预约，拦截
                    int days_remaining = (patient->blacklist_expire - current_time) / (24 * 3600) + 1;
                    printf("🔴 [爽约黑名单拦截] 该患者处于爽约惩罚期，限制提前预约！\n");
                    printf("🔴 请前往医院大厅现场挂号，距离解禁还有 %d 天！\n", days_remaining);
                    return NULL;
                }
                else
                {
                    // 是当天，即现场挂号，放行但打印信用警报
                    printf("⚠️ [信用警报] 您处于爽约惩罚期，请珍惜本次现场就诊机会！\n");
                }
            }
        }
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

// 登记预约爽约
int mark_appointment_missed(const char* appointment_id)
{
    AppointmentNode* appointment = NULL;
    PatientNode* patient = NULL;
    time_t current_time = 0;
    
    // 检查链表是否初始化
    if (g_appointment_list == NULL || g_patient_list == NULL)
    {
        printf("⚠️ 基础链表尚未初始化，无法登记爽约！\n");
        return 0;
    }
    
    // 查找预约记录
    appointment = find_appointment_by_id(g_appointment_list, appointment_id);
    if (appointment == NULL)
    {
        printf("⚠️ 未找到对应预约记录！\n");
        return 0;
    }
    
    // 检查预约状态
    if (appointment->appointment_status != RESERVED)
    {
        printf("⚠️ 当前预约状态为[%s]，不能执行爽约登记！\n",
            get_appointment_status_text(appointment->appointment_status));
        return 0;
    }
    
    // 查找对应患者
    patient = find_patient_by_id(g_patient_list, appointment->patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 对应患者不存在，爽约登记失败！\n");
        return 0;
    }
    
    // 更新预约状态为已过号
    appointment->appointment_status = MISSED;
    printf("✅ 预约爽约登记成功！预约编号：%s\n", appointment->appointment_id);
    
    // 执行惩罚逻辑
    current_time = time(NULL);
    
    // 将时间戳数组往后移位（丢弃最老的）
    patient->missed_times[2] = patient->missed_times[1];
    patient->missed_times[1] = patient->missed_times[0];
    patient->missed_times[0] = current_time;
    
    // 拉黑判定：30天内集齐3次爽约
    if (patient->missed_times[2] != 0 && (current_time - patient->missed_times[2]) <= 30 * 24 * 3600)
    {
        // 执行拉黑
        patient->is_blacklisted = 1;
        patient->blacklist_expire = current_time + 90 * 24 * 3600;
        printf("⛔ [信用分扣除及拉黑] 该患者在30天内爽约3次，已被限制挂号/预约权限90天！\n");
        printf("⚠️ 拉黑到期时间：%s\n", ctime(&patient->blacklist_expire));
    }
    
    return 1;
}
