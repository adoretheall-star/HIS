#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h> // 用于 isnan, isinf 函数
#include "admin_service.h"
#include "list_ops.h"
#include "medicine_service.h"
#include "pharmacy_service.h"
#include "data_io.h"
#include "patient_service.h"
#include "utils.h"

// 当前登录用户
AccountNode* g_current_account = NULL;
DoctorNode* g_current_doctor = NULL;
PatientNode* g_current_patient = NULL;

// 外部函数声明
extern void query_patient_complaints(const char* patient_id);



// 前置声明
static void show_expiring_medicine_warning(void);
static int days_between_dates(const char* start_date, const char* end_date);

// 检查药品是否近效期
static int is_medicine_expiring_soon(const char* expiry_date)
{
    if (expiry_date == NULL)
    {
        return 0;
    }
    
    // 简单的近效期检查：假设当前日期为 2024-01-01，检查是否在 30 天内过期
    // 实际项目中应该使用当前系统日期进行比较
    const char* current_date = "2024-01-01";
    int days_diff = days_between_dates(current_date, expiry_date);
    
    return (days_diff >= 0 && days_diff <= 30);
}

// 近效期药品预警
static void show_expiring_medicine_warning(void)
{
    int count = 0;
    int total_width = 80;
    
    // 检查近效期药品
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        MedicineNode* curr = g_medicine_list->next;
        while (curr != NULL)
        {
            if (is_medicine_expiring_soon(curr->expiry_date))
            {
                count++;
            }
            curr = curr->next;
        }
    }
    
    // 输出统计信息
    printf("\n======================================================\n");
    printf("                  近效期药品预警\n");
    printf("======================================================\n");
    
    if (count > 0)
    {
        printf("⚠️ 发现 %d 种近效期药品，请及时处理！\n", count);
    }
    else
    {
        printf("✅ 没有发现近效期药品。\n");
    }
    
    printf("======================================================\n");
    printf("按任意键返回...\n");
    get_single_char("");
}

// 解析日期字符串为 tm 结构
static int parse_date_string(const char* date_str, struct tm* tm_out);

// 计算两个日期之间的天数差
static int days_between_dates(const char* start_date, const char* end_date);

// 计算字符串的显示宽度（中文算2个宽度）
static int get_display_width(const char* str)
{
    if (str == NULL) return 0;
    
    int width = 0;
    const unsigned char* p = (const unsigned char*)str;
    
    while (*p != '\0')
    {
        if (*p < 128)
        {
            // ASCII 字符，宽度 1
            width++;
        }
        else
        {
            // 非 ASCII 字符（如中文），宽度 2
            width += 2;
            // 跳过后续字节（UTF-8 编码）
            if (*p >= 0xE0) p += 2; // 3字节UTF-8
            else p += 1; // 2字节UTF-8
        }
        p++;
    }
    
    return width;
}

// 按指定宽度打印文本并补空格
static void print_padded_text(const char* str, int target_width)
{
    if (str == NULL) str = "";

    int current_width = get_display_width(str);
    printf("%s", str);

    // 计算需要补的空格数
    int spaces = target_width - current_width;
    if (spaces > 0)
    {
        for (int i = 0; i < spaces; i++)
        {
            printf(" ");
        }
    }
}

// 根据病房类型估算日费用
static double estimate_daily_cost_by_ward_type(WardType ward_type)
{
    switch (ward_type)
    {
        case WARD_TYPE_GENERAL:
            return 200.0;
        case WARD_TYPE_ICU:
            return 1500.0;
        default:
            return 200.0;
    }
}

// 查看所有员工账号
void show_all_accounts(void)
{
    AccountNode* curr = NULL;
    int count = 0;
    
    // 第一步：统计每一列的最大显示宽度
    int username_width = get_display_width("登录账号");
    int real_name_width = get_display_width("真实姓名");
    int role_width = get_display_width("角色");
    
    if (g_account_list != NULL && g_account_list->next != NULL)
    {
        curr = g_account_list->next;
        while (curr != NULL)
        {
            const char* role_name = NULL;
            switch (curr->role)
            {
                case ROLE_ADMIN:
                    role_name = "管理员";
                    break;
                case ROLE_NURSE:
                    role_name = "护士";
                    break;
                case ROLE_DOCTOR:
                    role_name = "医生";
                    break;
                case ROLE_PHARMACIST:
                    role_name = "药师";
                    break;
                default:
                    role_name = "未知";
                    break;
            }
            
            int current_username_width = get_display_width(curr->username);
            int current_real_name_width = get_display_width(curr->real_name);
            int current_role_width = get_display_width(role_name);
            
            if (current_username_width > username_width)
                username_width = current_username_width;
            if (current_real_name_width > real_name_width)
                real_name_width = current_real_name_width;
            if (current_role_width > role_width)
                role_width = current_role_width;
            
            curr = curr->next;
        }
    }
    
    // 第二步：按动态列宽输出表格
    if (g_account_list == NULL || g_account_list->next == NULL)
    {
        printf("当前暂无员工账号数据\n");
        return;
    }
    
    // 计算总宽度并输出边框
    int total_width = username_width + real_name_width + role_width + 8; // 8是列间距总和
    
    printf("\n");
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    // 输出标题
    int title_width = get_display_width("员工列表");
    int title_padding = (total_width - title_width) / 2;
    for (int i = 0; i < title_padding; i++) printf(" ");
    printf("员工列表");
    for (int i = 0; i < total_width - title_width - title_padding; i++) printf(" ");
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    // 输出表头
    print_padded_text("登录账号", username_width);
    printf("    "); // 4个空格的列间距
    print_padded_text("真实姓名", real_name_width);
    printf("    "); // 4个空格的列间距
    print_padded_text("角色", role_width);
    printf("\n");
    
    // 输出表头下横线
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");
    
    // 输出数据行
    curr = g_account_list->next;
    while (curr != NULL)
    {
        const char* role_name = NULL;
        switch (curr->role)
        {
            case ROLE_ADMIN:
                role_name = "管理员";
                break;
            case ROLE_NURSE:
                role_name = "护士";
                break;
            case ROLE_DOCTOR:
                role_name = "医生";
                break;
            case ROLE_PHARMACIST:
                role_name = "药师";
                break;
            default:
                role_name = "未知";
                break;
        }
        
        print_padded_text(curr->username, username_width);
        printf("    "); // 4个空格的列间距
        print_padded_text(curr->real_name, real_name_width);
        printf("    "); // 4个空格的列间距
        print_padded_text(role_name, role_width);
        printf("\n");
        
        count++;
        curr = curr->next;
    }
    
    // 输出底部横线
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");
    
    // 输出员工总数
    char count_str[50];
    snprintf(count_str, sizeof(count_str), "员工总数：%d", count);
    int count_width = get_display_width(count_str);
    int count_padding = (total_width - count_width) / 2;
    for (int i = 0; i < count_padding; i++) printf(" ");
    printf("%s", count_str);
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
}

// 注册新员工账号
int register_account(const char* username, const char* password, const char* real_name, const char* gender, RoleType role)
{
    if (username == NULL || password == NULL || real_name == NULL)
    {
        return 0;
    }

    // 检查账号是否已存在
    if (find_account_by_username(g_account_list, username) != NULL)
    {
        return 0;
    }

    // 创建新账号节点
    AccountNode* new_node = (AccountNode*)malloc(sizeof(AccountNode));
    if (new_node == NULL)
    {
        return 0;
    }

    strncpy(new_node->username, username, sizeof(new_node->username) - 1);
    new_node->username[sizeof(new_node->username) - 1] = '\0';
    strncpy(new_node->password, password, sizeof(new_node->password) - 1);
    new_node->password[sizeof(new_node->password) - 1] = '\0';
    strncpy(new_node->real_name, real_name, sizeof(new_node->real_name) - 1);
    new_node->real_name[sizeof(new_node->real_name) - 1] = '\0';
    new_node->role = role;
    new_node->next = NULL;

    // 添加到链表
    if (g_account_list == NULL)
    {
        g_account_list = (AccountNode*)malloc(sizeof(AccountNode));
        if (g_account_list == NULL)
        {
            free(new_node);
            return 0;
        }
        g_account_list->next = new_node;
    }
    else
    {
        AccountNode* curr = g_account_list;
        while (curr->next != NULL)
        {
            curr = curr->next;
        }
        curr->next = new_node;
    }

    return 1;
}

