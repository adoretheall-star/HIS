// ==========================================
// 文件名: inpatient_service.c
// 作用: 病房管理与住院业务流程实现
// 描述: 实现住院登记、床位分配、押金管理、日结计费、转床、出院等功能
// ==========================================

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include "global.h"
#include "list_ops.h"
#include "utils.h"
#include "admin_service.h"
#include "inpatient_service.h"

// 押金预警阈值常量
#define UNASSIGNED_DEPOSIT_WARNING_THRESHOLD 1000.0

// 全局变量（声明为 extern，实际定义在 list_ops.c 中）
extern InpatientRecord* g_inpatient_list;

// 病房费用标准
#define GENERAL_WARD_RATE 120.0  // 普通病房每天费用
#define ICU_WARD_RATE 500.0      // ICU每天费用

// ==========================================
// 内部辅助函数
// ==========================================

/**
 * @brief 生成下一个住院流水号
 * @param new_id 存储生成的住院流水号
 */
static void generate_inpatient_id(char* new_id)
{
    int max_no = 0;
    InpatientRecord* curr = NULL;

    if (new_id == NULL)
    {
        return;
    }

    if (g_inpatient_list == NULL || g_inpatient_list->next == NULL)
    {
        snprintf(new_id, MAX_ID_LEN, "I-001");
        return;
    }

    curr = g_inpatient_list->next;
    while (curr != NULL)
    {
        int no = 0;
        if (sscanf(curr->inpatient_id, "I-%d", &no) == 1)
        {
            if (no > max_no)
            {
                max_no = no;
            }
        }
        curr = curr->next;
    }

    snprintf(new_id, MAX_ID_LEN, "I-%03d", max_no + 1);
}

/**
 * @brief 获取病房类型的费用
 * @param ward_type 病房类型
 * @return 每天的费用
 */
static double get_ward_rate(WardType ward_type)
{
    return ward_type == WARD_TYPE_ICU ? ICU_WARD_RATE : GENERAL_WARD_RATE;
}

// ==========================================
// 1. 病房/床位查询函数
// ==========================================

void show_all_beds()
{
    WardNode* curr = NULL;
    int total = 0;
    int occupied = 0;

    if (g_ward_list == NULL || g_ward_list->next == NULL)
    {
        printf("⚠️ 床位链表尚未初始化或无床位数据！\n");
        return;
    }

    printf("\n======================================================\n");
    printf("                   全部床位信息\n");
    printf("======================================================\n");
    printf("病房编号       床位编号       类型         状态       患者编号\n");
    printf("------------------------------------------------------\n");

    curr = g_ward_list->next;
    while (curr != NULL)
    {
        total++;
        if (curr->is_occupied)
        {
            occupied++;
        }

        printf("%-13s %-13s %-12s %-10s %s\n",
            curr->room_id,
            curr->bed_id,
            curr->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房",
            curr->is_occupied ? "占用" : "空闲",
            curr->is_occupied ? curr->patient_id : "-");

        curr = curr->next;
    }

    printf("------------------------------------------------------\n");
    printf("总计：%d 张床位，其中 %d 张已占用，%d 张空闲\n",
        total, occupied, total - occupied);
    printf("======================================================\n");
}

