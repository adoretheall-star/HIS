// ==========================================
// 文件名: patient_service.c
// 作用: 患者建档、档案管理与基础病历查询
// 描述: 实现患者信息管理的核心业务逻辑，包括患者建档、档案查询、信息修改等功能
// ==========================================
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "list_ops.h"
#include "utils.h"
#include "patient_service.h"

// ==========================================
// 内部辅助函数
// ==========================================

/**
 * @brief 生成下一个患者编号
 * @param new_id 存储生成的患者编号
 * @details 格式为 "P-XXX"，其中 XXX 为三位数字，从 001 开始递增
 */
static void generate_patient_id(char* new_id)
{
    int max_no = 0;
    PatientNode* curr = NULL;

    // 参数校验
    if (new_id == NULL)
    {
        return;
    }

    // 如果患者链表为空，从 P-001 开始
    if (g_patient_list == NULL)
    {
        snprintf(new_id, MAX_ID_LEN, "P-001");
        return;
    }

    // 遍历患者链表，查找最大的编号
    curr = g_patient_list->next;
    while (curr != NULL)
    {
        // 检查是否为患者编号格式 (P-XXX)
        if (strncmp(curr->id, "P-", 2) == 0)
        {
            // 提取数字部分并转换为整数
            int current_no = atoi(curr->id + 2);
            if (current_no > max_no)
            {
                max_no = current_no;
            }
        }
        curr = curr->next;
    }

    // 生成新的编号，格式为 P-XXX
    snprintf(new_id, MAX_ID_LEN, "P-%03d", max_no + 1);
}
/**
 * @brief 根据身份证号查找患者
 * @param id_card 患者身份证号
 * @return 找到返回患者节点指针，未找到返回NULL
 */
PatientNode* find_patient_by_id_card(const char* id_card)
{
    PatientNode* curr = NULL;
    
    // 参数校验
    if (g_patient_list == NULL || id_card == NULL) 
        return NULL;
    
    // 遍历患者链表查找匹配的身份证号
    curr = g_patient_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->id_card, id_card) == 0)
        {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}
<<<<<<< HEAD

/**
 * @brief 安全地复制文本字段
 * @param dest 目标缓冲区
 * @param max_len 目标缓冲区最大长度
 * @param src 源文本
 * @details 使用 strncpy 确保不会发生缓冲区溢出
 */
static void copy_text_field(char* dest, int max_len, const char* src)
{
    if (dest == NULL || max_len <= 0)
    {
        return;
    }

    // 使用 strncpy 进行安全复制，确保以 '\0' 结尾
    strncpy(dest, src, max_len - 1);
    dest[max_len - 1] = '\0';
}

/**
 * @brief 获取显示文本（处理空值）
 * @param text 原始文本
 * @return 如果文本为空返回"暂无"，否则返回原始文本
 */
static const char* get_display_text(const char* text)
{
    if (text == NULL || text[0] == '\0')
    {
        return "暂无";
    }

    return text;
}

/**
 * @brief 获取汉字的拼音首字母
 * @param c 汉字字符
 * @return 拼音首字母（大写），非汉字返回原字符
 */
static char get_pinyin_first_letter(char c)
{
    unsigned char uc = (unsigned char)c;
    
    // ASCII字符直接返回大写
    if (uc < 0x80)
    {
        if (c >= 'a' && c <= 'z')
            return c - 32;
        return c;
    }
    
    // 常见汉字拼音首字母映射表（简化版）
    if (uc >= 0xB0 && uc <= 0xF7)
    {
        // 根据汉字的GB2312编码范围判断首字母
        if (uc == 0xB0) return 'A';
        if (uc == 0xB1) return 'B';
        if (uc == 0xB2) return 'C';
        if (uc == 0xB3) return 'D';
        if (uc == 0xB4) return 'E';
        if (uc == 0xB5) return 'F';
        if (uc == 0xB6) return 'G';
        if (uc == 0xB7) return 'H';
        if (uc == 0xB8) return 'J';
        if (uc == 0xB9) return 'K';
        if (uc == 0xBA) return 'L';
        if (uc == 0xBB) return 'M';
        if (uc == 0xBC) return 'N';
        if (uc == 0xBD) return 'O';
        if (uc == 0xBE) return 'P';
        if (uc == 0xBF) return 'Q';
        if (uc == 0xC0) return 'R';
        if (uc == 0xC1) return 'S';
        if (uc == 0xC2) return 'T';
        if (uc == 0xC3) return 'W';
        if (uc == 0xC4) return 'X';
        if (uc == 0xC5) return 'Y';
        if (uc == 0xC6) return 'Z';
    }
    
    return c;
}