// 验证账号
int verify_account(const char* username, const char* password, int* role)
{
    AccountNode* account = find_account_by_username(g_account_list, username);
    if (account == NULL)
    {
        return 0;
    }

    if (strcmp(account->password, password) != 0)
    {
        return 0;
    }

    if (role != NULL)
    {
        *role = account->role;
    }

    return 1;
}

// 修改员工资料
int update_account_basic_info(const char* username, const char* new_real_name, const char* new_password, RoleType new_role)
{
    AccountNode* account = find_account_by_username(g_account_list, username);
    if (account == NULL)
    {
        return 0;
    }

    if (new_real_name != NULL)
    {
        strncpy(account->real_name, new_real_name, sizeof(account->real_name) - 1);
        account->real_name[sizeof(account->real_name) - 1] = '\0';
    }

    if (new_password != NULL)
    {
        strncpy(account->password, new_password, sizeof(account->password) - 1);
        account->password[sizeof(account->password) - 1] = '\0';
    }

    account->role = new_role;

    return 1;
}

// 删除员工账号
int delete_account(const char* username)
{
    if (username == NULL || g_account_list == NULL || g_account_list->next == NULL)
    {
        return 0;
    }

    AccountNode* prev = g_account_list;
    AccountNode* curr = g_account_list->next;

    while (curr != NULL)
    {
        if (strcmp(curr->username, username) == 0)
        {
            prev->next = curr->next;
            free(curr);
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }

    return 0;
}

// 查看所有医生及其值班状态
void show_all_doctors_with_duty_status(void)
{
    AccountNode* curr = NULL;
    int count = 0;

    if (g_account_list == NULL || g_account_list->next == NULL)
    {
        printf("当前暂无医生数据\n");
        return;
    }

    // 第一步：统计每一列的最大显示宽度
    int username_width = get_display_width("登录账号");
    int real_name_width = get_display_width("真实姓名");
    int duty_width = get_display_width("值班状态");

    curr = g_account_list->next;
    while (curr != NULL)
    {
        if (curr->role == ROLE_DOCTOR)
        {
            const char* duty_status = curr->is_on_duty ? "在岗" : "休息";

            int current_username_width = get_display_width(curr->username);
            int current_real_name_width = get_display_width(curr->real_name);
            int current_duty_width = get_display_width(duty_status);

            if (current_username_width > username_width)
                username_width = current_username_width;
            if (current_real_name_width > real_name_width)
                real_name_width = current_real_name_width;
            if (current_duty_width > duty_width)
                duty_width = current_duty_width;

            count++;
        }
        curr = curr->next;
    }

    // 第二步：按动态列宽输出表格
    int total_width = username_width + real_name_width + duty_width + 8;

    printf("\n");
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");

    // 输出标题
    int title_width = get_display_width("医生值班状态");
    int title_padding = (total_width - title_width) / 2;
    for (int i = 0; i < title_padding; i++) printf(" ");
    printf("医生值班状态");
    for (int i = 0; i < total_width - title_width - title_padding; i++) printf(" ");
    printf("\n");

    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");

    // 输出表头
    print_padded_text("登录账号", username_width);
    printf("    ");
    print_padded_text("真实姓名", real_name_width);
    printf("    ");
    print_padded_text("值班状态", duty_width);
    printf("\n");

    // 输出表头下横线
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");

    // 输出数据行
    curr = g_account_list->next;
    while (curr != NULL)
    {
        if (curr->role == ROLE_DOCTOR)
        {
            const char* duty_status = curr->is_on_duty ? "在岗" : "休息";

            print_padded_text(curr->username, username_width);
            printf("    ");
            print_padded_text(curr->real_name, real_name_width);
            printf("    ");
            print_padded_text(duty_status, duty_width);
            printf("\n");
        }
        curr = curr->next;
    }

    // 输出底部横线
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");

    // 输出统计信息
    char count_str[50];
    snprintf(count_str, sizeof(count_str), "医生总数：%d", count);
    int count_width = get_display_width(count_str);
    int count_padding = (total_width - count_width) / 2;
    for (int i = 0; i < count_padding; i++) printf(" ");
    printf("%s", count_str);
    printf("\n");

    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
}

// 查看所有护士及其值班状态
void show_all_nurses_with_duty_status(void)
{
    AccountNode* curr = NULL;
    int count = 0;

    if (g_account_list == NULL || g_account_list->next == NULL)
    {
        printf("当前暂无护士数据\n");
        return;
    }

    // 第一步：统计每一列的最大显示宽度
    int username_width = get_display_width("登录账号");
    int real_name_width = get_display_width("真实姓名");
    int duty_width = get_display_width("值班状态");

    curr = g_account_list->next;
    while (curr != NULL)
    {
        if (curr->role == ROLE_NURSE)
        {
            const char* duty_status = curr->is_on_duty ? "在岗" : "休息";

            int current_username_width = get_display_width(curr->username);
            int current_real_name_width = get_display_width(curr->real_name);
            int current_duty_width = get_display_width(duty_status);

            if (current_username_width > username_width)
                username_width = current_username_width;
            if (current_real_name_width > real_name_width)
                real_name_width = current_real_name_width;
            if (current_duty_width > duty_width)
                duty_width = current_duty_width;

            count++;
        }
        curr = curr->next;
    }

    // 第二步：按动态列宽输出表格
    int total_width = username_width + real_name_width + duty_width + 8;

    printf("\n");
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");

    // 输出标题
    int title_width = get_display_width("护士值班状态");
    int title_padding = (total_width - title_width) / 2;
    for (int i = 0; i < title_padding; i++) printf(" ");
    printf("护士值班状态");
    for (int i = 0; i < total_width - title_width - title_padding; i++) printf(" ");
    printf("\n");

    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");

    // 输出表头
    print_padded_text("登录账号", username_width);
    printf("    ");
    print_padded_text("真实姓名", real_name_width);
    printf("    ");
    print_padded_text("值班状态", duty_width);
    printf("\n");

    // 输出表头下横线
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");

    // 输出数据行
    curr = g_account_list->next;
    while (curr != NULL)
    {
        if (curr->role == ROLE_NURSE)
        {
            const char* duty_status = curr->is_on_duty ? "在岗" : "休息";

            print_padded_text(curr->username, username_width);
            printf("    ");
            print_padded_text(curr->real_name, real_name_width);
            printf("    ");
            print_padded_text(duty_status, duty_width);
            printf("\n");
        }
        curr = curr->next;
    }

    // 输出底部横线
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");

    // 输出统计信息
    char count_str[50];
    snprintf(count_str, sizeof(count_str), "护士总数：%d", count);
    int count_width = get_display_width(count_str);
    int count_padding = (total_width - count_width) / 2;
    for (int i = 0; i < count_padding; i++) printf(" ");
    printf("%s", count_str);
    printf("\n");

    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
}

// 查看所有药师及其值班状态
void show_all_pharmacists_with_duty_status(void)
{
    AccountNode* curr = NULL;
    int count = 0;

    if (g_account_list == NULL || g_account_list->next == NULL)
    {
        printf("当前暂无药师数据\n");
        return;
    }

    // 第一步：统计每一列的最大显示宽度
    int username_width = get_display_width("登录账号");
    int real_name_width = get_display_width("真实姓名");
    int duty_width = get_display_width("值班状态");

    curr = g_account_list->next;
    while (curr != NULL)
    {
        if (curr->role == ROLE_PHARMACIST)
        {
            const char* duty_status = curr->is_on_duty ? "在岗" : "休息";

            int current_username_width = get_display_width(curr->username);
            int current_real_name_width = get_display_width(curr->real_name);
            int current_duty_width = get_display_width(duty_status);

            if (current_username_width > username_width)
                username_width = current_username_width;
            if (current_real_name_width > real_name_width)
                real_name_width = current_real_name_width;
            if (current_duty_width > duty_width)
                duty_width = current_duty_width;

            count++;
        }
        curr = curr->next;
    }

    // 第二步：按动态列宽输出表格
    int total_width = username_width + real_name_width + duty_width + 8;

    printf("\n");
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");

    // 输出标题
    int title_width = get_display_width("药师值班状态");
    int title_padding = (total_width - title_width) / 2;
    for (int i = 0; i < title_padding; i++) printf(" ");
    printf("药师值班状态");
    for (int i = 0; i < total_width - title_width - title_padding; i++) printf(" ");
    printf("\n");

    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");

    // 输出表头
    print_padded_text("登录账号", username_width);
    printf("    ");
    print_padded_text("真实姓名", real_name_width);
    printf("    ");
    print_padded_text("值班状态", duty_width);
    printf("\n");

    // 输出表头下横线
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");

    // 输出数据行
    curr = g_account_list->next;
    while (curr != NULL)
    {
        if (curr->role == ROLE_PHARMACIST)
        {
            const char* duty_status = curr->is_on_duty ? "在岗" : "休息";

            print_padded_text(curr->username, username_width);
            printf("    ");
            print_padded_text(curr->real_name, real_name_width);
            printf("    ");
            print_padded_text(duty_status, duty_width);
            printf("\n");
        }
        curr = curr->next;
    }

    // 输出底部横线
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");

    // 输出统计信息
    char count_str[50];
    snprintf(count_str, sizeof(count_str), "药师总数：%d", count);
    int count_width = get_display_width(count_str);
    int count_padding = (total_width - count_width) / 2;
    for (int i = 0; i < count_padding; i++) printf(" ");
    printf("%s", count_str);
    printf("\n");

    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
}

// 更新医生值班状态
int update_doctor_duty_status(const char* username, int is_on_duty)
{
    AccountNode* account = find_account_by_username(g_account_list, username);
    if (account == NULL || account->role != ROLE_DOCTOR)
    {
        return 0;
    }

    account->is_on_duty = is_on_duty;
    return 1;
}

// 更新护士值班状态
int update_nurse_duty_status(const char* username, int is_on_duty)
{
    AccountNode* account = find_account_by_username(g_account_list, username);
    if (account == NULL || account->role != ROLE_NURSE)
    {
        return 0;
    }

    account->is_on_duty = is_on_duty;
    return 1;
}

// 更新药师值班状态
int update_pharmacist_duty_status(const char* username, int is_on_duty)
{
    AccountNode* account = find_account_by_username(g_account_list, username);
    if (account == NULL || account->role != ROLE_PHARMACIST)
    {
        return 0;
    }

    account->is_on_duty = is_on_duty;
    return 1;
}

// 显示管理员操作面板
void show_admin_dashboard(void)
{
    int patient_count = 0;
    int medicine_count = 0;
    int waiting_dispense_count = 0;
    int low_stock_count = 0;
    int expiring_medicine_count = 0;
    int doctor_count = 0;
    int nurse_count = 0;
    int pharmacist_count = 0;

    // 统计患者数量和待发药患者数量
    if (g_patient_list != NULL && g_patient_list->next != NULL)
    {
        PatientNode* curr = g_patient_list->next;
        while (curr != NULL)
        {
            patient_count++;
            // STATUS_WAIT_MED = 5 表示"已缴费待取药"
            if (curr->status == STATUS_WAIT_MED)
            {
                waiting_dispense_count++;
            }
            curr = curr->next;
        }
    }

    // 统计药品数量、低库存数量和近效期药品数量
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        MedicineNode* curr = g_medicine_list->next;
        while (curr != NULL)
        {
            medicine_count++;
            if (curr->stock < 10)
            {
                low_stock_count++;
            }
            
            // 检查是否为近效期药品（30天内过期）
            if (strlen(curr->expiry_date) > 0)
            {
                time_t now = time(NULL);
                struct tm now_tm;
                localtime_s(&now_tm, &now);
                
                char current_date[11];
                snprintf(current_date, sizeof(current_date), "%d-%02d-%02d", 
                         now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday);
                
                int days_left = days_between_dates(current_date, curr->expiry_date);
                if (days_left > 0 && days_left <= 30)
                {
                    expiring_medicine_count++;
                }
            }
            
            curr = curr->next;
        }
    }

    // 统计医生、护士和药师数量
    if (g_account_list != NULL && g_account_list->next != NULL)
    {
        AccountNode* curr = g_account_list->next;
        while (curr != NULL)
        {
            switch (curr->role)
            {
                case ROLE_DOCTOR:
                    doctor_count++;
                    break;
                case ROLE_NURSE:
                    nurse_count++;
                    break;
                case ROLE_PHARMACIST:
                    pharmacist_count++;
                    break;
                default:
                    break;
            }
            curr = curr->next;
        }
    }

    printf("\n==============================================================\n");
    printf("                      管理统计面板\n");
    printf("==============================================================\n");
    printf("患者总数：%d\n", patient_count);
    printf("当前待发药患者数量：%d\n", waiting_dispense_count);
    printf("药品总数：%d\n", medicine_count);
    printf("低库存药品数量：%d\n", low_stock_count);
    printf("近效期药品数量：%d\n", expiring_medicine_count);
    printf("医生总数：%d\n", doctor_count);
    printf("护士总数：%d\n", nurse_count);
    printf("药师总数：%d\n", pharmacist_count);
    printf("==============================================================\n");
}