void show_free_beds()
{
    WardNode* curr = NULL;
    int free_count = 0;

    if (g_ward_list == NULL || g_ward_list->next == NULL)
    {
        printf("⚠️ 床位链表尚未初始化或无床位数据！\n");
        return;
    }

    printf("\n======================================================\n");
    printf("                   空闲床位信息\n");
    printf("======================================================\n");
    printf("病房编号       床位编号       类型\n");
    printf("------------------------------------------------------\n");

    curr = g_ward_list->next;
    while (curr != NULL)
    {
        if (!curr->is_occupied)
        {
            free_count++;
            printf("%-13s %-13s %s\n",
                curr->room_id,
                curr->bed_id,
                curr->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
        }
        curr = curr->next;
    }

    printf("------------------------------------------------------\n");
    printf("总计：%d 张空闲床位\n", free_count);
    printf("======================================================\n");
}

void show_hospitalized_patients()
{
    InpatientRecord* curr = NULL;
    PatientNode* patient = NULL;
    WardNode* bed = NULL;
    int count = 0;

    if (g_inpatient_list == NULL || g_inpatient_list->next == NULL)
    {
        printf("⚠️ 住院记录链表尚未初始化或无住院患者！\n");
        return;
    }

    printf("\n======================================================\n");
    printf("                   住院患者信息\n");
    printf("======================================================\n");
    printf("住院号       患者编号     患者姓名   病房编号       床位编号       推荐类型     实际类型     已住天数  押金余额\n");
    printf("------------------------------------------------------\n");

    curr = g_inpatient_list->next;
    while (curr != NULL)
    {
        if (curr->is_active)
        {
            patient = find_patient_by_id(g_patient_list, curr->patient_id);
            if (patient != NULL)
            {
                count++;
                // 处理未分床的情况
                const char* actual_ward_type = "未分配";
                const char* room_id = "未分配";
                
                if (strlen(curr->bed_id) > 0)
                {
                    actual_ward_type = curr->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房";
                    // 通过床位编号查找病房编号
                    bed = find_bed_by_id(curr->bed_id);
                    if (bed != NULL)
                    {
                        room_id = bed->room_id;
                    }
                }
                
                printf("%-12s %-12s %-10s %-13s %-13s %-12s %-12s %-8d %.2f\n",
                    curr->inpatient_id,
                    curr->patient_id,
                    patient->name,
                    room_id,
                    strlen(curr->bed_id) > 0 ? curr->bed_id : "未分配",
                    curr->recommended_ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房",
                    actual_ward_type,
                    curr->days_stayed,
                    curr->deposit_balance);
            }
            else
            {
                count++;
                // 处理未分床的情况
                const char* actual_ward_type = "未分配";
                const char* room_id = "未分配";
                
                if (strlen(curr->bed_id) > 0)
                {
                    actual_ward_type = curr->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房";
                    // 通过床位编号查找病房编号
                    bed = find_bed_by_id(curr->bed_id);
                    if (bed != NULL)
                    {
                        room_id = bed->room_id;
                    }
                }
                
                printf("%-12s %-12s %-10s %-13s %-13s %-12s %-12s %-8d %.2f\n",
                    curr->inpatient_id,
                    curr->patient_id,
                    "患者档案缺失",
                    room_id,
                    strlen(curr->bed_id) > 0 ? curr->bed_id : "未分配",
                    curr->recommended_ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房",
                    actual_ward_type,
                    curr->days_stayed,
                    curr->deposit_balance);
            }
        }
        curr = curr->next;
    }

    printf("------------------------------------------------------\n");
    printf("总计：%d 位住院患者\n", count);
    printf("======================================================\n");
}

void show_inpatient_record_by_patient_id(const char* patient_id)
{
    InpatientRecord* curr = NULL;
    PatientNode* patient = NULL;
    WardNode* bed = NULL;

    if (is_blank_string(patient_id))
    {
        printf("⚠️ 患者编号不能为空！\n");
        return;
    }

    if (g_inpatient_list == NULL || g_inpatient_list->next == NULL)
    {
        printf("⚠️ 住院记录链表尚未初始化或无住院记录！\n");
        return;
    }

    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到该患者！\n");
        return;
    }

    printf("\n======================================================\n");
    printf("                 患者住院记录\n");
    printf("======================================================\n");
    printf("患者信息：%s - %s\n", patient->id, patient->name);
    printf("------------------------------------------------------\n");
    printf("住院号       病房编号       床位编号       推荐类型     实际类型     预计天数  已住天数  押金余额  状态\n");
    printf("------------------------------------------------------\n");

    curr = g_inpatient_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->patient_id, patient_id) == 0)
        {
            // 处理未分床的情况
            const char* actual_ward_type = "未分配";
            const char* room_id = "未分配";
            
            if (strlen(curr->bed_id) > 0)
            {
                actual_ward_type = curr->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房";
                // 通过床位编号查找病房编号
                bed = find_bed_by_id(curr->bed_id);
                if (bed != NULL)
                {
                    room_id = bed->room_id;
                }
            }
            
            printf("%-12s %-13s %-13s %-12s %-12s %-8d %-8d %.2f %s\n",
                curr->inpatient_id,
                room_id,
                strlen(curr->bed_id) > 0 ? curr->bed_id : "未分配",
                curr->recommended_ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房",
                actual_ward_type,
                curr->estimated_days,
                curr->days_stayed,
                curr->deposit_balance,
                curr->is_active ? "住院中" : "已出院");
        }
        curr = curr->next;
    }

    printf("======================================================\n");
}