/**
 * @brief 将中文姓名转换为拼音首字母
 * @param name 中文姓名
 * @param pinyin 存储拼音首字母的缓冲区
 * @param max_len 缓冲区最大长度
 */
static void name_to_pinyin(const char* name, char* pinyin, int max_len)
{
    if (name == NULL || pinyin == NULL || max_len <= 0)
        return;
    
    int i = 0;
    int pos = 0;
    
    while (name[i] != '\0' && pos < max_len - 1)
    {
        unsigned char uc = (unsigned char)name[i];
        
        if (uc < 0x80)
        {
            // ASCII字符
            pinyin[pos++] = get_pinyin_first_letter(name[i]);
            i++;
        }
        else
        {
            // 汉字（GB2312编码，2字节）
            pinyin[pos++] = get_pinyin_first_letter(name[i]);
            i += 2;
        }
    }
    
    pinyin[pos] = '\0';
}

/**
 * @brief 获取预约状态文本描述
 * @param status 预约状态枚举值
 * @return 状态对应的中文描述字符串
 */
static const char* get_appointment_status_text(AppointmentStatus status)
{
    switch (status)
    {
        case RESERVED:
            return "已预约";
        case CHECKED_IN:
            return "已签到";
        case CANCELLED:
            return "已取消";
        case MISSED:
            return "已过号";
        default:
            return "未知状态";
    }
}

/**
 * @brief 根据患者编号查找最近一次预约记录
 * @param patient_id 患者编号
 * @return 找到返回预约节点指针，未找到返回NULL
 */
static AppointmentNode* find_latest_appointment_by_patient_id(const char* patient_id)
{
    AppointmentNode* curr = NULL;
    AppointmentNode* latest = NULL;

    // 参数校验
    if (g_appointment_list == NULL || patient_id == NULL || patient_id[0] == '\0')
    {
        return NULL;
    }

    // 遍历预约链表查找该患者的所有预约记录
    curr = g_appointment_list->next;
    while (curr != NULL)
    {
        if (strcmp(curr->patient_id, patient_id) == 0)
        {
            latest = curr;
        }
        curr = curr->next;
    }

    return latest;
}

/**
 * @brief 根据患者编号获取患者节点（带错误检查）
 * @param patient_id 患者编号
 * @param action_name 操作名称（用于错误提示）
 * @return 成功返回患者节点指针，失败返回NULL
 */
static PatientNode* get_patient_by_id_checked(const char* patient_id, const char* action_name)
{
    PatientNode* patient = NULL;

    // 检查患者链表是否初始化
    if (g_patient_list == NULL)
    {
        printf("提示：患者链表尚未初始化，无法%s！\n", action_name);
        return NULL;
    }

    // 检查患者编号是否为空
    if (patient_id == NULL || patient_id[0] == '\0')
    {
        printf("提示：患者编号不能为空！\n");
        return NULL;
    }

    // 查找患者
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("未找到对应患者档案！\n");
        return NULL;
    }

    return patient;
}

/**
 * @brief 获取患者状态文本描述
 * @param status 患者就医状态枚举值
 * @return 状态对应的中文描述字符串
 */
const char* get_patient_status_text(MedStatus status)
{
    switch (status)
    {
        case STATUS_PENDING:
            return "待诊";
        case STATUS_EXAMINING:
            return "检查中";
        case STATUS_UNPAID:
            return "已看诊待缴费";
        case STATUS_WAIT_MED:
            return "已缴费待取药";
        case STATUS_HOSPITALIZED:
            return "住院中";
        case STATUS_COMPLETED:
            return "就诊结束";
        default:
            return "未知状态";
    }
}

// ==========================================
// 患者建档功能
// ==========================================