// 解析日期字符串为 tm 结构
static int parse_date_string(const char* date_str, struct tm* tm_out)
{
    if (date_str == NULL || tm_out == NULL)
    {
        return 0;
    }

    int year, month, day;
    if (sscanf(date_str, "%d-%d-%d", &year, &month, &day) != 3)
    {
        return 0;
    }

    if (month < 1 || month > 12 || day < 1 || day > 31)
    {
        return 0;
    }

    memset(tm_out, 0, sizeof(struct tm));
    tm_out->tm_year = year - 1900;
    tm_out->tm_mon = month - 1;
    tm_out->tm_mday = day;
    tm_out->tm_hour = 12; // 避免夏令时问题

    return 1;
}

// 计算两个日期之间的天数差
static int days_between_dates(const char* start_date, const char* end_date)
{
    struct tm start_tm;
    struct tm end_tm;
    time_t start_time;
    time_t end_time;
    double seconds;

    if (!parse_date_string(start_date, &start_tm) ||
        !parse_date_string(end_date, &end_tm))
    {
        return 999999;
    }

    start_time = mktime(&start_tm);
    end_time = mktime(&end_tm);
    if (start_time == (time_t)-1 || end_time == (time_t)-1)
    {
        return 999999;
    }

    seconds = difftime(end_time, start_time);
    return (int)(seconds / (60 * 60 * 24));
}