// ==========================================
// 2. 住院流程函数
// ==========================================

int register_inpatient(const char* patient_id, int estimated_days, double deposit, int condition_level)
{
    PatientNode* patient = NULL;
    InpatientRecord* existing_record = NULL;
    InpatientRecord* new_record = NULL;
    char new_inpatient_id[MAX_ID_LEN];
    WardType recommended_ward_type = WARD_TYPE_GENERAL;

    // 输入校验
    if (is_blank_string(patient_id))
    {
        printf("⚠️ 患者编号不能为空！\n");
        return 0;
    }

    if (estimated_days <= 0)
    {
        printf("⚠️ 预计住院天数必须大于 0！\n");
        return 0;
    }

    if (deposit <= 0)
    {
        printf("⚠️ 押金必须大于 0！\n");
        return 0;
    }

    // 根据病情级别确定推荐病房类型
    if (condition_level == 2) // 重症
    {
        recommended_ward_type = WARD_TYPE_ICU;
    }
    else // 普通
    {
        recommended_ward_type = WARD_TYPE_GENERAL;
    }

    // 检查患者是否存在
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到该患者，请先为患者建档！\n");
        return 0;
    }

    // 检查患者是否已住院
    existing_record = find_active_inpatient_by_patient_id(patient_id);
    if (existing_record != NULL)
    {
        printf("⚠️ 该患者已经在住院中，不能重复住院登记！\n");
        return 0;
    }

    // 初始化住院记录链表（押金已在调用前扣除，余额已检查）

    if (g_inpatient_list == NULL)
    {
        g_inpatient_list = create_inpatient_record_head();
        if (g_inpatient_list == NULL)
        {
            printf("⚠️ 住院记录链表初始化失败！\n");
            return 0;
        }
    }

    // 生成住院流水号
    generate_inpatient_id(new_inpatient_id);

    // 创建住院记录
    new_record = create_inpatient_record_node(
        new_inpatient_id,
        patient_id,
        "",  // 初始无床位
        WARD_TYPE_GENERAL,  // 初始为普通病房
        recommended_ward_type,  // 推荐病房类型
        estimated_days,
        0,  // 已住院天数初始为0
        deposit,
        1   // 活跃状态
    );

    if (new_record == NULL)
    {
        printf("⚠️ 创建住院记录失败！\n");
        return 0;
    }

    // 插入住院记录
    insert_inpatient_record_tail(g_inpatient_list, new_record);

    // 更新患者状态为住院中（押金已在调用前扣除）
    patient->status = STATUS_HOSPITALIZED;

    // 记录日志
    char description[200];
    snprintf(description, sizeof(description), "患者 %s (%s) 住院登记，预计住院 %d 天，押金 %.2f 元，推荐病房类型：%s",
        patient->name, patient_id, estimated_days, deposit,
        recommended_ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
    add_log("住院登记", patient_id, description);

    printf("✅ 住院登记成功！住院号：%s\n", new_inpatient_id);
    printf("📋 推荐病房类型：%s\n", recommended_ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
    return 1;
}

int assign_bed_to_patient(const char* patient_id, const char* bed_id)
{
    PatientNode* patient = NULL;
    InpatientRecord* inpatient_record = NULL;
    WardNode* bed = NULL;

    // 输入校验
    if (is_blank_string(patient_id))
    {
        printf("⚠️ 患者编号不能为空！\n");
        return 0;
    }

    if (is_blank_string(bed_id))
    {
        printf("⚠️ 床位编号不能为空！\n");
        return 0;
    }

    // 检查患者是否存在
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到该患者！\n");
        return 0;
    }

    // 检查患者是否已住院
    inpatient_record = find_active_inpatient_by_patient_id(patient_id);
    if (inpatient_record == NULL)
    {
        printf("⚠️ 该患者未住院，无法分配床位！\n");
        return 0;
    }

    // 检查患者是否已占用床位
    if (patient_has_bed(patient_id))
    {
        printf("⚠️ 该患者已经占用床位，不能重复分配！\n");
        return 0;
    }

    // 显示推荐病房类型
    printf("📋 推荐病房类型：%s\n", 
        inpatient_record->recommended_ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");

    // 显示推荐类型可用床位
    printf("\n💡 推荐类型可用床位：\n");
    
    if (g_ward_list == NULL)
    {
        printf("  ⚠️ 床位链表未初始化，暂无可用床位！\n");
    }
    else
    {
        WardNode* curr_bed = g_ward_list->next;
        int has_recommended_beds = 0;
        while (curr_bed != NULL)
        {
            if (!curr_bed->is_occupied && curr_bed->ward_type == inpatient_record->recommended_ward_type)
            {
                printf("  - %s (%s)\n", curr_bed->bed_id, 
                    curr_bed->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
                has_recommended_beds = 1;
            }
            curr_bed = curr_bed->next;
        }
        if (!has_recommended_beds)
        {
            printf("  无推荐类型可用床位\n");
        }
    }

    // 检查床位是否存在且可用
    bed = find_bed_by_id(bed_id);
    if (bed == NULL)
    {
        printf("⚠️ 未找到该床位！\n");
        return 0;
    }

    if (bed->is_occupied)
    {
        printf("⚠️ 该床位已被占用，无法分配！\n");
        return 0;
    }

    // 检查床位类型是否与推荐类型一致
    if (bed->ward_type != inpatient_record->recommended_ward_type)
    {
        printf("⚠️ 所选床位类型与推荐类型不一致！\n");
        printf("   推荐：%s，实际：%s\n", 
            inpatient_record->recommended_ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房",
            bed->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
        printf("   继续分配？(1=是/0=否): ");
        int confirm = get_safe_int("");
        if (confirm != 1)
        {
            printf("❌ 分配取消！\n");
            return 0;
        }
    }

    // 分配床位
    bed->is_occupied = 1;
    strncpy(bed->patient_id, patient_id, MAX_ID_LEN - 1);
    bed->patient_id[MAX_ID_LEN - 1] = '\0';

    // 更新住院记录
    strncpy(inpatient_record->bed_id, bed_id, MAX_ID_LEN - 1);
    inpatient_record->bed_id[MAX_ID_LEN - 1] = '\0';
    inpatient_record->ward_type = bed->ward_type;

    // 记录日志
    char description[200];
    if (inpatient_record->recommended_ward_type == bed->ward_type)
    {
        snprintf(description, sizeof(description), "为患者 %s (%s) 分配床位 %s，类型：%s（系统推荐类型）",
            patient->name, patient_id, bed_id,
            bed->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
    }
    else
    {
        if (inpatient_record->recommended_ward_type == WARD_TYPE_ICU)
        {
            snprintf(description, sizeof(description), "为患者 %s (%s) 分配床位 %s，类型：%s（系统推荐 ICU，人工调整为普通病房）",
                patient->name, patient_id, bed_id,
                bed->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
        }
        else
        {
            snprintf(description, sizeof(description), "为患者 %s (%s) 分配床位 %s，类型：%s（系统推荐普通病房，人工调整为 ICU）",
                patient->name, patient_id, bed_id,
                bed->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
        }
    }
    add_log("床位分配", patient_id, description);

    printf("✅ 床位分配成功！患者 %s 已分配到床位 %s\n",
        patient->name, bed_id);
    return 1;
}

int transfer_bed(const char* patient_id, const char* old_bed_id, const char* new_bed_id)
{
    PatientNode* patient = NULL;
    InpatientRecord* inpatient_record = NULL;
    WardNode* old_bed = NULL;
    WardNode* new_bed = NULL;

    // 输入校验
    if (is_blank_string(patient_id))
    {
        printf("⚠️ 患者编号不能为空！\n");
        return 0;
    }

    if (is_blank_string(old_bed_id))
    {
        printf("⚠️ 原床位编号不能为空！\n");
        return 0;
    }

    if (is_blank_string(new_bed_id))
    {
        printf("⚠️ 新床位编号不能为空！\n");
        return 0;
    }

    // 检查患者是否存在
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到该患者！\n");
        return 0;
    }

    // 检查患者是否已住院
    inpatient_record = find_active_inpatient_by_patient_id(patient_id);
    if (inpatient_record == NULL)
    {
        printf("⚠️ 该患者未住院，无法转床！\n");
        return 0;
    }

    // 校验：用户输入的原床位应与住院记录中的床位一致
    if (strcmp(inpatient_record->bed_id, old_bed_id) != 0)
    {
        printf("⚠️ 输入的原床位 %s 与当前住院记录床位 %s 不一致！\n", 
            old_bed_id, inpatient_record->bed_id);
        return 0;
    }

    // 检查原床位是否存在且为该患者占用
    old_bed = find_bed_by_id(old_bed_id);
    if (old_bed == NULL)
    {
        printf("⚠️ 未找到原床位！\n");
        return 0;
    }

    if (!old_bed->is_occupied || strcmp(old_bed->patient_id, patient_id) != 0)
    {
        printf("⚠️ 原床位未被该患者占用！\n");
        return 0;
    }

    // 检查新床位是否存在且可用
    new_bed = find_bed_by_id(new_bed_id);
    if (new_bed == NULL)
    {
        printf("⚠️ 未找到新床位！\n");
        return 0;
    }

    if (new_bed->is_occupied)
    {
        printf("⚠️ 新床位已被占用，无法转床！\n");
        return 0;
    }

    // 检查原床位和新床位是否相同
    if (strcmp(old_bed_id, new_bed_id) == 0)
    {
        printf("⚠️ 原床位和新床位相同，无需转床！\n");
        return 0;
    }

    // 执行转床
    // 释放原床位
    old_bed->is_occupied = 0;
    old_bed->patient_id[0] = '\0';

    // 占用新床位
    new_bed->is_occupied = 1;
    strncpy(new_bed->patient_id, patient_id, MAX_ID_LEN - 1);
    new_bed->patient_id[MAX_ID_LEN - 1] = '\0';

    // 更新住院记录
    strncpy(inpatient_record->bed_id, new_bed_id, MAX_ID_LEN - 1);
    inpatient_record->bed_id[MAX_ID_LEN - 1] = '\0';
    inpatient_record->ward_type = new_bed->ward_type;

    // 记录日志
    char description[200];
    snprintf(description, sizeof(description), "患者 %s (%s) 从床位 %s 转至床位 %s",
        patient->name, patient_id, old_bed_id, new_bed_id);
    add_log("转床", patient_id, description);

    printf("✅ 转床成功！患者 %s 已从床位 %s 转至床位 %s\n",
        patient->name, old_bed_id, new_bed_id);
    return 1;
}

int discharge_patient(const char* patient_id)
{
    PatientNode* patient = NULL;
    InpatientRecord* inpatient_record = NULL;
    WardNode* bed = NULL;
    double refund_amount = 0.0;

    // 输入校验
    if (is_blank_string(patient_id))
    {
        printf("⚠️ 患者编号不能为空！\n");
        return 0;
    }

    // 检查患者是否存在
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到该患者！\n");
        return 0;
    }

    // 检查患者是否已住院
    inpatient_record = find_active_inpatient_by_patient_id(patient_id);
    if (inpatient_record == NULL)
    {
        printf("⚠️ 该患者未住院，无法办理出院！\n");
        return 0;
    }

    // 释放床位
    if (strlen(inpatient_record->bed_id) > 0)
    {
        bed = find_bed_by_id(inpatient_record->bed_id);
        if (bed != NULL)
        {
            bed->is_occupied = 0;
            bed->patient_id[0] = '\0';
        }
    }

    // 更新住院记录
    refund_amount = inpatient_record->deposit_balance;
    if (refund_amount > 0)
    {
        patient->balance += refund_amount;
    }

    inpatient_record->is_active = 0;
    inpatient_record->deposit_balance = 0.0;

    // 更新患者状态为已完成
    patient->status = STATUS_COMPLETED;

    // 记录日志
    char description[200];
    snprintf(description, sizeof(description), "患者 %s (%s) 出院，住院 %d 天，剩余押金 %.2f 元",
        patient->name, patient_id, inpatient_record->days_stayed, refund_amount);
    add_log("出院", patient_id, description);

    printf("✅ 出院办理成功！患者 %s 已出院\n", patient->name);
    printf("住院天数：%d 天\n", inpatient_record->days_stayed);
    printf("剩余押金：%.2f 元\n", refund_amount);
    return 1;
}

// ==========================================
// 3. 押金与计费函数
// ==========================================

int recharge_inpatient_deposit(const char* patient_id, double amount)
{
    PatientNode* patient = NULL;
    InpatientRecord* inpatient_record = NULL;

    // 输入校验
    if (is_blank_string(patient_id))
    {
        printf("⚠️ 患者编号不能为空！\n");
        return 0;
    }

    if (amount <= 0)
    {
        printf("⚠️ 充值金额必须大于 0！\n");
        return 0;
    }

    // 检查患者是否存在
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到该患者！\n");
        return 0;
    }

    // 检查患者是否已住院
    inpatient_record = find_active_inpatient_by_patient_id(patient_id);
    if (inpatient_record == NULL)
    {
        printf("⚠️ 该患者未住院，无法充值押金！\n");
        return 0;
    }

    // 充值押金
    if (patient->balance < amount)
    {
        printf("⚠️ 您的账户余额不足，无法充值押金！\n");
        printf("   当前余额：%.2f 元，本次充值金额：%.2f 元\n", patient->balance, amount);
        return 0;
    }

    patient->balance -= amount;
    inpatient_record->deposit_balance += amount;

    // 记录日志
    char description[200];
    snprintf(description, sizeof(description), "为患者 %s (%s) 充值住院押金 %.2f 元",
        patient->name, patient_id, amount);
    add_log("押金充值", patient_id, description);

    printf("✅ 押金充值成功！当前押金余额：%.2f 元\n",
        inpatient_record->deposit_balance);
    return 1;
}

int daily_settlement(const char* patient_id)
{
    PatientNode* patient = NULL;
    InpatientRecord* inpatient_record = NULL;
    double daily_rate = 0.0;

    // 输入校验
    if (is_blank_string(patient_id))
    {
        printf("⚠️ 患者编号不能为空！\n");
        return 0;
    }

    // 检查患者是否存在
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到该患者！\n");
        return 0;
    }

    // 检查患者是否已住院
    inpatient_record = find_active_inpatient_by_patient_id(patient_id);
    if (inpatient_record == NULL)
    {
        printf("⚠️ 该患者未住院，无法执行日结计费！\n");
        return 0;
    }

    // 检查患者是否已分配床位
    if (strlen(inpatient_record->bed_id) == 0)
    {
        printf("⚠️ 该患者未分配床位，无法执行日结计费！\n");
        return 0;
    }
    
    // 检查当天是否已结算
    time_t now = time(NULL);
    struct tm* local_time = localtime(&now);
    char today[11];
    strftime(today, sizeof(today), "%Y-%m-%d", local_time);
    
    char last_settlement_date[11];
    struct tm* settlement_time = localtime((time_t*)&inpatient_record->last_settlement_time);
    if (inpatient_record->last_settlement_time > 0)
    {
        strftime(last_settlement_date, sizeof(last_settlement_date), "%Y-%m-%d", settlement_time);
        if (strcmp(today, last_settlement_date) == 0)
        {
            printf("⚠️ 该患者今日已完成日结，不能重复结算！\n");
            return 0;
        }
    }

    // 计算当日费用
    daily_rate = get_ward_rate(inpatient_record->ward_type);

    // 检查押金是否足够
    if (inpatient_record->deposit_balance < daily_rate)
    {
        printf("⚠️ 押金不足，无法执行日结计费！\n");
        printf("当前押金余额：%.2f 元，当日费用：%.2f 元\n",
            inpatient_record->deposit_balance, daily_rate);
        return 0;
    }

    // 执行日结计费
    inpatient_record->days_stayed++;
    inpatient_record->deposit_balance -= daily_rate;
    inpatient_record->last_settlement_time = (long)now;

    // 记录日志
    char description[200];
    snprintf(description, sizeof(description), "对患者 %s (%s) 执行日结计费 %.2f 元，已住 %d 天",
        patient->name, patient_id, daily_rate, inpatient_record->days_stayed);
    add_log("日结计费", patient_id, description);

    printf("✅ 日结计费成功！\n");
    printf("已住天数：%d 天\n", inpatient_record->days_stayed);
    printf("当日费用：%.2f 元\n", daily_rate);
    printf("剩余押金：%.2f 元\n", inpatient_record->deposit_balance);
    return 1;
}

void show_deposit_warnings()
{
    InpatientRecord* curr = NULL;
    PatientNode* patient = NULL;
    double daily_rate = 0.0;
    int warning_count = 0;

    if (g_inpatient_list == NULL || g_inpatient_list->next == NULL)
    {
        printf("⚠️ 住院记录链表尚未初始化或无住院患者！\n");
        return;
    }

    printf("\n======================================================\n");
    printf("                   押金预警信息\n");
    printf("======================================================\n");
    printf("患者编号     患者姓名   病房编号       床位编号       推荐类型     实际类型     已住天数  押金余额  预警状态\n");
    printf("------------------------------------------------------\n");

    curr = g_inpatient_list->next;
    while (curr != NULL)
    {
        if (curr->is_active)
        {
            patient = find_patient_by_id(g_patient_list, curr->patient_id);
            if (patient != NULL)
            {
                // 处理未分床的情况
                const char* actual_ward_type = "未分配";
                const char* room_id = "未分配";
                if (strlen(curr->bed_id) > 0)
                {
                    actual_ward_type = curr->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房";
                    // 通过床位编号查找病房编号
                    WardNode* bed = find_bed_by_id(curr->bed_id);
                    if (bed != NULL)
                    {
                        room_id = bed->room_id;
                    }
                }
                
                // 计算每日费用（如果有床位）
                double daily_rate = 0.0;
                if (strlen(curr->bed_id) > 0)
                {
                    daily_rate = get_ward_rate(curr->ward_type);
                }
                
                // 检查押金是否不足
                if (strlen(curr->bed_id) > 0 && curr->deposit_balance < daily_rate * 3)  // 不足3天费用
                {
                    warning_count++;
                    printf("%-12s %-10s %-13s %-13s %-12s %-12s %-8d %.2f %s\n",
                        curr->patient_id,
                        patient->name,
                        room_id,
                        curr->bed_id,
                        curr->recommended_ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房",
                        actual_ward_type,
                        curr->days_stayed,
                        curr->deposit_balance,
                        "押金不足");
                }
                else if (strlen(curr->bed_id) == 0)  // 未分床但押金较少
                {
                    // 根据推荐病房类型计算预警阈值
                    double warning_threshold = 0.0;
                    if (curr->recommended_ward_type == WARD_TYPE_ICU)
                    {
                        warning_threshold = get_ward_rate(WARD_TYPE_ICU) * 3;  // ICU 3天费用
                    }
                    else
                    {
                        warning_threshold = get_ward_rate(WARD_TYPE_GENERAL) * 3;  // 普通病房 3天费用
                    }
                    
                    if (curr->deposit_balance < warning_threshold)
                    {
                        warning_count++;
                    printf("%-12s %-10s %-13s %-13s %-12s %-12s %-8d %.2f %s\n",
                        curr->patient_id,
                        patient->name,
                        room_id,
                        "-",
                        curr->recommended_ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房",
                        actual_ward_type,
                        curr->days_stayed,
                        curr->deposit_balance,
                        "押金较少");
                    }
                }
            }
        }
        curr = curr->next;
    }

    printf("------------------------------------------------------\n");
    if (warning_count == 0)
    {
        printf("暂无押金预警信息\n");
    }
    else
    {
        printf("总计：%d 位患者押金不足\n", warning_count);
    }
    printf("======================================================\n");
}

// ==========================================
// 4. 辅助校验函数
// ==========================================

WardNode* find_bed_by_id(const char* bed_id)
{
    WardNode* curr = NULL;

    if (is_blank_string(bed_id) || g_ward_list == NULL)
    {
        return NULL;
    }

    curr = g_ward_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->bed_id, bed_id) == 0)
        {
            return curr;
        }
        curr = curr->next;
    }

    return NULL;
}