/**
 * @brief 患者建档
 * @param name 患者姓名
 * @param age 患者年龄
 * @param id_card 患者身份证号
 * @param symptom 症状描述
 * @param target_dept 目标科室
 * @return 成功返回患者节点指针，失败返回NULL
 */
PatientNode* register_patient(
    const char* name,
    int age,
    const char* id_card,
    const char* symptom,
    const char* target_dept
)
=======
// 患者建档
PatientNode* register_patient(const char* name, int age, const char* id_card, const char* symptom)
>>>>>>> medicine
{
    char new_id[MAX_ID_LEN];
    PatientNode* new_patient = NULL;
    
    // 检查患者链表是否初始化
    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化，无法建档！\n");
        return NULL;
    }
    
    // 检查患者姓名是否为空
    if (name == NULL || strlen(name) == 0)
    {
        printf("⚠️ 患者姓名不能为空！\n");
        return NULL;
    }
    
    // 检查患者姓名长度是否合法
    int name_len = strlen(name);
    if (name_len < 2 || name_len > 20)
    {
        printf("⚠️ 患者姓名长度必须在 2-20 个字符之间！\n");
        return NULL;
    }
    
    // 检查患者年龄是否合法
    if (age < 0 || age > 130)
    {
        printf("⚠️ 患者年龄必须在 0-130 岁之间！\n");
        return NULL;
    }
    
    // 检查身份证号格式是否合法
    if (id_card == NULL || !validate_id_card(id_card))
    {
        printf("⚠️ 身份证号格式不合法，建档失败！\n");
        return NULL;
    }
    
    // 检查身份证号是否已存在
    if (find_patient_by_id_card(id_card) != NULL)
    {
        printf("⚠️ 该身份证号已存在，不能重复建档！\n");
        return NULL;
    }
    
    // 生成新的患者编号
    generate_patient_id(new_id);
    
    // 创建患者节点
    new_patient = create_patient_node(new_id, name, age, id_card);
    if (new_patient == NULL)
    {
        printf("⚠️ 患者节点创建失败！\n");
        return NULL;
    }
<<<<<<< HEAD

    // 复制患者信息到节点中
    copy_text_field(new_patient->id_card, MAX_ID_LEN, id_card);
    copy_text_field(new_patient->symptom, MAX_SYMPTOM_LEN, symptom);
    copy_text_field(new_patient->target_dept, MAX_NAME_LEN, target_dept);

    // 将患者节点插入到链表尾部
=======
    
    // 设置症状描述
    if (symptom != NULL && strlen(symptom) > 0)
    {
        strncpy(new_patient->symptom, symptom, MAX_SYMPTOM_LEN - 1);
        new_patient->symptom[MAX_SYMPTOM_LEN - 1] = '\0';
    }
    
>>>>>>> medicine
    insert_patient_tail(g_patient_list, new_patient);

    return new_patient;
}

// ==========================================
// 患者档案管理功能
// ==========================================

/**
 * @brief 显示患者档案详细信息
 * @param patient 患者节点指针
 */
void display_patient_archive(const PatientNode* patient)
{
    char masked_id[19] = "";

    // 参数校验
    if (patient == NULL)
    {
        return;
    }

    // 处理身份证号脱敏
    if (strlen(patient->id_card) > 0)
    {
        mask_id_card(patient->id_card, masked_id);
    }
    else
    {
        copy_text_field(masked_id, (int)sizeof(masked_id), get_display_text(patient->id_card));
    }

    // 显示患者档案信息
    printf("\n================ 患者档案信息 ================\n");
    printf("患者编号：%s\n", patient->id);
    printf("姓名：%s\n", patient->name);
    printf("年龄：%d\n", patient->age);
    printf("脱敏身份证号：%s\n", masked_id);
    printf("症状：%s\n", get_display_text(patient->symptom));
    printf("目标科室：%s\n", get_display_text(patient->target_dept));
    printf("当前状态：%s\n", get_patient_status_text(patient->status));
    printf("账户余额：%.2f\n", patient->balance);
    printf("==============================================\n");
}

/**
 * @brief 根据患者编号查询档案
 * @param patient_id 患者编号
 * @return 成功返回1，失败返回0
 */