// 低库存药品预警
static void show_low_stock_warning(void)
{
    int count = 0;
    
    // 第一步：统计每一列的最大显示宽度
    int id_width = get_display_width("药品编号");
    int name_width = get_display_width("商品名");
    int generic_name_width = get_display_width("通用名");
    int alias_width = get_display_width("别名");
    int stock_width = get_display_width("当前库存");
    int price_width = get_display_width("单价");
    int medicare_width = get_display_width("医保类型");
    int expiry_width = get_display_width("效期");
    
    // 统计低库存药品并计算列宽
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        MedicineNode* curr = g_medicine_list->next;
        while (curr != NULL)
        {
            if (curr->stock < 10)
            {
                count++;
                
                const char* medicare_name = NULL;
                switch (curr->m_type)
                {
                    case MEDICARE_NONE:
                        medicare_name = "无医保";
                        break;
                    case MEDICARE_CLASS_A:
                        medicare_name = "甲类医保";
                        break;
                    case MEDICARE_CLASS_B:
                        medicare_name = "乙类医保";
                        break;
                    default:
                        medicare_name = "未知";
                        break;
                }
                
                int current_id_width = get_display_width(curr->id);
                int current_name_width = get_display_width(curr->name);
                int current_generic_name_width = get_display_width(curr->generic_name);
                int current_alias_width = get_display_width(curr->alias);
                int current_medicare_width = get_display_width(medicare_name);
                int current_expiry_width = get_display_width(curr->expiry_date);
                
                // 处理库存和单价的宽度
                char stock_str[20];
                char price_str[20];
                snprintf(stock_str, sizeof(stock_str), "%d", curr->stock);
                snprintf(price_str, sizeof(price_str), "%.2f", curr->price);
                int current_stock_width = get_display_width(stock_str);
                int current_price_width = get_display_width(price_str);
                
                if (current_id_width > id_width)
                    id_width = current_id_width;
                if (current_name_width > name_width)
                    name_width = current_name_width;
                if (current_generic_name_width > generic_name_width)
                    generic_name_width = current_generic_name_width;
                if (current_alias_width > alias_width)
                    alias_width = current_alias_width;
                if (current_stock_width > stock_width)
                    stock_width = current_stock_width;
                if (current_price_width > price_width)
                    price_width = current_price_width;
                if (current_medicare_width > medicare_width)
                    medicare_width = current_medicare_width;
                if (current_expiry_width > expiry_width)
                    expiry_width = current_expiry_width;
            }
            curr = curr->next;
        }
    }
    
    // 第二步：按动态列宽输出表格
    printf("\n");
    int total_width = id_width + name_width + generic_name_width + alias_width + stock_width + price_width + medicare_width + expiry_width + 28; // 28是列间距总和
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    // 输出标题
    int title_width = get_display_width("低库存药品预警");
    int title_padding = (total_width - title_width) / 2;
    for (int i = 0; i < title_padding; i++) printf(" ");
    printf("低库存药品预警");
    for (int i = 0; i < total_width - title_width - title_padding; i++) printf(" ");
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    if (count == 0)
    {
        printf("当前无低库存药品预警\n");
    }
    else
    {
        // 输出表头
        print_padded_text("药品编号", id_width);
        printf("    ");
        print_padded_text("商品名", name_width);
        printf("    ");
        print_padded_text("通用名", generic_name_width);
        printf("    ");
        print_padded_text("别名", alias_width);
        printf("    ");
        print_padded_text("当前库存", stock_width);
        printf("    ");
        print_padded_text("单价", price_width);
        printf("    ");
        print_padded_text("医保类型", medicare_width);
        printf("    ");
        print_padded_text("效期", expiry_width);
        printf("\n");
        
        // 输出表头下横线
        for (int i = 0; i < total_width; i++) printf("-");
        printf("\n");
        
        // 输出数据行
        if (g_medicine_list != NULL && g_medicine_list->next != NULL)
        {
            MedicineNode* curr = g_medicine_list->next;
            while (curr != NULL)
            {
                if (curr->stock < 10)
                {
                    const char* medicare_name = NULL;
                    switch (curr->m_type)
                    {
                        case MEDICARE_NONE:
                            medicare_name = "无医保";
                            break;
                        case MEDICARE_CLASS_A:
                            medicare_name = "甲类医保";
                            break;
                        case MEDICARE_CLASS_B:
                            medicare_name = "乙类医保";
                            break;
                        default:
                            medicare_name = "未知";
                            break;
                    }
                    
                    print_padded_text(curr->id, id_width);
                    printf("    ");
                    print_padded_text(curr->name, name_width);
                    printf("    ");
                    print_padded_text(curr->generic_name, generic_name_width);
                    printf("    ");
                    print_padded_text(curr->alias, alias_width);
                    printf("    ");
                    char stock_str[20];
                    char price_str[20];
                    snprintf(stock_str, sizeof(stock_str), "%d", curr->stock);
                    snprintf(price_str, sizeof(price_str), "%.2f", curr->price);
                    print_padded_text(stock_str, stock_width);
                    printf("    ");
                    print_padded_text(price_str, price_width);
                    printf("    ");
                    print_padded_text(medicare_name, medicare_width);
                    printf("    ");
                    print_padded_text(curr->expiry_date, expiry_width);
                    printf("\n");
                }
                curr = curr->next;
            }
        }
    }
    
    // 输出底部横线
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");
    
    // 输出统计信息
    char count_str[50];
    snprintf(count_str, sizeof(count_str), "低库存药品数量：%d", count);
    int count_width = get_display_width(count_str);
    int count_padding = (total_width - count_width) / 2;
    for (int i = 0; i < count_padding; i++) printf(" ");
    printf("%s", count_str);
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    printf("按任意键返回...\n");
    get_single_char("");
}