WardNode* find_free_bed_by_type(WardType ward_type)
{
    WardNode* curr = NULL;

    if (g_ward_list == NULL)
    {
        return NULL;
    }

    curr = g_ward_list->next;
    while (curr != NULL)
    {
        if (!curr->is_occupied && curr->ward_type == ward_type)
        {
            return curr;
        }
        curr = curr->next;
    }

    return NULL;
}

InpatientRecord* find_active_inpatient_by_patient_id(const char* patient_id)
{
    InpatientRecord* curr = NULL;

    if (is_blank_string(patient_id) || g_inpatient_list == NULL)
    {
        return NULL;
    }

    curr = g_inpatient_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->patient_id, patient_id) == 0 && curr->is_active)
        {
            return curr;
        }
        curr = curr->next;
    }

    return NULL;
}

int patient_has_bed(const char* patient_id)
{
    WardNode* curr = NULL;

    if (is_blank_string(patient_id) || g_ward_list == NULL)
    {
        return 0;
    }

    curr = g_ward_list->next;
    while (curr != NULL)
    {
        if (curr->is_occupied && strcmp(curr->patient_id, patient_id) == 0)
        {
            return 1;
        }
        curr = curr->next;
    }

    return 0;
}

int bed_is_available(const char* bed_id)
{
    WardNode* bed = find_bed_by_id(bed_id);
    return (bed != NULL && !bed->is_occupied);
}