int query_patient_archive_by_id(const char* patient_id)
{
    PatientNode* patient = NULL;

    // 获取患者节点（带错误检查）
    patient = get_patient_by_id_checked(patient_id, "查询患者档案");
    if (patient == NULL)
    {
        return 0;
    }

    // 显示患者档案信息
    display_patient_archive(patient);
    return 1;
}

/**
 * @brief 根据身份证号查询档案
 * @param id_card 患者身份证号
 * @return 成功返回1，失败返回0
 */
int query_patient_archive_by_id_card(const char* id_card)
{
    PatientNode* patient = NULL;
    
    // 检查患者链表是否初始化
    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化，无法查询档案！\n");
        return 0;
    }
    
    // 检查身份证号是否为空
    if (id_card == NULL || strlen(id_card) == 0)
    {
        printf("⚠️ 身份证号不能为空！\n");
        return 0;
    }
    
    // 检查身份证号格式是否合法
    if (!validate_id_card(id_card))
    {
        printf("⚠️ 身份证号格式不合法！\n");
        return 0;
    }
    
    // 根据身份证号查找患者
    patient = find_patient_by_id_card(id_card);
    if (patient == NULL)
    {
        printf("⚠️ 未找到对应患者档案！\n");
        return 0;
    }
    
    // 显示患者档案信息
    display_patient_archive(patient);
    return 1;
}

/**
 * @brief 根据姓名关键词查询档案（支持模糊查询和拼音首字母搜索）
 * @param name_keyword 姓名关键词（可以是汉字或拼音首字母）
 * @return 成功返回1，失败返回0
 */
int query_patient_archive_by_name(const char* name_keyword)
{
    PatientNode* curr = NULL;
    int found = 0;
    char pinyin[MAX_NAME_LEN];
    
    // 检查患者链表是否初始化
    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化，无法查询档案！\n");
        return 0;
    }
    
    // 检查姓名关键词是否为空
    if (name_keyword == NULL || strlen(name_keyword) == 0)
    {
        printf("⚠️ 患者姓名不能为空！\n");
        return 0;
    }
    
    // 遍历患者链表，查找匹配的姓名
    curr = g_patient_list->next;
    while (curr != NULL)
    {
        // 1. 子串匹配（支持汉字模糊查询）
        if (strstr(curr->name, name_keyword) != NULL)
        {
            display_patient_archive(curr);
            found = 1;
        }
        else
        {
            // 2. 拼音首字母匹配（支持拼音首字母搜索）
            name_to_pinyin(curr->name, pinyin, MAX_NAME_LEN);
            if (strstr(pinyin, name_keyword) != NULL)
            {
                display_patient_archive(curr);
                found = 1;
            }
        }
        curr = curr->next;
    }
    
    // 如果没有找到匹配的患者
    if (!found)
    {
        printf("⚠️ 未找到包含\"%s\"的患者档案！\n", name_keyword);
        return 0;
    }
    
    return 1;
}

// ==========================================
// 患者自助基础病历查询功能
// ==========================================

/**
 * @brief 患者自助查询基础病历信息（需要身份核验）
 * @param patient_id 患者编号
 * @param id_card 患者身份证号（用于身份核验）
 * @return 成功返回1，失败返回0
 */
int query_basic_patient_record(const char* patient_id, const char* id_card)
{
    char masked_id[19];
    
    // 获取患者节点（带错误检查）
    PatientNode* patient = get_patient_by_id_checked(patient_id, "查询基础病历");

    if (patient == NULL)
    {
        printf("⚠️ 未找到对应患者，病历查询失败！\n");
        return 0;
    }

    // 检查身份证号格式是否合法
    if (id_card == NULL || !validate_id_card(id_card))
    {
        printf("提示：身份证号格式不合法，无法进行身份核验！\n");
        return 0;
    }

    // 身份核验：检查输入的身份证号是否与患者档案中的身份证号匹配
    if (strcmp(patient->id_card, id_card) != 0)
    {
        printf("⚠️ 身份核验失败，禁止访问基础病历信息！\n");
        return 0;
    }

    // 身份证号脱敏处理
    mask_id_card(patient->id_card, masked_id);
    
    // 显示基础病历信息
    printf("\n================ 基础病历信息 ================\n");
    printf("患者编号：%s\n", patient->id);
    printf("姓名：%s\n", patient->name);
    printf("年龄：%d\n", patient->age);
    printf("症状描述: %s\n", get_display_text(patient->symptom));
    printf("目标科室: %s\n", get_display_text(patient->target_dept));
    printf("当前就诊状态: %s\n", get_patient_status_text(patient->status));
    printf("最近一次诊断结论: %s\n",
        strlen(patient->diagnosis_text) > 0 ? patient->diagnosis_text : "暂无诊断记录");
    printf("最近一次处理意见: %s\n",
        strlen(patient->treatment_advice) > 0 ? patient->treatment_advice : "暂无处理意见");
    printf("身份证号: %s\n", masked_id);

    return 1;
}