// 显示负载监控查看
void show_load_monitoring(void)
{
    int count = 0;
    
    // 第一步：统计每一列的最大显示宽度
    int id_width = get_display_width("药品编号");
    int name_width = get_display_width("商品名");
    int generic_name_width = get_display_width("通用名");
    int stock_width = get_display_width("当前库存");
    int price_width = get_display_width("单价");
    int expiry_width = get_display_width("效期");
    
    // 获取当前日期
    time_t now = time(NULL);
    struct tm now_tm;
    localtime_s(&now_tm, &now);
    char current_date[11];
    snprintf(current_date, sizeof(current_date), "%d-%02d-%02d", 
             now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday);
    
    // 统计近效期药品并计算列宽
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        MedicineNode* curr = g_medicine_list->next;
        while (curr != NULL)
        {
            if (strlen(curr->expiry_date) > 0)
            {
                int days_left = days_between_dates(current_date, curr->expiry_date);
                if (days_left > 0 && days_left <= 30)
                {
                    count++;
                    
                    int current_id_width = get_display_width(curr->id);
                    int current_name_width = get_display_width(curr->name);
                    int current_generic_name_width = get_display_width(curr->generic_name);
                    int current_expiry_width = get_display_width(curr->expiry_date);
                    
                    // 处理库存和单价的宽度
                    char stock_str[20];
                    char price_str[20];
                    snprintf(stock_str, sizeof(stock_str), "%d", curr->stock);
                    snprintf(price_str, sizeof(price_str), "%.2f", curr->price);
                    int current_stock_width = get_display_width(stock_str);
                    int current_price_width = get_display_width(price_str);
                    
                    if (current_id_width > id_width)
                        id_width = current_id_width;
                    if (current_name_width > name_width)
                        name_width = current_name_width;
                    if (current_generic_name_width > generic_name_width)
                        generic_name_width = current_generic_name_width;
                    if (current_stock_width > stock_width)
                        stock_width = current_stock_width;
                    if (current_price_width > price_width)
                        price_width = current_price_width;
                    if (current_expiry_width > expiry_width)
                        expiry_width = current_expiry_width;
                }
            }
            curr = curr->next;
        }
    }
    
    // 第二步：按动态列宽输出表格
    printf("\n");
    int total_width = id_width + name_width + generic_name_width + stock_width + price_width + expiry_width + 20; // 20是列间距总和
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    // 输出标题
    int title_width = get_display_width("近效期药品预警");
    int title_padding = (total_width - title_width) / 2;
    for (int i = 0; i < title_padding; i++) printf(" ");
    printf("近效期药品预警");
    for (int i = 0; i < total_width - title_width - title_padding; i++) printf(" ");
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    if (count == 0)
    {
        printf("当前无近效期药品预警\n");
    }
    else
    {
        // 输出表头
        print_padded_text("药品编号", id_width);
        printf("    ");
        print_padded_text("商品名", name_width);
        printf("    ");
        print_padded_text("通用名", generic_name_width);
        printf("    ");
        print_padded_text("当前库存", stock_width);
        printf("    ");
        print_padded_text("单价", price_width);
        printf("    ");
        print_padded_text("效期", expiry_width);
        printf("\n");
        
        // 输出表头下横线
        for (int i = 0; i < total_width; i++) printf("-");
        printf("\n");
        
        // 输出数据行
        if (g_medicine_list != NULL && g_medicine_list->next != NULL)
        {
            MedicineNode* curr = g_medicine_list->next;
            while (curr != NULL)
            {
                if (strlen(curr->expiry_date) > 0)
                {
                    int days_left = days_between_dates(current_date, curr->expiry_date);
                    if (days_left > 0 && days_left <= 30)
                    {
                        print_padded_text(curr->id, id_width);
                        printf("    ");
                        print_padded_text(curr->name, name_width);
                        printf("    ");
                        print_padded_text(curr->generic_name, generic_name_width);
                        printf("    ");
                        char stock_str[20];
                        char price_str[20];
                        snprintf(stock_str, sizeof(stock_str), "%d", curr->stock);
                        snprintf(price_str, sizeof(price_str), "%.2f", curr->price);
                        print_padded_text(stock_str, stock_width);
                        printf("    ");
                        print_padded_text(price_str, price_width);
                        printf("    ");
                        print_padded_text(curr->expiry_date, expiry_width);
                        printf("\n");
                    }
                }
                curr = curr->next;
            }
        }
    }
    
    // 输出底部横线
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");
    
    // 输出统计信息
    char count_str[50];
    snprintf(count_str, sizeof(count_str), "近效期药品数量：%d", count);
    int count_width = get_display_width(count_str);
    int count_padding = (total_width - count_width) / 2;
    for (int i = 0; i < count_padding; i++) printf(" ");
    printf("%s", count_str);
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    printf("按任意键返回...\n");
    get_single_char("");
}

// 待发药患者预警
static void show_waiting_dispense_warning(void)
{
    int count = 0;
    int total_width = 0;
    
    // 第一步：统计每一列的最大显示宽度
    int id_width = get_display_width("患者编号");
    int name_width = get_display_width("姓名");
    int age_width = get_display_width("年龄");
    int status_width = get_display_width("当前状态");
    int dept_width = get_display_width("就诊科室");
    
    // 统计待发药患者并计算列宽
    if (g_patient_list != NULL && g_patient_list->next != NULL)
    {
        PatientNode* curr = g_patient_list->next;
        while (curr != NULL)
        {
            if (curr->status == STATUS_WAIT_MED)
            {
                count++;
                
                const char* status_name = NULL;
                switch (curr->status)
                {
                    case STATUS_WAIT_MED:
                        status_name = "已缴费待取药";
                        break;
                    default:
                        status_name = "未知";
                        break;
                }
                
                int current_id_width = get_display_width(curr->id);
                int current_name_width = get_display_width(curr->name);
                int current_status_width = get_display_width(status_name);
                int current_dept_width = get_display_width(curr->target_dept);
                
                if (current_id_width > id_width)
                    id_width = current_id_width;
                if (current_name_width > name_width)
                    name_width = current_name_width;
                if (current_status_width > status_width)
                    status_width = current_status_width;
                if (current_dept_width > dept_width)
                    dept_width = current_dept_width;
            }
            curr = curr->next;
        }
    }

    // 计算总宽度
    total_width = id_width + name_width + age_width + status_width + dept_width + 12; // 12 是中间的空格

    // 输出表头
    printf("\n======================================================\n");
    printf("                  待取药患者提醒\n");
    printf("======================================================\n");
    print_padded_text("患者编号", id_width);
    printf("    ");
    print_padded_text("姓名", name_width);
    printf("    ");
    print_padded_text("年龄", age_width);
    printf("    ");
    print_padded_text("当前状态", status_width);
    printf("    ");
    print_padded_text("就诊科室", dept_width);
    printf("\n");
    
    // 输出表头下横线
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");
    
    // 输出数据行
    if (g_patient_list != NULL && g_patient_list->next != NULL)
    {
        PatientNode* curr = g_patient_list->next;
        while (curr != NULL)
        {
            if (curr->status == STATUS_WAIT_MED)
            {
                const char* status_name = NULL;
                switch (curr->status)
                {
                    case STATUS_WAIT_MED:
                        status_name = "已缴费待取药";
                        break;
                    default:
                        status_name = "未知";
                        break;
                }
                
                print_padded_text(curr->id, id_width);
                printf("    ");
                print_padded_text(curr->name, name_width);
                printf("    ");
                printf("%d", curr->age);
                printf("    ");
                print_padded_text(status_name, status_width);
                printf("    ");
                print_padded_text(curr->target_dept, dept_width);
                printf("\n");
            }
            curr = curr->next;
        }
    }
    
    // 输出底部横线
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");
    
    // 输出统计信息
    char count_str[50];
    snprintf(count_str, sizeof(count_str), "待取药患者数量：%d", count);
    int count_width = get_display_width(count_str);
    int count_padding = (total_width - count_width) / 2;
    for (int i = 0; i < count_padding; i++) printf(" ");
    printf("%s", count_str);
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    printf("按任意键返回...\n");
    get_single_char("");
}

// 函数声明
static void show_waiting_dispense_warning(void);
static void show_bed_occupancy_warning(void);
static void show_deposit_warning(void);
static void show_arrears_warning(void);

// 显示资源预警
void show_resource_warnings(void)
{
    int choice;
    
    do
    {
        int low_stock_count = 0;
        int expiring_medicine_count = 0;
        int deposit_warning_count = 0;
        int waiting_dispense_count = 0;
        int occupied_beds = 0;
        int free_beds = 0;
        int unpaid_patient_count = 0;

        // 统计低库存药品和近效期药品数量
        if (g_medicine_list != NULL && g_medicine_list->next != NULL)
        {
            MedicineNode* curr = g_medicine_list->next;
            
            time_t now = time(NULL);
            struct tm now_tm;
            localtime_s(&now_tm, &now);
            
            char current_date[11];
            snprintf(current_date, sizeof(current_date), "%d-%02d-%02d", 
                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday);
            
            while (curr != NULL)
            {
                if (curr->stock < 10)
                {
                    low_stock_count++;
                }
                
                // 检查是否为近效期药品（30天内过期）
                if (strlen(curr->expiry_date) > 0)
                {
                    int days_left = days_between_dates(current_date, curr->expiry_date);
                    if (days_left > 0 && days_left <= 30)
                    {
                        expiring_medicine_count++;
                    }
                }
                
                curr = curr->next;
            }
        }

        // 统计押金预警住院患者数量
        if (g_inpatient_list != NULL && g_inpatient_list->next != NULL)
        {
            InpatientRecord* curr = g_inpatient_list->next;
            while (curr != NULL)
            {
                // 仅统计当前仍在住院中的患者
                if (curr->is_active)
                {
                    occupied_beds++;
                    // 根据病房类型估算日费用，若押金不足3天费用则预警
                    double daily_cost = estimate_daily_cost_by_ward_type(curr->ward_type);
                    double threshold = daily_cost * 3; // 押金不足3天费用时预警
                    if (curr->deposit_balance < threshold)
                    {
                        deposit_warning_count++;
                    }
                }
                curr = curr->next;
            }
        }

        // 统计待发药患者数量
        if (g_patient_list != NULL && g_patient_list->next != NULL)
        {
            PatientNode* curr = g_patient_list->next;
            while (curr != NULL)
            {
                if (curr->status == STATUS_UNPAID)
                {
                    waiting_dispense_count++;
                }
                curr = curr->next;
            }
        }

        // 统计欠费/待缴费患者数量
        if (g_patient_list != NULL && g_patient_list->next != NULL)
        {
            PatientNode* curr = g_patient_list->next;
            while (curr != NULL)
            {
                if (curr->status == STATUS_UNPAID)
                {
                    unpaid_patient_count++;
                }
                curr = curr->next;
            }
        }

        // 计算空闲床位数（假设有100个床位）
        free_beds = 100 - occupied_beds;
        if (free_beds < 0) free_beds = 0;

        printf("\n==============================================================\n");
        printf("                      资源预警\n");
        printf("==============================================================\n");
        printf("低库存药品数量：%d\n", low_stock_count);
        printf("近效期药品数量：%d\n", expiring_medicine_count);
        printf("待发药患者数量：%d\n", waiting_dispense_count);
        printf("当前已占用床位数：%d\n", occupied_beds);
        printf("当前空闲床位数：%d\n", free_beds);
        printf("押金预警住院患者数量：%d\n", deposit_warning_count);
        printf("欠费 / 待缴费患者数量：%d\n", unpaid_patient_count);
        printf("==============================================================\n");
        printf("请选择要查看的预警详情\n");
        printf("==============================================================\n");
        printf("[1] 低库存药品预警\n");
        printf("[2] 近效期药品预警\n");
        printf("[3] 待发药患者预警\n");
        printf("[4] 床位占用预警\n");
        printf("[5] 住院押金预警\n");
        printf("[6] 欠费 / 待缴费预警\n");
        printf("[0] 返回上一级\n");
        printf("==============================================================\n");
        printf("请输入选择：");
        
        choice = get_safe_int("");
        
        switch (choice)
        {
            case 1:
                show_low_stock_warning();
                break;
            case 2:
                show_expiring_medicine_warning();
                break;
            case 3:
                show_waiting_dispense_warning();
                break;
            case 4:
                show_bed_occupancy_warning();
                break;
            case 5:
                show_deposit_warning();
                break;
            case 6:
                show_arrears_warning();
                break;
            case 0:
                // 返回上一级
                break;
            default:
                printf("输入错误，请重新选择！\n");
                printf("按任意键继续...\n");
                get_single_char("");
                break;
        }
    } while (choice != 0);
}





// 床位占用预警
static void show_bed_occupancy_warning(void)
{
    system("cls");
    printf("\n======================================================\n");
    printf("                床位占用预警\n");
    printf("======================================================\n");

    int occupied_count = 0;
    int free_count = 0;

    if (g_inpatient_list != NULL && g_inpatient_list->next != NULL)
    {
        InpatientRecord* curr = g_inpatient_list->next;
        
        while (curr != NULL)
        {
            if (curr->is_active)
            {
                occupied_count++;
                printf("患者编号：%s\n", curr->patient_id);
                
                // 查找患者姓名
                PatientNode* patient = find_patient_by_id(g_patient_list, curr->patient_id);
                if (patient != NULL)
                {
                    printf("患者姓名：%s\n", patient->name);
                }
                else
                {
                    printf("患者姓名：未知\n");
                }
                
                printf("当前状态：已占用\n");
                printf("------------------------------------------------------\n");
            }
            curr = curr->next;
        }
    }

    // 计算空闲床位数（假设有100个床位）
    free_count = 100 - occupied_count;
    if (free_count < 0) free_count = 0;

    if (occupied_count == 0)
    {
        printf("当前暂无床位占用预警。\n");
    }
    else
    {
        printf("已占用床位总数：%d\n", occupied_count);
        printf("空闲床位总数：%d\n", free_count);
    }
    
    printf("======================================================\n");
    printf("按任意键返回...\n");
    get_single_char("");
}

// 住院押金预警
static void show_deposit_warning(void)
{
    system("cls");
    printf("\n======================================================\n");
    printf("                住院押金预警\n");
    printf("======================================================\n");

    int count = 0;

    if (g_inpatient_list != NULL && g_inpatient_list->next != NULL)
    {
        InpatientRecord* curr = g_inpatient_list->next;
        
        while (curr != NULL)
        {
            if (curr->is_active)
            {
                // 根据病房类型估算日费用，若押金不足3天费用则预警
                double daily_cost = estimate_daily_cost_by_ward_type(curr->ward_type);
                double threshold = daily_cost * 3; // 押金不足3天费用时预警
                
                if (curr->deposit_balance < threshold)
                {
                    count++;
                    printf("患者编号：%s\n", curr->patient_id);
                    
                    // 查找患者姓名
                    PatientNode* patient = find_patient_by_id(g_patient_list, curr->patient_id);
                    if (patient != NULL)
                    {
                        printf("患者姓名：%s\n", patient->name);
                    }
                    else
                    {
                        printf("患者姓名：未知\n");
                    }
                    
                    printf("当前押金：%.2f\n", curr->deposit_balance);
                    printf("预警阈值：%.2f\n", threshold);
                    printf("差额：%.2f\n", threshold - curr->deposit_balance);
                    printf("当前住院状态：住院中\n");
                    printf("------------------------------------------------------\n");
                }
            }
            curr = curr->next;
        }
    }

    if (count == 0)
    {
        printf("当前暂无住院押金预警。\n");
    }
    else
    {
        printf("押金预警住院患者总数：%d\n", count);
    }
    
    printf("======================================================\n");
    printf("按任意键返回...\n");
    get_single_char("");
}

// 欠费/待缴费预警
static void show_arrears_warning(void)
{
    system("cls");
    printf("\n======================================================\n");
    printf("              欠费 / 待缴费预警\n");
    printf("======================================================\n");

    int count = 0;

    if (g_patient_list != NULL && g_patient_list->next != NULL)
    {
        PatientNode* curr = g_patient_list->next;
        
        while (curr != NULL)
        {
            if (curr->status == STATUS_UNPAID)
            {
                count++;
                printf("患者编号：%s\n", curr->id);
                printf("患者姓名：%s\n", curr->name);
                printf("当前状态：待缴费\n");
                printf("账户余额：%.2f\n", curr->balance);
                
                // 检查是否为住院患者
                int is_inpatient = 0;
                if (g_inpatient_list != NULL && g_inpatient_list->next != NULL)
                {
                    InpatientRecord* inpatient_curr = g_inpatient_list->next;
                    while (inpatient_curr != NULL)
                    {
                        if (inpatient_curr->is_active && strcmp(inpatient_curr->patient_id, curr->id) == 0)
                        {
                            is_inpatient = 1;
                            break;
                        }
                        inpatient_curr = inpatient_curr->next;
                    }
                }
                
                printf("是否住院：%s\n", is_inpatient ? "是" : "否");
                printf("备注：%s\n", curr->balance < 0 ? "欠费" : "待缴费");
                printf("------------------------------------------------------\n");
            }
            curr = curr->next;
        }
    }

    if (count == 0)
    {
        printf("当前暂无欠费 / 待缴费预警。\n");
    }
    else
    {
        printf("欠费 / 待缴费患者总数：%d\n", count);
    }
    
    printf("======================================================\n");
    printf("按任意键返回...\n");
    get_single_char("");
}