/**
 * @brief 更新患者档案信息
 * @param patient_id 患者编号（不可修改）
 * @param name 新姓名（留空表示不修改）
 * @param age 新年龄（0表示不修改）
 * @param symptom 新症状描述（留空表示不修改）
 * @param target_dept 新目标科室（留空表示不修改）
 * @param id_card 新身份证号（留空表示不修改）
 * @param balance 新账户余额
 * @return 成功返回1，失败返回0
 */
int update_patient_archive(const char* patient_id, const char* name, int age, const char* symptom, const char* target_dept, const char* id_card, double balance)
{
    // 获取患者节点（带错误检查）
    PatientNode* patient = get_patient_by_id_checked(patient_id, "修改患者档案");

    if (patient == NULL)
    {
        printf("⚠️ 未找到对应患者，更新失败！\n");
        return 0;
    }
    
    // 更新患者信息（只更新非空字段）
    if (name != NULL && strlen(name) > 0)
    {
        // 检查患者姓名长度是否合法
        int name_len = strlen(name);
        if (name_len < 2 || name_len > 20)
        {
            printf("⚠️ 患者姓名长度必须在 2-20 个字符之间！\n");
            return 0;
        }
        copy_text_field(patient->name, MAX_NAME_LEN, name);
    }
    
    if (age > 0)
    {
        // 检查患者年龄是否合法
        if (age < 0 || age > 130)
        {
            printf("⚠️ 患者年龄必须在 0-130 岁之间！\n");
            return 0;
        }
        patient->age = age;
    }
    
    if (symptom != NULL && symptom[0] != '\0')
    {
        copy_text_field(patient->symptom, MAX_SYMPTOM_LEN, symptom);
    }
    
    if (target_dept != NULL && target_dept[0] != '\0')
    {
        copy_text_field(patient->target_dept, MAX_NAME_LEN, target_dept);
    }
    
    // 更新身份证号（需要验证）
    if (id_card != NULL && strlen(id_card) > 0)
    {
        // 验证身份证号格式
        if (!validate_id_card(id_card))
        {
            printf("⚠️ 身份证号格式不合法，更新失败！\n");
            return 0;
        }
        
        // 检查新身份证号是否已被其他患者使用
        PatientNode* existing_patient = find_patient_by_id_card(id_card);
        if (existing_patient != NULL && strcmp(existing_patient->id, patient_id) != 0)
        {
            printf("⚠️ 该身份证号已被其他患者使用，更新失败！\n");
            return 0;
        }
        
        // 更新身份证号
        copy_text_field(patient->id_card, MAX_ID_LEN, id_card);
    }
    
    // 更新账户余额
    patient->balance = balance;
    
    printf("✅ 患者档案更新成功！\n");
    return 1;
}

// ==========================================
// 患者就诊概览功能
// ==========================================

/**
 * @brief 显示患者就诊概览信息
 * @param patient 患者节点指针
 */