// 显示公共状态统计摘要
void show_public_status_statistics(void)
{
    printf("\n==============================================================\n");
    printf("                      公共状态统计摘要\n");
    printf("==============================================================\n");
    
    int patient_count = 0;
    int doctor_count = 0;
    int nurse_count = 0;
    int pharmacist_count = 0;
    int medicine_count = 0;
    
    // 统计患者数量
    if (g_patient_list != NULL && g_patient_list->next != NULL)
    {
        PatientNode* curr = g_patient_list->next;
        while (curr != NULL)
        {
            patient_count++;
            curr = curr->next;
        }
    }
    
    // 统计医护人员数量
    if (g_account_list != NULL && g_account_list->next != NULL)
    {
        AccountNode* curr = g_account_list->next;
        while (curr != NULL)
        {
            switch (curr->role)
            {
                case ROLE_DOCTOR:
                    doctor_count++;
                    break;
                case ROLE_NURSE:
                    nurse_count++;
                    break;
                case ROLE_PHARMACIST:
                    pharmacist_count++;
                    break;
                default:
                    break;
            }
            curr = curr->next;
        }
    }
    
    // 统计药品数量
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        MedicineNode* curr = g_medicine_list->next;
        while (curr != NULL)
        {
            medicine_count++;
            curr = curr->next;
        }
    }
    
    printf("患者总数：%d\n", patient_count);
    printf("医生总数：%d\n", doctor_count);
    printf("护士总数：%d\n", nurse_count);
    printf("药师总数：%d\n", pharmacist_count);
    printf("药品总数：%d\n", medicine_count);
    
    printf("==============================================================\n");
}

// 显示传染病异常提醒
void show_infectious_disease_alerts(void)
{
    printf("\n==============================================================\n");
    printf("                      传染病异常提醒\n");
    printf("==============================================================\n");
    printf("当前无传染病异常提醒\n");
    printf("==============================================================\n");
}

// 初始化日志链表
void init_log_list(void)
{
    if (g_log_list == NULL)
    {
        g_log_list = (LogNode*)malloc(sizeof(LogNode));
        if (g_log_list != NULL)
        {
            g_log_list->next = NULL;
        }
    }
}

// 添加日志
void add_log(const char* operation, const char* target, const char* description)
{
    if (operation == NULL || target == NULL || description == NULL)
        return;
    
    // 初始化日志链表（如果未初始化）
    if (g_log_list == NULL)
    {
        init_log_list();
    }
    
    // 创建新日志节点
    LogNode* new_node = (LogNode*)malloc(sizeof(LogNode));
    if (new_node == NULL)
        return;
    
    // 设置日志时间
    time_t now = time(NULL);
    struct tm now_tm;
    localtime_s(&now_tm, &now);
    snprintf(new_node->timestamp, sizeof(new_node->timestamp), 
             "%d-%02d-%02d %02d:%02d:%02d",
             now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday,
             now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec);
    
    // 设置日志内容
    strncpy(new_node->operation, operation, sizeof(new_node->operation) - 1);
    new_node->operation[sizeof(new_node->operation) - 1] = '\0';
    
    strncpy(new_node->target, target, sizeof(new_node->target) - 1);
    new_node->target[sizeof(new_node->target) - 1] = '\0';
    
    strncpy(new_node->description, description, sizeof(new_node->description) - 1);
    new_node->description[sizeof(new_node->description) - 1] = '\0';
    
    new_node->next = NULL;
    
    // 添加到链表尾部
    LogNode* curr = g_log_list;
    while (curr->next != NULL)
    {
        curr = curr->next;
    }
    curr->next = new_node;
}

// 显示日志
void show_logs(void)
{
    printf("\n==============================================================\n");
    printf("                      系统日志\n");
    printf("==============================================================\n");
    
    if (g_log_list == NULL || g_log_list->next == NULL)
    {
        printf("当前无日志记录\n");
        printf("==============================================================\n");
        return;
    }
    
    LogNode* curr = g_log_list->next;
    int count = 0;
    
    while (curr != NULL)
    {
        printf("[%s] %s - %s: %s\n", curr->timestamp, curr->operation, curr->target, curr->description);
        count++;
        curr = curr->next;
    }
    
    printf("--------------------------------------------------------------\n");
    printf("日志总数：%d\n", count);
    printf("==============================================================\n");
}

// 处理药品注册
void handle_medicine_register(void)
{
    char name[MAX_MED_NAME_LEN];
    char alias[MAX_ALIAS_LEN];
    char generic_name[MAX_GENERIC_NAME_LEN];
    double price;
    int stock;
    int medicare_type;
    char expiry_date[MAX_DATE_LEN];
    
    printf("\n==============================================================\n");
    printf("                      注册新药品\n");
    printf("==============================================================\n");
    
    get_safe_string("请输入药品名称：", name, sizeof(name));
    get_safe_string("请输入药品别名：", alias, sizeof(alias));
    get_safe_string("请输入药品通用名：", generic_name, sizeof(generic_name));
    price = get_safe_double("请输入药品价格：");
    stock = get_safe_int("请输入药品库存：");
    
    printf("请选择医保类型：\n");
    printf("1. 无医保\n");
    printf("2. 甲类医保\n");
    printf("3. 乙类医保\n");
    medicare_type = get_safe_int("请输入选择：");
    
    while (medicare_type < 1 || medicare_type > 3)
    {
        printf("输入错误，请重新选择：");
        medicare_type = get_safe_int("");
    }
    
    get_safe_string("请输入有效期（YYYY-MM-DD）：", expiry_date, sizeof(expiry_date));
    
    // 调用注册药品函数
    MedicineNode* result = register_medicine(
        name, alias, generic_name, price, stock, (MedicareType)(medicare_type - 1), expiry_date
    );
    
    if (result != NULL)
    {
        printf("\n药品注册成功！\n");
        add_log("药品注册", name, "注册新药品");
    }
    else
    {
        printf("\n药品注册失败！\n");
        add_log("药品注册", name, "注册药品失败");
    }
    
    printf("==============================================================\n");
}

// 处理药品基本信息更新
void handle_medicine_basic_info_update(void)
{
    char med_id[MAX_ID_LEN];
    char new_name[MAX_MED_NAME_LEN];
    char new_alias[MAX_ALIAS_LEN];
    char new_generic_name[MAX_GENERIC_NAME_LEN];
    double new_price;
    char new_expiry_date[MAX_DATE_LEN];
    
    printf("\n==============================================================\n");
    printf("                      更新药品基本信息\n");
    printf("==============================================================\n");
    
    get_safe_string("请输入药品编号：", med_id, sizeof(med_id));
    
    // 检查药品是否存在
    MedicineNode* medicine = find_medicine_by_id(g_medicine_list, med_id);
    if (medicine == NULL)
    {
        printf("药品不存在！\n");
        printf("==============================================================\n");
        return;
    }
    
    get_safe_string("请输入新的药品名称：", new_name, sizeof(new_name));
    get_safe_string("请输入新的药品别名：", new_alias, sizeof(new_alias));
    get_safe_string("请输入新的药品通用名：", new_generic_name, sizeof(new_generic_name));
    new_price = get_safe_double("请输入新的药品价格：");
    get_safe_string("请输入新的有效期（YYYY-MM-DD）：", new_expiry_date, sizeof(new_expiry_date));
    
    // 调用更新药品基本信息函数
    int result = update_medicine_basic_info(
        med_id, new_name, new_alias, new_generic_name, new_price, new_expiry_date
    );
    
    if (result == 1)
    {
        printf("\n药品基本信息更新成功！\n");
        add_log("药品信息更新", med_id, "更新药品基本信息");
    }
    else
    {
        printf("\n药品基本信息更新失败！\n");
        add_log("药品信息更新", med_id, "更新药品基本信息失败");
    }
    
    printf("==============================================================\n");
}

// 处理药品库存更新
void handle_medicine_stock_update(void)
{
    char med_id[MAX_ID_LEN];
    int new_stock;
    
    printf("\n==============================================================\n");
    printf("                      更新药品库存\n");
    printf("==============================================================\n");
    
    get_safe_string("请输入药品编号：", med_id, sizeof(med_id));
    
    // 检查药品是否存在
    MedicineNode* medicine = find_medicine_by_id(g_medicine_list, med_id);
    if (medicine == NULL)
    {
        printf("药品不存在！\n");
        printf("==============================================================\n");
        return;
    }
    
    new_stock = get_safe_int("请输入新的库存数量：");
    
    // 调用更新药品库存函数
    int result = update_medicine_stock(med_id, new_stock);
    
    if (result == 1)
    {
        printf("\n药品库存更新成功！\n");
        add_log("药品库存更新", med_id, "更新药品库存");
    }
    else
    {
        printf("\n药品库存更新失败！\n");
        add_log("药品库存更新", med_id, "更新药品库存失败");
    }
    
    printf("==============================================================\n");
}

// 处理近效期药品检查
void handle_expiring_medicine_check(void)
{
    char today[MAX_DATE_LEN];
    int days_threshold;
    
    printf("\n==============================================================\n");
    printf("                      近效期药品检查\n");
    printf("==============================================================\n");
    
    get_safe_string("请输入当前日期（YYYY-MM-DD）：", today, sizeof(today));
    days_threshold = get_safe_int("请输入预警天数：");
    
    // 调用显示近效期药品函数
    show_expiring_medicines(today, days_threshold);
    
    add_log("近效期检查", today, "执行近效期药品检查");
    printf("==============================================================\n");
}

// 处理药品发药
void handle_medicine_dispense(void)
{
    char patient_id[MAX_ID_LEN];
    
    printf("\n==============================================================\n");
    printf("                      药品发药\n");
    printf("==============================================================\n");
    
    // 显示待发药患者列表
    show_paid_patients_waiting_for_dispense();
    
    get_safe_string("请输入患者编号：", patient_id, sizeof(patient_id));
    
    // 调用发药函数
    int result = dispense_medicine_for_patient(patient_id);
    
    if (result == 1)
    {
        printf("\n发药成功！\n");
        add_log("药品发药", patient_id, "为患者发药");
    }
    else
    {
        printf("\n发药失败！\n");
        add_log("药品发药", patient_id, "发药失败");
    }
    
    printf("==============================================================\n");
}

// 处理药品下架
void handle_medicine_remove(void)
{
    char med_id[MAX_ID_LEN];
    char confirm;

    printf("\n==============================================================\n");
    printf("                      药品下架\n");
    printf("==============================================================\n");

    get_safe_string("请输入药品编号：", med_id, sizeof(med_id));

    // 检查药品是否存在
    MedicineNode* medicine = find_medicine_by_id(g_medicine_list, med_id);
    if (medicine == NULL)
    {
        printf("药品不存在！\n");
        printf("==============================================================\n");
        return;
    }

    // 显示药品基本信息
    printf("\n该药品信息如下：\n");
    printf("--------------------------------------------------------------\n");
    printf("药品编号：%s\n", medicine->id);
    printf("药品名称：%s\n", medicine->name);
    printf("药品别名：%s\n", medicine->alias);
    printf("通用名称：%s\n", medicine->generic_name);
    printf("药品价格：%.2f\n", medicine->price);
    printf("当前库存：%d\n", medicine->stock);
    printf("有效期：%s\n", medicine->expiry_date);

    // 根据医保类型显示文字
    const char* medicare_text = "无医保";
    switch (medicine->m_type)
    {
        case MEDICARE_CLASS_A:
            medicare_text = "甲类医保";
            break;
        case MEDICARE_CLASS_B:
            medicare_text = "乙类医保";
            break;
        default:
            medicare_text = "无医保";
            break;
    }
    printf("医保类型：%s\n", medicare_text);
    printf("--------------------------------------------------------------\n");

    printf("\n确定要下架该药品吗？(Y/N)：");
    confirm = get_single_char("");

    if (confirm == 'Y' || confirm == 'y')
    {
        // 调用下架药品函数
        int result = remove_medicine(med_id);

        if (result == 1)
        {
            printf("\n药品下架成功！\n");
            add_log("药品下架", med_id, "下架药品");
        }
        else
        {
            printf("\n药品下架失败！\n");
            add_log("药品下架", med_id, "下架药品失败");
        }
    }
    else
    {
        printf("\n取消下架操作\n");
    }

    printf("==============================================================\n");
}

// 显示评价统计
void show_evaluation_statistics()
{
    if (g_consult_record_list == NULL)
    {
        printf("\n⚠️ 接诊记录链表尚未初始化！\n");
        return;
    }

    int total_evaluations = 0;
    int unevaluated_count = 0;
    int star_counts[6] = {0};   // 1~5 星，0 位置不用
    float total_score = 0.0f;

    ConsultRecordNode* curr = g_consult_record_list->next;
    while (curr != NULL)
    {
        if (curr->star_rating >= 1 && curr->star_rating <= 5)
        {
            total_evaluations++;
            star_counts[curr->star_rating]++;
            total_score += curr->star_rating;
        }
        else
        {
            unevaluated_count++;
        }
        curr = curr->next;
    }

    printf("\n======================================================\n");
    printf("                    评价统计\n");
    printf("======================================================\n");
    printf("总评价数量：%d\n", total_evaluations);

    if (total_evaluations > 0)
    {
        float average_score = total_score / total_evaluations;
        printf("平均满意度：%.2f 星\n", average_score);
        printf("------------------------------------------------------\n");
        printf("各星级评价分布：\n");
        printf("1 星：%d\n", star_counts[1]);
        printf("2 星：%d\n", star_counts[2]);
        printf("3 星：%d\n", star_counts[3]);
        printf("4 星：%d\n", star_counts[4]);
        printf("5 星：%d\n", star_counts[5]);
    }
    else
    {
        printf("当前暂无有效评价数据。\n");
    }

    printf("未评价记录数：%d\n", unevaluated_count);
    printf("======================================================\n");
}

// 投诉管理菜单
void admin_complaint_menu(void)
{
    int running = 1;

    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               📝 投诉管理\n");
        printf("======================================================\n");
        printf("  [1] 查看所有投诉\n");
        printf("  [2] 处理投诉\n");
        printf("  [3] 按患者编号查询投诉历史\n");
        printf("  [4] 按投诉编号查询投诉详情\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");

        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                printf("\n查看所有投诉功能开发中...\n");
                system("pause");
                break;
            case 2:
                printf("\n处理投诉功能开发中...\n");
                system("pause");
                break;
            case 3:
                printf("\n按患者编号查询投诉历史功能开发中...\n");
                system("pause");
                break;
            case 4:
                printf("\n按投诉编号查询投诉详情功能开发中...\n");
                system("pause");
                break;
            case 0:
                running = 0;
                break;
            default:
                printf("\n⚠️ 无效的选项，请重新输入！\n");
                system("pause");
                break;
        }
    }
}

// 评价管理菜单
void admin_evaluation_menu(void)
{
    int running = 1;

    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               ⭐ 评价管理\n");
        printf("======================================================\n");
        printf("  [1] 查看所有评价\n");
        printf("  [2] 查看评价统计\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");

        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                printf("\n查看所有评价功能开发中...\n");
                system("pause");
                break;
            case 2:
                show_evaluation_statistics();
                system("pause");
                break;
            case 0:
                running = 0;
                break;
            default:
                printf("\n⚠️ 无效的选项，请重新输入！\n");
                system("pause");
                break;
        }
    }
}