void display_patient_visit_overview(const PatientNode* patient)
{
    // 参数校验
    if (patient == NULL)
    {
        printf("⚠️ 患者信息为空！\n");
        return;
    }
    
    // 显示患者基本信息
    printf("\n================ 患者就诊概览 ================\n");
    printf("患者编号：%s\n", patient->id);
    printf("姓名：%s\n", patient->name);
    printf("年龄：%d\n", patient->age);
    
    // 脱敏身份证号
    if (strlen(patient->id_card) > 0)
    {
        char masked_id[MAX_ID_LEN] = {0};
        mask_id_card(patient->id_card, masked_id);
        printf("身份证号：%s\n", masked_id);
    }
    else
    {
        printf("身份证号：暂无\n");
    }
    
    // 症状描述（为空时显示"暂无"）
    if (patient->symptom[0] != '\0')
        printf("症状描述: %s\n", patient->symptom);
    else
        printf("症状描述: 暂无\n");
    
    // 目标科室（为空时显示"暂无"）
    if (patient->target_dept[0] != '\0')
        printf("目标科室: %s\n", patient->target_dept);
    else
        printf("目标科室: 暂无\n");
    
    // 当前状态和账户余额
    printf("当前状态: %s\n", get_patient_status_text(patient->status));
    printf("账户余额：%.2f\n", patient->balance);
    
    // 最近一次诊断结论
    if (patient->diagnosis_text[0] != '\0')
        printf("最近诊断结论: %s\n", patient->diagnosis_text);
    else
        printf("最近诊断结论: 暂无诊断记录\n");
    
    // 最近一次处理意见
    if (patient->treatment_advice[0] != '\0')
        printf("处理意见: %s\n", patient->treatment_advice);
    else
        printf("处理意见: 暂无处理意见\n");
    
    // 最近一次预约信息
    AppointmentNode* latest_appointment = find_latest_appointment_by_patient_id(patient->id);
    if (latest_appointment != NULL)
    {
        printf("\n最近预约信息:\n");
        printf("  预约编号：%s\n", latest_appointment->appointment_id);
        printf("  预约日期：%s\n", latest_appointment->appointment_date);
        printf("  预约时段：%s\n", latest_appointment->appointment_slot);
        
        // 显示预约医生或科室
        if (latest_appointment->appoint_doctor[0] != '\0')
            printf("  预约医生：%s\n", latest_appointment->appoint_doctor);
        else if (latest_appointment->appoint_dept[0] != '\0')
            printf("  预约科室：%s\n", latest_appointment->appoint_dept);
        else
            printf("  预约科室：暂无\n");
        printf("  预约状态：%s\n", get_appointment_status_text(latest_appointment->appointment_status));
    }
    else
    {
        printf("\n最近预约信息: 暂无预约记录\n");
    }
    
    printf("==============================================\n");
}

/**
 * @brief 根据患者编号查询就诊概览
 * @param patient_id 患者编号
 * @return 成功返回1，失败返回0
 */
int query_patient_visit_overview_by_id(const char* patient_id)
{
    PatientNode* patient = NULL;
    
    // 检查患者链表是否初始化
    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化，无法查询！\n");
        return 0;
    }
    
    // 检查患者编号是否为空
    if (patient_id == NULL || strlen(patient_id) == 0)
    {
        printf("⚠️ 患者编号不能为空！\n");
        return 0;
    }
    
    // 根据患者编号查找患者
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到对应患者，查询失败！\n");
        return 0;
    }
    
    // 显示患者就诊概览信息
    display_patient_visit_overview(patient);
    return 1;
}

/**
 * @brief 根据身份证号查询就诊概览
 * @param id_card 患者身份证号
 * @return 成功返回1，失败返回0
 */
int query_patient_visit_overview_by_id_card(const char* id_card)
{
    PatientNode* patient = NULL;
    
    // 检查患者链表是否初始化
    if (g_patient_list == NULL)
    {
        printf("⚠️ 患者链表尚未初始化，无法查询！\n");
        return 0;
    }
    
    // 检查身份证号是否为空
    if (id_card == NULL || strlen(id_card) == 0)
    {
        printf("⚠️ 身份证号不能为空！\n");
        return 0;
    }
    
    // 检查身份证号格式是否合法
    if (!validate_id_card(id_card))
    {
        printf("提示：身份证号格式不合法，无法查询！\n");
        return 0;
    }
    
    // 根据身份证号查找患者
    patient = find_patient_by_id_card(id_card);
    if (patient == NULL)
    {
        printf("⚠️ 未找到对应患者，查询失败！\n");
        return 0;
    }
    
    // 显示患者就诊概览信息
    display_patient_visit_overview(patient);
    return 1;
}
