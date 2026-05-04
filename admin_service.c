#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <direct.h>
#include <time.h>

#ifdef STATUS_PENDING
#undef STATUS_PENDING
#endif

#include "admin_service.h"
#include "list_ops.h"
#include "medicine_service.h"
#include "pharmacy_service.h"
#include "patient_service.h"
#include "appointment.h"
#include "utils.h"
#include "data_io.h"

// 病房费率定义
#define GENERAL_WARD_RATE 120.0  // 普通病房每天费用
#define ICU_WARD_RATE 500.0      // ICU每天费用
#define ISOLATION_WARD_RATE 300.0  // 隔离病房每天费用
#define SINGLE_WARD_RATE 400.0    // 单人病房每天费用

// 当前登录用户
AccountNode* g_current_account = NULL;
DoctorNode* g_current_doctor = NULL;
PatientNode* g_current_patient = NULL;

// 外部函数声明
extern void query_patient_complaints(const char* patient_id);

// 大小写不敏感的字符串比较函数
int my_strcasecmp(const char* s1, const char* s2)
{
    if (s1 == NULL || s2 == NULL)
    {
        return s1 - s2;
    }
    
    while (*s1 && *s2)
    {
        char c1 = *s1;
        char c2 = *s2;
        
        // 转换为小写进行比较
        if (c1 >= 'A' && c1 <= 'Z')
        {
            c1 += 'a' - 'A';
        }
        if (c2 >= 'A' && c2 <= 'Z')
        {
            c2 += 'a' - 'A';
        }
        
        if (c1 != c2)
        {
            return c1 - c2;
        }
        
        s1++;
        s2++;
    }
    
    return *s1 - *s2;
}

// 获取病房类型的费用
static double get_ward_rate(WardType ward_type)
{
    switch (ward_type) {
        case WARD_ICU:
            return ICU_WARD_RATE;
        case WARD_ISOLATION:
            return ISOLATION_WARD_RATE;
        case WARD_SINGLE:
            return SINGLE_WARD_RATE;
        case WARD_GENERAL:
        default:
            return GENERAL_WARD_RATE;
    }
}

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

    time_t now = time(NULL);
    struct tm now_tm;
    char current_date[11];

    localtime_s(&now_tm, &now);
    snprintf(current_date, sizeof(current_date), "%d-%02d-%02d",
             now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday);

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

// 前置声明
static void show_expiring_medicine_warning(void);



// 根据病房类型估算日费用
static double estimate_daily_cost_by_ward_type(WardType ward_type)
{
    switch (ward_type)
    {
        case WARD_GENERAL:
            return 200.0;
        case WARD_ICU:
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
    system("cls");
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
                    case MEDICARE_NONE:       medicare_name = "无医保";       break;
                    case MEDICARE_CLASS_A:    medicare_name = "甲类医保";     break;
                    case MEDICARE_CLASS_B:    medicare_name = "乙类医保";     break;
                    case MEDICARE_BASIC:      medicare_name = "基本医保";     break;
                    case MEDICARE_GOVERNMENT: medicare_name = "公务员医保";   break;
                    case MEDICARE_COMMERCIAL: medicare_name = "商业保险";     break;
                    default:                  medicare_name = "未知";         break;
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
                        case MEDICARE_NONE:       medicare_name = "无医保";       break;
                        case MEDICARE_CLASS_A:    medicare_name = "甲类医保";     break;
                        case MEDICARE_CLASS_B:    medicare_name = "乙类医保";     break;
                        case MEDICARE_BASIC:      medicare_name = "基本医保";     break;
                        case MEDICARE_GOVERNMENT: medicare_name = "公务员医保";   break;
                        case MEDICARE_COMMERCIAL: medicare_name = "商业保险";     break;
                        default:                  medicare_name = "未知";         break;
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
    printf("\n请按任意键返回...\n");
    system("pause");
}

// ==========================================
// 系统预警查看模块（整合所有7种预警类型）
// ==========================================

// 颜色定义
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

// 预警管理子菜单
void admin_alert_menu()
{
    int running = 1;
    
    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("                ⚠️  预警管理\n");
        printf("======================================================\n");
        printf("  [1] 查看事件预警（恶意挂号、爽约、急诊）\n");
        printf("  [2] 查看药品预警（低库存、临期药）\n");
        printf("  [3] 查看住院预警（押金不足、欠费）\n");
        printf("  [4] 查看床位资源预警\n");
        printf("  [5] 查看所有预警（整合视图）\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");
        
        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                show_event_alerts();
                system("pause");
                break;
            case 2:
                show_medicine_alerts();
                system("pause");
                break;
            case 3:
                show_inpatient_alerts();
                system("pause");
                break;
            case 4:
                show_bed_resource_alerts();
                system("pause");
                break;
            case 5:
                show_system_alerts();
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

// 显示床位资源预警
void show_bed_resource_alerts(void)
{
    int total_beds = 0;
    int occupied_beds = 0;
    int free_beds = 0;
    
    int general_total = 0, general_occupied = 0, general_free = 0;
    int icu_total = 0, icu_occupied = 0, icu_free = 0;
    int isolation_total = 0, isolation_occupied = 0, isolation_free = 0;
    int single_total = 0, single_occupied = 0, single_free = 0;
    
    printf("\n=====================================================\n");
    printf("                     床位资源预警\n");
    printf("=====================================================\n");
    
    if (g_ward_list == NULL || g_ward_list->next == NULL)
    {
        printf("\n当前无床位数据，无法生成床位资源预警。\n");
        printf("=====================================================\n");
        return;
    }
    
    WardNode* ward_curr = g_ward_list->next;
    while (ward_curr != NULL)
    {
        total_beds++;
        
        if (ward_curr->is_occupied)
            occupied_beds++;
        
        switch (ward_curr->ward_type)
        {
            case WARD_GENERAL:
                general_total++;
                if (ward_curr->is_occupied)
                    general_occupied++;
                else
                    general_free++;
                break;
            case WARD_ICU:
                icu_total++;
                if (ward_curr->is_occupied)
                    icu_occupied++;
                else
                    icu_free++;
                break;
            case WARD_ISOLATION:
                isolation_total++;
                if (ward_curr->is_occupied)
                    isolation_occupied++;
                else
                    isolation_free++;
                break;
            case WARD_SINGLE:
                single_total++;
                if (ward_curr->is_occupied)
                    single_occupied++;
                else
                    single_free++;
                break;
            default:
                break;
        }
        
        ward_curr = ward_curr->next;
    }
    
    free_beds = total_beds - occupied_beds;
    double occupancy_rate = total_beds > 0 ? (double)occupied_beds / total_beds * 100 : 0;
    
    // 动态计算列宽
    int type_w = str_display_width("病房类型");
    int total_w = str_display_width("总数");
    int occupied_w = str_display_width("已占用");
    int free_w = str_display_width("空闲");
    
    // 病房类型列宽度
    const char* types[] = {"普通病房", "ICU", "隔离病房", "单人病房"};
    for (int i = 0; i < 4; i++) {
        int len = str_display_width(types[i]);
        if (len > type_w) type_w = len;
    }
    
    // 数字列宽度
    char num_str[16];
    sprintf(num_str, "%d", total_beds);
    int len = str_display_width(num_str);
    if (len > total_w) total_w = len;
    if (len > occupied_w) occupied_w = len;
    if (len > free_w) free_w = len;
    
    int col_padding = 4;
    
    // 输出概览信息
    printf("\n总床位数    ：%d\n", total_beds);
    printf("已占用床位数：%d\n", occupied_beds);
    printf("空闲床位数  ：%d\n", free_beds);
    printf("床位使用率  ：%.2f%%\n", occupancy_rate);
    
    // 输出分隔线
    int total_width = type_w + total_w + occupied_w + free_w + col_padding * 3 + 4;
    printf("\n");
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");
    
    // 输出表头
    print_padded_text("病房类型", type_w + col_padding);
    print_padded_text("总数", total_w + col_padding);
    print_padded_text("已占用", occupied_w + col_padding);
    print_padded_text("空闲", free_w + col_padding);
    printf("\n");
    
    // 输出分隔线
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");
    
    // 输出数据行 - 全部左对齐
    // 普通病房
    print_padded_text("普通病房", type_w + col_padding);
    sprintf(num_str, "%d", general_total);
    print_padded_text(num_str, total_w + col_padding);
    sprintf(num_str, "%d", general_occupied);
    print_padded_text(num_str, occupied_w + col_padding);
    sprintf(num_str, "%d", general_free);
    print_padded_text(num_str, free_w + col_padding);
    printf("\n");
    
    // ICU
    print_padded_text("ICU", type_w + col_padding);
    sprintf(num_str, "%d", icu_total);
    print_padded_text(num_str, total_w + col_padding);
    sprintf(num_str, "%d", icu_occupied);
    print_padded_text(num_str, occupied_w + col_padding);
    sprintf(num_str, "%d", icu_free);
    print_padded_text(num_str, free_w + col_padding);
    printf("\n");
    
    // 隔离病房
    print_padded_text("隔离病房", type_w + col_padding);
    sprintf(num_str, "%d", isolation_total);
    print_padded_text(num_str, total_w + col_padding);
    sprintf(num_str, "%d", isolation_occupied);
    print_padded_text(num_str, occupied_w + col_padding);
    sprintf(num_str, "%d", isolation_free);
    print_padded_text(num_str, free_w + col_padding);
    printf("\n");
    
    // 单人病房
    print_padded_text("单人病房", type_w + col_padding);
    sprintf(num_str, "%d", single_total);
    print_padded_text(num_str, total_w + col_padding);
    sprintf(num_str, "%d", single_occupied);
    print_padded_text(num_str, occupied_w + col_padding);
    sprintf(num_str, "%d", single_free);
    print_padded_text(num_str, free_w + col_padding);
    printf("\n");
    
    // 输出分隔线
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");
    
    // 输出床位资源状态
    if (free_beds == 0)
    {
        printf("\n床位资源状态：" RED "⚠️ 所有床位已满，请及时协调床位资源。" RESET "\n");
    }
    else if (free_beds < 5)
    {
        printf("\n床位资源状态：" RED "⚠️ 床位资源紧张，仅剩 %d 张空闲床位。" RESET "\n", free_beds);
    }
    else if (occupancy_rate >= 80)
    {
        printf("\n床位资源状态：" RED "⚠️ 床位使用率较高，请关注住院收治压力。" RESET "\n");
    }
    else
    {
        printf("\n床位资源状态：床位资源状态正常\n");
    }
    
    printf("=====================================================\n");
}

// 显示事件预警（恶意挂号、爽约、急诊）
void show_event_alerts(void)
{
    printf("\n======================================================\n");
    printf("                事件预警查看\n");
    printf("======================================================\n");
    
    if (g_alert_list != NULL && g_alert_list->next != NULL)
    {
        AlertNode* curr = g_alert_list->next;
        int count = 0;
        while (curr != NULL)
        {
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&curr->time));
            printf("[%s] " RED "%s" RESET "\n", time_str, curr->message);
            curr = curr->next;
            count++;
        }
        printf("共找到 %d 条事件预警\n", count);
    }
    else
    {
        printf("当前无事件预警\n");
    }
    
    printf("======================================================\n");
}

// 显示药品预警（低库存、临期药）
void show_medicine_alerts(void)
{
    int low_stock_count = 0;
    int expiring_count = 0;
    
    printf("\n======================================================\n");
    printf("                药品预警查看\n");
    printf("======================================================\n");
    
    // 显示低库存药品预警
    printf("【低库存药品预警】\n");
    printf("------------------------------------------------------\n");
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        MedicineNode* med_curr = g_medicine_list->next;
        while (med_curr != NULL)
        {
            if (med_curr->stock < 5)
            {
                printf("药品编号：%s\n", med_curr->id);
                printf("商品名：%s\n", med_curr->name);
                printf("当前库存：" RED "%d" RESET "\n", med_curr->stock);
                printf("效期：%s\n", med_curr->expiry_date);
                printf("------------------------------------------------------\n");
                low_stock_count++;
            }
            med_curr = med_curr->next;
        }
        if (low_stock_count == 0)
        {
            printf("当前无低库存药品预警\n");
        }
        printf("共找到 %d 种低库存药品\n", low_stock_count);
    }
    else
    {
        printf("当前无药品数据\n");
    }
    printf("------------------------------------------------------\n");
    
    // 显示临期药品预警
    printf("【临期药品预警】\n");
    printf("------------------------------------------------------\n");
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        MedicineNode* med_curr = g_medicine_list->next;
        time_t now = time(NULL);
        struct tm* tm_now = localtime(&now);
        char today_str[11];
        sprintf(today_str, "%04d-%02d-%02d", tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday);
        
        while (med_curr != NULL)
        {
            int diff_days = days_between_dates(today_str, med_curr->expiry_date);
            if (diff_days >= 0 && diff_days <= 30)
            {
                printf("药品编号：%s\n", med_curr->id);
                printf("商品名：%s\n", med_curr->name);
                printf("效期：%s\n", med_curr->expiry_date);
                printf("距离过期：" RED "%d天" RESET "\n", diff_days);
                printf("------------------------------------------------------\n");
                expiring_count++;
            }
            med_curr = med_curr->next;
        }
        if (expiring_count == 0)
        {
            printf("当前无临期药品预警\n");
        }
        printf("共找到 %d 种临期药品\n", expiring_count);
    }
    else
    {
        printf("当前无药品数据\n");
    }
    
    printf("======================================================\n");
}

// 显示住院预警（押金不足、欠费）
void show_inpatient_alerts(void)
{
    int deposit_warning_count = 0;
    
    printf("\n======================================================\n");
    printf("                住院预警查看\n");
    printf("======================================================\n");
    
    // 显示押金不足预警
    printf("【押金不足预警】\n");
    printf("------------------------------------------------------\n");
    if (g_inpatient_list != NULL && g_inpatient_list->next != NULL)
    {
        InpatientRecord* inpatient_curr = g_inpatient_list->next;
        PatientNode* patient = NULL;
        
        while (inpatient_curr != NULL)
        {
            if (inpatient_curr->is_active)
            {
                patient = find_patient_by_id(g_patient_list, inpatient_curr->patient_id);
                if (patient != NULL)
                {
                    double daily_rate = 0.0;
                    const char* warn_msg = "";
                    int show_warn = 0;
                    
                    if (strlen(inpatient_curr->bed_id) > 0)
                    {
                        daily_rate = get_ward_rate(inpatient_curr->ward_type);
                        if (inpatient_curr->deposit_balance < daily_rate * 3)
                        {
                            warn_msg = "押金不足";
                            show_warn = 1;
                        }
                    }
                    else if (strlen(inpatient_curr->bed_id) == 0)
                    {
                        double warning_threshold = get_ward_rate(inpatient_curr->recommended_ward_type) * 3;
                        if (inpatient_curr->deposit_balance < warning_threshold)
                        {
                            warn_msg = "押金较少";
                            show_warn = 1;
                        }
                    }
                    
                    if (show_warn)
                    {
                        printf("患者编号：%s\n", inpatient_curr->patient_id);
                        printf("患者姓名：%s\n", patient->name);
                        printf("押金余额：" RED "%.2f 元" RESET "\n", inpatient_curr->deposit_balance);
                        printf("预警状态：" RED "%s" RESET "\n", warn_msg);
                        printf("------------------------------------------------------\n");
                        deposit_warning_count++;
                    }
                }
            }
            inpatient_curr = inpatient_curr->next;
        }
        if (deposit_warning_count == 0)
        {
            printf("当前无押金不足预警\n");
        }
        printf("共找到 %d 位患者押金不足\n", deposit_warning_count);
    }
    else
    {
        printf("当前无住院数据\n");
    }
    printf("------------------------------------------------------\n");
    
    // 显示欠费患者预警（待启用）
    printf("【欠费患者预警】\n");
    printf("------------------------------------------------------\n");
    printf("当前版本欠费患者预警功能待收费/结算模块完全接入后启用。\n");
    
    printf("======================================================\n");
}

// 显示系统预警查看
void show_system_alerts(void)
{
    int has_alerts = 0;
    
    printf("\n======================================================\n");
    printf("                  系统预警查看\n");
    printf("======================================================\n");
    
    // 1. 显示恶意挂号拦截、爽约拦截、急诊强制放行（来自预警队列）
    printf("【事件预警】\n");
    printf("------------------------------------------------------\n");
    if (g_alert_list != NULL && g_alert_list->next != NULL)
    {
        AlertNode* curr = g_alert_list->next;
        int count = 0;
        while (curr != NULL)
        {
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&curr->time));
            printf("[%s] " RED "%s" RESET "\n", time_str, curr->message);
            curr = curr->next;
            count++;
            has_alerts = 1;
        }
        printf("共找到 %d 条事件预警\n", count);
    }
    else
    {
        printf("当前无事件预警\n");
    }
    printf("------------------------------------------------------\n");
    
    // 2. 显示低库存药品预警
    printf("【低库存药品预警】\n");
    printf("------------------------------------------------------\n");
    int low_stock_count = 0;
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        MedicineNode* med_curr = g_medicine_list->next;
        while (med_curr != NULL)
        {
            if (med_curr->stock < 5)
            {
                printf("药品编号：%s\n", med_curr->id);
                printf("商品名：%s\n", med_curr->name);
                printf("当前库存：" RED "%d" RESET "\n", med_curr->stock);
                printf("效期：%s\n", med_curr->expiry_date);
                printf("------------------------------------------------------\n");
                low_stock_count++;
                has_alerts = 1;
            }
            med_curr = med_curr->next;
        }
        if (low_stock_count == 0)
        {
            printf("当前无低库存药品预警\n");
        }
        printf("共找到 %d 种低库存药品\n", low_stock_count);
    }
    else
    {
        printf("当前无药品数据\n");
    }
    printf("------------------------------------------------------\n");
    
    // 3. 显示临期药品预警
    printf("【临期药品预警】\n");
    printf("------------------------------------------------------\n");
    int expiring_count = 0;
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        MedicineNode* med_curr = g_medicine_list->next;
        time_t now = time(NULL);
        struct tm* tm_now = localtime(&now);
        char today_str[11];
        sprintf(today_str, "%04d-%02d-%02d", tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday);
        
        while (med_curr != NULL)
        {
            int diff_days = days_between_dates(today_str, med_curr->expiry_date);
            if (diff_days >= 0 && diff_days <= 30)
            {
                printf("药品编号：%s\n", med_curr->id);
                printf("商品名：%s\n", med_curr->name);
                printf("效期：%s\n", med_curr->expiry_date);
                printf("距离过期：" RED "%d天" RESET "\n", diff_days);
                printf("------------------------------------------------------\n");
                expiring_count++;
                has_alerts = 1;
            }
            med_curr = med_curr->next;
        }
        if (expiring_count == 0)
        {
            printf("当前无临期药品预警\n");
        }
        printf("共找到 %d 种临期药品\n", expiring_count);
    }
    else
    {
        printf("当前无药品数据\n");
    }
    printf("------------------------------------------------------\n");
    
    // 4. 显示押金不足预警
    printf("【押金不足预警】\n");
    printf("------------------------------------------------------\n");
    int deposit_warning_count = 0;
    if (g_inpatient_list != NULL && g_inpatient_list->next != NULL)
    {
        InpatientRecord* inpatient_curr = g_inpatient_list->next;
        PatientNode* patient = NULL;
        
        while (inpatient_curr != NULL)
        {
            if (inpatient_curr->is_active)
            {
                patient = find_patient_by_id(g_patient_list, inpatient_curr->patient_id);
                if (patient != NULL)
                {
                    double daily_rate = 0.0;
                    const char* warn_msg = "";
                    int show_warn = 0;
                    
                    if (strlen(inpatient_curr->bed_id) > 0)
                    {
                        daily_rate = get_ward_rate(inpatient_curr->ward_type);
                        if (inpatient_curr->deposit_balance < daily_rate * 3)
                        {
                            warn_msg = "押金不足";
                            show_warn = 1;
                        }
                    }
                    else if (strlen(inpatient_curr->bed_id) == 0)
                    {
                        double warning_threshold = get_ward_rate(inpatient_curr->recommended_ward_type) * 3;
                        if (inpatient_curr->deposit_balance < warning_threshold)
                        {
                            warn_msg = "押金较少";
                            show_warn = 1;
                        }
                    }
                    
                    if (show_warn)
                    {
                        printf("患者编号：%s\n", inpatient_curr->patient_id);
                        printf("患者姓名：%s\n", patient->name);
                        printf("押金余额：" RED "%.2f 元" RESET "\n", inpatient_curr->deposit_balance);
                        printf("预警状态：" RED "%s" RESET "\n", warn_msg);
                        printf("------------------------------------------------------\n");
                        deposit_warning_count++;
                        has_alerts = 1;
                    }
                }
            }
            inpatient_curr = inpatient_curr->next;
        }
        if (deposit_warning_count == 0)
        {
            printf("当前无押金不足预警\n");
        }
        printf("共找到 %d 位患者押金不足\n", deposit_warning_count);
    }
    else
    {
        printf("当前无住院数据\n");
    }
    printf("------------------------------------------------------\n");
    
    // 5. 显示欠费患者预警（待启用）
    printf("【欠费患者预警】\n");
    printf("------------------------------------------------------\n");
    printf("当前版本欠费患者预警功能待收费/结算模块完全接入后启用。\n");
    printf("------------------------------------------------------\n");
    
    printf("======================================================\n");
    if (!has_alerts)
    {
        printf("✅ 当前无预警，系统运行正常\n");
    }
    else
    {
        printf("" RED "⚠️ 发现预警信息，请及时处理" RESET "\n");
    }
    printf("======================================================\n");
}

// 回收站管理菜单
void admin_recycle_bin_menu(void)
{
    int running = 1;
    int choice = 0;
    
    while (running)
    {
        system("cls");
        printf("\n================ 回收站管理 ================\n");
        printf("  [1] 查看回收站\n");
        printf("  [2] 恢复药品\n");
        printf("  [0] 返回\n");
        printf("--------------------------------------------\n");
        
        switch (get_safe_int("请输入选择："))
        {
            case 1:
                system("cls");
                show_recycle_bin();
                system("pause");
                break;
            case 2:
                system("cls");
                handle_restore_medicine_from_recycle();
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

// 显示回收站
void show_recycle_bin(void)
{
    printf("\n================ 回收站内容 ================\n");
    printf("当前回收站功能开发中...\n");
    printf("============================================\n");
}

// 从回收站恢复药品
void handle_restore_medicine_from_recycle(void)
{
    printf("\n================ 恢复药品 ================\n");
    printf("当前恢复药品功能开发中...\n");
    printf("==========================================\n");
}

// ==========================================
// 投诉管理模块
// ==========================================

// 管理员投诉管理子菜单
void admin_complaint_menu()
{
    int running = 1;
    
    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               📝 投诉管理\n");
        printf("======================================================\n");
        printf("  [1] 查看已处理投诉\n");
        printf("  [2] 处理投诉\n");
        printf("  [3] 按患者编号查询投诉历史\n");
        printf("  [4] 按投诉编号查询投诉详情\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");
        
        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                system("cls");
                show_all_complaints();
                system("pause");
                break;
            case 2:
                system("cls");
                handle_complaint_response();
                system("pause");
                break;
            case 3:
                system("cls");
                query_patient_complaints_by_id();
                system("pause");
                break;
            case 4:
                system("cls");
                query_complaint_by_id();
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

// 显示已处理投诉
void show_all_complaints()
{
    if (g_complaint_list == NULL || g_complaint_list->next == NULL)
    {
        printf("\n⚠️ 当前暂无投诉记录！\n");
        return;
    }
    
    printf("\n======================================================\n");
    printf("                    已处理投诉列表\n");
    printf("======================================================\n");
    
    ComplaintNode* curr = g_complaint_list->next;
    int index = 1;
    int has_processed = 0;
    
    while (curr != NULL)
    {
        if (curr->status == 1)
        {
            has_processed = 1;
            printf("\n【投诉工单 %d】\n", index++);
            printf("工单编号：%s\n", curr->complaint_id);
            printf("患者编号：%s\n", curr->patient_id);
            printf("提交时间：%s\n", curr->submit_time);
            
            printf("投诉类型：");
            switch (curr->target_type)
            {
                case 1: printf("对医生\n"); break;
                case 2: printf("对护士/前台\n"); break;
                case 3: printf("对药师\n"); break;
                default: printf("未知\n"); break;
            }
            
            printf("被投诉人：%s（账号：%s）\n", curr->target_name, curr->target_id);
            printf("投诉内容：%s\n", curr->content);
            
            printf("处理状态：已回复\n");
            printf("处理意见：%s\n", curr->response);
            printf("------------------------------------------------------\n");
        }
        curr = curr->next;
    }
    
    if (!has_processed)
    {
        printf("\n⚠️ 当前没有已处理的投诉！\n");
    }
    
    printf("======================================================\n");
}

// 处理投诉
void handle_complaint_response()
{
    if (g_complaint_list == NULL || g_complaint_list->next == NULL)
    {
        printf("\n⚠️ 当前暂无投诉记录！\n");
        return;
    }

    char complaint_id[MAX_ID_LEN];
    char response[MAX_RECORD_LEN];
    ComplaintNode* target_complaint = NULL;

    while (1)
    {
        printf("\n======================================================\n");
        printf("                    待处理投诉\n");
        printf("======================================================\n");

        ComplaintNode* curr = g_complaint_list->next;
        int index = 1;
        int has_pending = 0;

        while (curr != NULL)
        {
            if (curr->status == 0)
            {
                has_pending = 1;
                printf("\n【投诉工单 %d】\n", index++);
                printf("工单编号：%s\n", curr->complaint_id);
                printf("患者编号：%s\n", curr->patient_id);
                printf("投诉类型：");
                switch (curr->target_type)
                {
                    case 1: printf("对医生\n"); break;
                    case 2: printf("对护士/前台\n"); break;
                    case 3: printf("对药师\n"); break;
                    default: printf("未知\n"); break;
                }
                printf("被投诉人：%s（账号：%s）\n", curr->target_name, curr->target_id);
                printf("投诉内容：%s\n", curr->content);
                printf("------------------------------------------------------\n");
            }
            curr = curr->next;
        }

        if (!has_pending)
        {
            printf("\n✅ 当前没有待处理的投诉！\n");
            return;
        }

        printf("提示：输入 B 返回上一步操作，输入 Q 退出操作\n\n");

        while (1)
        {
            get_safe_string("请输入要处理的投诉工单编号: ", complaint_id, MAX_ID_LEN);

            if (is_blank_string(complaint_id))
            {
                printf("⚠️ 输入不能为空或纯空格，请重新输入！\n");
                continue;
            }

            if (my_strcasecmp(complaint_id, "Q") == 0)
            {
                printf("\n已退出操作。\n");
                return;
            }

            if (my_strcasecmp(complaint_id, "B") == 0)
            {
                printf("\n已返回投诉管理菜单。\n");
                return;
            }

            break;
        }

        curr = g_complaint_list->next;
        target_complaint = NULL;

        while (curr != NULL)
        {
            if (strcmp(curr->complaint_id, complaint_id) == 0)
            {
                target_complaint = curr;
                break;
            }
            curr = curr->next;
        }

        if (target_complaint == NULL)
        {
            printf("\n⚠️ 未找到该投诉工单！\n");
            continue;
        }

        if (target_complaint->status != 0)
        {
            printf("\n⚠️ 该投诉工单已经处理过了！\n");
            continue;
        }

        while (1)
        {
            get_safe_string("请输入处理意见: ", response, MAX_RECORD_LEN);

            if (is_blank_string(response))
            {
                printf("⚠️ 输入不能为空或纯空格，请重新输入！\n");
                continue;
            }

            if (my_strcasecmp(response, "Q") == 0)
            {
                printf("\n已退出操作。\n");
                return;
            }

            if (my_strcasecmp(response, "B") == 0)
            {
                break;
            }

            target_complaint->status = 1;
            strncpy(target_complaint->response, response, MAX_RECORD_LEN - 1);
            target_complaint->response[MAX_RECORD_LEN - 1] = '\0';

            printf("\n✅ 投诉处理成功！\n");
            printf("工单编号：%s\n", target_complaint->complaint_id);
            printf("处理意见：%s\n", target_complaint->response);
            return;
        }
    }
}

// 按患者编号查询投诉历史
void query_patient_complaints_by_id()
{
    char patient_id[MAX_ID_LEN];

    system("cls");
    printf("=============== 按患者编号查询投诉历史 ===============\n");
    printf("提示：输入 Q 取消该操作返回上一级菜单\n\n");

    get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);

    if (my_strcasecmp(patient_id, "Q") == 0)
    {
        printf("\n已取消操作。\n");
        return;
    }

    query_patient_complaints(patient_id);
}

// 按投诉编号查询投诉详情
void query_complaint_by_id()
{
    char complaint_id[MAX_ID_LEN];

    system("cls");
    printf("=============== 按投诉编号查询投诉详情 ===============\n");
    printf("提示：输入 Q 取消该操作返回上一级菜单\n\n");

    get_safe_string("请输入投诉编号: ", complaint_id, MAX_ID_LEN);

    if (my_strcasecmp(complaint_id, "Q") == 0)
    {
        printf("\n已取消操作。\n");
        return;
    }

    if (is_blank_string(complaint_id))
    {
        printf("\n⚠️ 投诉编号不能为空！\n");
        return;
    }

    if (g_complaint_list == NULL || g_complaint_list->next == NULL)
    {
        printf("\n⚠️ 当前暂无投诉记录！\n");
        return;
    }
    
    // 查找投诉
    ComplaintNode* curr = g_complaint_list->next;
    ComplaintNode* target_complaint = NULL;
    
    while (curr != NULL)
    {
        if (strcmp(curr->complaint_id, complaint_id) == 0)
        {
            target_complaint = curr;
            break;
        }
        curr = curr->next;
    }
    
    if (target_complaint == NULL)
    {
        printf("\n⚠️ 未找到该投诉工单！\n");
        return;
    }
    
    // 显示投诉详情
    printf("\n======================================================\n");
    printf("                    投诉详情\n");
    printf("======================================================\n");
    printf("工单编号：%s\n", target_complaint->complaint_id);
    printf("患者编号：%s\n", target_complaint->patient_id);
    printf("提交时间：%s\n", target_complaint->submit_time);
    
    // 打印投诉类型
    printf("投诉类型：");
    switch (target_complaint->target_type)
    {
        case 1: printf("对医生\n"); break;
        case 2: printf("对护士/前台\n"); break;
        case 3: printf("对药师\n"); break;
        default: printf("未知\n"); break;
    }
    
    printf("被投诉人：%s（账号：%s）\n", target_complaint->target_name, target_complaint->target_id);
    printf("投诉内容：%s\n", target_complaint->content);
    
    // 打印处理状态
    printf("处理状态：");
    if (target_complaint->status == 0)
    {
        printf("待处理\n");
    }
    else
    {
        printf("已回复\n");
        printf("处理意见：%s\n", target_complaint->response);
    }
    printf("======================================================\n");
}

// ==========================================
// 评价管理模块
// ==========================================

// 管理员评价管理子菜单
void admin_evaluation_menu()
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
                system("cls");
                show_all_evaluations();
                system("pause");
                break;
            case 2:
                system("cls");
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

// 显示所有评价
void show_all_evaluations()
{
    if (g_consult_record_list == NULL || g_consult_record_list->next == NULL)
    {
        printf("\n⚠️ 当前暂无评价记录！\n");
        return;
    }
    
    printf("\n======================================================\n");
    printf("                    评价列表\n");
    printf("======================================================\n");
    
    ConsultRecordNode* curr = g_consult_record_list->next;
    int index = 1;
    int evaluation_count = 0;
    
    while (curr != NULL)
    {
        if (curr->star_rating > 0)
        {
            evaluation_count++;
            printf("\n【评价工单 %d】\n", index++);
            printf("记录编号：%s\n", curr->record_id);
            printf("患者编号：%s\n", curr->patient_id);
            printf("就诊医生：%s（编号：%s）\n", 
                   find_doctor_by_id(g_doctor_list, curr->doctor_id) ? 
                   find_doctor_by_id(g_doctor_list, curr->doctor_id)->name : "未知", 
                   curr->doctor_id);
            printf("就诊时间：%s\n", curr->consult_time);
            
            // 打印星级评价
            printf("满意度评分：");
            for (int i = 0; i < curr->star_rating; i++)
            {
                printf("⭐");
            }
            printf(" (%d星)\n", curr->star_rating);
            
            // 打印文字评价
            if (strlen(curr->feedback) > 0)
            {
                printf("文字评价：%s\n", curr->feedback);
            }
            else
            {
                printf("文字评价：（无）\n");
            }
            printf("------------------------------------------------------\n");
        }
        curr = curr->next;
    }
    
    if (evaluation_count == 0)
    {
        printf("\n⚠️ 当前暂无评价记录！\n");
    }
    else
    {
        printf("\n共找到 %d 条评价记录\n", evaluation_count);
    }
    printf("======================================================\n");
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
    int star_counts[6] = {0}; // star_counts[0] 表示未评价, star_counts[1-5] 表示各星级
    float total_score = 0.0;
    
    ConsultRecordNode* curr = g_consult_record_list->next;
    while (curr != NULL)
    {
        if (curr->star_rating > 0)
        {
            total_evaluations++;
            star_counts[curr->star_rating]++;
            total_score += curr->star_rating;
        }
        else
        {
            star_counts[0]++;
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
        printf("  ⭐⭐⭐⭐⭐ (5星): %d 条 (%.1f%%)\n", 
               star_counts[5], (float)star_counts[5] / total_evaluations * 100);
        printf("  ⭐⭐⭐⭐ (4星): %d 条 (%.1f%%)\n", 
               star_counts[4], (float)star_counts[4] / total_evaluations * 100);
        printf("  ⭐⭐⭐ (3星): %d 条 (%.1f%%)\n", 
               star_counts[3], (float)star_counts[3] / total_evaluations * 100);
        printf("  ⭐⭐ (2星): %d 条 (%.1f%%)\n", 
               star_counts[2], (float)star_counts[2] / total_evaluations * 100);
        printf("  ⭐ (1星): %d 条 (%.1f%%)\n", 
               star_counts[1], (float)star_counts[1] / total_evaluations * 100);
        
        // 好评率（4星及以上）
        int positive_count = star_counts[5] + star_counts[4];
        float positive_rate = (float)positive_count / total_evaluations * 100;
        printf("------------------------------------------------------\n");
        printf("好评率（4星及以上）：%.1f%%\n", positive_rate);
        
        // 差评率（2星及以下）
        int negative_count = star_counts[2] + star_counts[1];
        float negative_rate = (float)negative_count / total_evaluations * 100;
        printf("差评率（2星及以下）：%.1f%%\n", negative_rate);
    }
    else
    {
        printf("\n⚠️ 当前暂无评价记录！\n");
    }
    
    printf("======================================================\n");
    
    // 输出统计信息
    printf("评价总数：%d\n", total_evaluations);
    printf("======================================================\n");
    printf("按任意键返回...\n");
    get_single_char("");
}

static int str_disp_width(const char* s)
{
    int width = 0;
    while (*s)
    {
        unsigned char c = (unsigned char)*s;
        if (c < 0x80)      { width += 1; s += 1; }
        else if ((c & 0xE0) == 0xC0) { width += 2; s += 2; }
        else if ((c & 0xF0) == 0xE0) { width += 2; s += 3; }
        else if ((c & 0xF8) == 0xF0) { width += 2; s += 4; }
        else                { s += 1; }
    }
    return width;
}

static void print_field(const char* s, int field_width)
{
    int dw = str_disp_width(s);
    printf("%s", s);
    int pad = field_width - dw;
    if (pad > 0) printf("%*s", pad, "");
}

void show_load_monitoring(void)
{
    system("cls");
    printf("\n================================================================================\n");
    printf("                        公共负载监控\n");
    printf("================================================================================\n");

    int doctor_count = 0;
    int patient_count = 0;
    int bed_count = 0;
    int occupied_beds = 0;

    if (g_doctor_list != NULL && g_doctor_list->next != NULL)
    {
        DoctorNode* curr = g_doctor_list->next;
        while (curr != NULL)
        {
            doctor_count++;
            curr = curr->next;
        }
    }

    if (g_patient_list != NULL && g_patient_list->next != NULL)
    {
        PatientNode* curr = g_patient_list->next;
        while (curr != NULL)
        {
            patient_count++;
            curr = curr->next;
        }
    }

    if (g_ward_list != NULL && g_ward_list->next != NULL)
    {
        WardNode* curr = g_ward_list->next;
        while (curr != NULL)
        {
            bed_count++;
            if (curr->is_occupied)
                occupied_beds++;
            curr = curr->next;
        }
    }

    printf("【公共信息统计】\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("医生总数：%d\n", doctor_count);
    printf("患者总数：%d\n", patient_count);
    printf("床位总数：%d\n", bed_count);
    printf("已占用床位：%d\n", occupied_beds);
    printf("空闲床位：%d\n", bed_count - occupied_beds);
    printf("--------------------------------------------------------------------------------\n");

    printf("提示：此面板数据实时更新，可作为系统运行监控参考。\n");
    printf("================================================================================\n");
    printf("按任意键返回...\n");
    get_single_char("");
}

// 待发药患者预警
static void show_waiting_dispense_warning(void)
{
    system("cls");
    int count = 0;
    int total_width = 0;
    
    int id_width = get_display_width("患者编号");
    int name_width = get_display_width("姓名");
    int age_width = get_display_width("年龄");
    int status_width = get_display_width("当前状态");
    int dept_width = get_display_width("就诊科室");
    total_width = id_width + name_width + age_width + status_width + dept_width + 20; // 20是列间距总和
    
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

    // 第二步：按动态列宽输出表格
    printf("\n");
    total_width = id_width + name_width + age_width + status_width + dept_width + 20; // 20是列间距总和
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    // 输出标题
    int title_width = get_display_width("待发药患者预警");
    int title_padding = (total_width - title_width) / 2;
    for (int i = 0; i < title_padding; i++) printf(" ");
    printf("待发药患者预警");
    for (int i = 0; i < total_width - title_width - title_padding; i++) printf(" ");
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    if (count == 0)
    {
        printf("当前无待发药患者\n");
    }
    else
    {
        // 输出表头
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
    }
    
    // 输出底部横线
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");
    
    char count_str[50];
    snprintf(count_str, sizeof(count_str), "待取药患者数量：%d", count);
    int count_width = get_display_width(count_str);
    int count_padding = (total_width - count_width) / 2;
    for (int i = 0; i < count_padding; i++) printf(" ");
    printf("%s", count_str);
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    printf("\n请按任意键返回...\n");
    system("pause");
}

// 函数声明
static void show_bed_occupancy_warning(void);
static void show_deposit_warning(void);
static void show_arrears_warning(void);

// 显示资源预警
void show_resource_warnings(void)
{
    int choice;

    do
    {
        system("cls");
        int low_stock_count = 0;
        int expiring_medicine_count = 0;
        int deposit_warning_count = 0;
        int waiting_dispense_count = 0;
        int occupied_beds = 0;
        int free_beds = 0;
        int unpaid_patient_count = 0;

        time_t now = time(NULL);
        struct tm now_tm;
        localtime_s(&now_tm, &now);
        char current_date[11];
        snprintf(current_date, sizeof(current_date), "%d-%02d-%02d",
                 now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday);

        // 统计低库存药品和近效期药品数量
        if (g_medicine_list != NULL && g_medicine_list->next != NULL)
        {
            MedicineNode* curr = g_medicine_list->next;
            
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
                show_expiring_medicines(current_date, 30);
                system("pause");
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
    printf("\n请按任意键返回...\n");
    system("pause");
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
    printf("\n请按任意键返回...\n");
    system("pause");
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
    printf("\n请按任意键返回...\n");
    system("pause");
}

// 显示负载监控


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
    
    PatientNode* patient_curr = NULL;
    DoctorNode* doctor_curr = NULL;
    AccountNode* account_curr = NULL;

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

    // 统计护士和药师总数
    if (g_account_list != NULL && g_account_list->next != NULL)
    {
        account_curr = g_account_list->next;
        while (account_curr != NULL)
        {
            if (account_curr->role == ROLE_NURSE)
            {
                nurse_count++;
            }
            else if (account_curr->role == ROLE_PHARMACIST)
            {
                pharmacist_count++;
            }
            account_curr = account_curr->next;
        }
    }

    // 显示公共状态统计摘要
    printf("\n======================================================\n");
    printf("                  状态统计摘要\n");
    printf("======================================================\n");
    printf("1. 患者总数：%d\n", patient_count);
    printf("2. 医生总数：%d\n", doctor_count);
    printf("3. 护士总数：%d\n", nurse_count);
    printf("4. 药师总数：%d\n", pharmacist_count);
    printf("5. 药品总数：%d\n", medicine_count);
    printf("======================================================\n");
    printf("提示：此面板数据实时更新，仅供参考。\n");
    printf("======================================================\n");
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

void handle_medicine_register(void)
{
    char name[MAX_MED_NAME_LEN] = "";
    char alias[MAX_ALIAS_LEN] = "";
    char generic_name[MAX_GENERIC_NAME_LEN] = "";
    char expiry_date[MAX_DATE_LEN] = "";
    double price = 0.0;
    int stock = 0;
    int medicare_type = 0;
    int step = 1;

    printf("\n================ 新增药品 ================\n");
    printf("提示：输入 B 返回上一步操作，输入 Q 取消该操作\n");

    while (step >= 1 && step <= 7)
    {
        switch (step)
        {
            case 1:
                while (1)
                {
                    if (!get_form_string("请输入商品名：", name, MAX_MED_NAME_LEN))
                    {
                        return;
                    }
                    if (my_strcasecmp(name, "Q") == 0)
                    {
                        printf("已取消操作\n");
                        return;
                    }
                    if (is_blank_string(name))
                    {
                        printf("商品名不能为空，请重新输入\n");
                        continue;
                    }
                    // 提前检查商品名是否已存在，避免用户白填后续信息
                    if (is_medicine_name_exists(name))
                    {
                        printf("警告：已存在同名药品，请使用不同名称或修改已有药品\n");
                        continue;
                    }
                    step = 2;
                    break;
                }
                break;

            case 2:
                while (1)
                {
                    if (!get_form_string("请输入别名（可留空）：", alias, MAX_ALIAS_LEN))
                    {
                        step = 1;
                        break;
                    }
                    if (my_strcasecmp(alias, "Q") == 0)
                    {
                        printf("已取消操作\n");
                        return;
                    }
                    step = 3;
                    break;
                }
                break;

            case 3:
                while (1)
                {
                    if (!get_form_string("请输入通用名：", generic_name, MAX_GENERIC_NAME_LEN))
                    {
                        step = 2;
                        break;
                    }
                    if (my_strcasecmp(generic_name, "Q") == 0)
                    {
                        printf("已取消操作\n");
                        return;
                    }
                    if (is_blank_string(generic_name))
                    {
                        printf("通用名不能为空，请重新输入\n");
                        continue;
                    }
                    step = 4;
                    break;
                }
                break;

            case 4:
                while (1)
                {
                    char price_str[32];
                    char* endptr;
                    get_safe_string("请输入药品单价：", price_str, sizeof(price_str));

                    if (my_strcasecmp(price_str, "B") == 0)
                    {
                        step = 3;
                        break;
                    }
                    if (my_strcasecmp(price_str, "Q") == 0)
                    {
                        printf("已取消操作\n");
                        return;
                    }
                    if (is_blank_string(price_str))
                    {
                        printf("单价必须是大于 0 的数字，请重新输入\n");
                        continue;
                    }
                    errno = 0;
                    price = strtod(price_str, &endptr);
                    if (*endptr != '\0' || errno != 0 || price <= 0)
                    {
                        printf("单价必须是大于 0 的数字，请重新输入\n");
                        continue;
                    }
                    step = 5;
                    break;
                }
                break;

            case 5:
                while (1)
                {
                    char stock_str[32];
                    char* endptr;
                    get_safe_string("请输入初始库存：", stock_str, sizeof(stock_str));

                    if (my_strcasecmp(stock_str, "B") == 0)
                    {
                        step = 4;
                        break;
                    }
                    if (my_strcasecmp(stock_str, "Q") == 0)
                    {
                        printf("已取消操作\n");
                        return;
                    }
                    if (is_blank_string(stock_str))
                    {
                        printf("库存必须是大于等于 0 的整数，请重新输入\n");
                        continue;
                    }
                    errno = 0;
                    stock = strtol(stock_str, &endptr, 10);
                    if (*endptr != '\0' || errno != 0 || stock < 0)
                    {
                        printf("库存必须是大于等于 0 的整数，请重新输入\n");
                        continue;
                    }
                    step = 6;
                    break;
                }
                break;

            case 6:
                printf("医保类型：0=非医保 1=甲类医保 2=乙类医保\n");
                while (1)
                {
                    char type_str[32];
                    get_safe_string("请输入医保类型编号：", type_str, sizeof(type_str));

                    if (my_strcasecmp(type_str, "B") == 0)
                    {
                        step = 5;
                        break;
                    }
                    if (my_strcasecmp(type_str, "Q") == 0)
                    {
                        printf("已取消操作\n");
                        return;
                    }
                    if (sscanf(type_str, "%d", &medicare_type) != 1 || medicare_type < 0 || medicare_type > 2)
                    {
                        printf("医保类型输入非法，请重新输入\n");
                        continue;
                    }
                    step = 7;
                    break;
                }
                break;

            case 7:
                while (1)
                {
                    if (!get_form_date("请输入效期（YYYY-MM-DD）：", expiry_date, MAX_DATE_LEN))
                    {
                        step = 6;
                        break;
                    }
                    if (my_strcasecmp(expiry_date, "Q") == 0)
                    {
                        printf("已取消操作\n");
                        return;
                    }
                    step = 8;
                    break;
                }
                break;

            default:
                break;
        }
    }

    if (step == 8)
    {
        register_medicine(
            name,
            alias,
            generic_name,
            price,
            stock,
            (MedicareType)medicare_type,
            expiry_date
        );
    }
}

void handle_medicine_basic_info_update(void)
{
    char med_id[MAX_ID_LEN];
    char new_name[MAX_MED_NAME_LEN];
    char new_alias[MAX_ALIAS_LEN];
    char new_generic_name[MAX_GENERIC_NAME_LEN];
    char new_expiry_date[MAX_DATE_LEN];
    char* alias_ptr = NULL;
    MedicineNode* medicine_node = NULL;

    while (1)
    {
        system("cls");
        printf("\n================ 修改药品基础信息 ===============-\n");
        printf("提示：输入 B 返回上一步操作，输入 Q 取消该操作\n");

        // 1. 药品编号（必填）
        while (1)
        {
            get_safe_string("请输入药品编号：", med_id, MAX_ID_LEN);

            // 优先检查退出/返回指令
            if (my_strcasecmp(med_id, "Q") == 0)
            {
                printf("已取消操作\n");
                return;
            }
            if (my_strcasecmp(med_id, "B") == 0)
            {
                printf("已取消操作\n");
                return;
            }

            // 检查是否为空
            if (is_blank_string(med_id))
            {
                printf("药品编号不能为空，请重新输入\n");
                continue;
            }

            // 执行正常的药品搜索
            medicine_node = find_medicine_by_id(g_medicine_list, med_id);
            if (medicine_node == NULL)
            {
                printf("未找到编号为 %s 的药品，修改流程结束\n", med_id);
                printf("\n按任意键继续...\n");
                system("pause");
                continue;
            }

            break;
        }

        // 显示当前药品信息
        printf("\n------------- 当前药品信息 ------------\n");
        printf("药品编号：%s\n", medicine_node->id);
        printf("商品名：%s\n", medicine_node->name);
        printf("通用名：%s\n", medicine_node->generic_name);
        printf("别名：%s\n", (medicine_node->alias[0] == '\0') ? "无" : medicine_node->alias);
        printf("单价：%.2f\n", medicine_node->price);
        printf("库存：%d\n", medicine_node->stock);

        // 显示医保类型
        switch (medicine_node->m_type)
        {
            case MEDICARE_NONE:
                printf("医保类型：非医保\n");
                break;
            case MEDICARE_CLASS_A:
                printf("医保类型：甲类医保\n");
                break;
            case MEDICARE_CLASS_B:
                printf("医保类型：乙类医保\n");
                break;
            default:
                printf("医保类型：未知\n");
                break;
        }
        printf("效期：%s\n", medicine_node->expiry_date);
        printf("----------------------------------------\n");

        // 2. 商品名（选填，空白字符串表示不修改）
        get_safe_string("请输入新商品名（留空不修改）：", new_name, MAX_MED_NAME_LEN);
        if (my_strcasecmp(new_name, "Q") == 0)
        {
            printf("已取消操作\n");
            return;
        }
        if (my_strcasecmp(new_name, "B") == 0)
        {
            continue;
        }

        // 3. 通用名（选填，空白字符串表示不修改）
        get_safe_string("请输入新通用名（留空不修改）：", new_generic_name, MAX_GENERIC_NAME_LEN);
        if (my_strcasecmp(new_generic_name, "Q") == 0)
        {
            printf("已取消操作\n");
            return;
        }
        
        break;
    }

    // 显示当前药品信息
    printf("\n------------- 当前药品信息 ------------\n");
    printf("药品编号：%s\n", medicine_node->id);
    printf("商品名：%s\n", medicine_node->name);
    printf("通用名：%s\n", medicine_node->generic_name);
    printf("别名：%s\n", (medicine_node->alias[0] == '\0') ? "无" : medicine_node->alias);
    printf("单价：%.2f\n", medicine_node->price);
    printf("库存：%d\n", medicine_node->stock);
    
    // 显示医保类型
    switch (medicine_node->m_type)
    {
        case MEDICARE_NONE:       printf("医保类型：非医保\n");       break;
        case MEDICARE_CLASS_A:    printf("医保类型：甲类医保\n");     break;
        case MEDICARE_CLASS_B:    printf("医保类型：乙类医保\n");     break;
        case MEDICARE_BASIC:      printf("医保类型：基本医保\n");     break;
        case MEDICARE_GOVERNMENT: printf("医保类型：公务员医保\n");   break;
        case MEDICARE_COMMERCIAL: printf("医保类型：商业保险\n");     break;
        default:                  printf("医保类型：未知\n");         break;
    }
    printf("效期：%s\n", medicine_node->expiry_date);
    printf("----------------------------------------\n");

    // 2. 商品名（选填，空白字符串表示不修改）
    get_safe_string("请输入新商品名（留空不修改，输入 B 返回上一级）: ", new_name, MAX_MED_NAME_LEN);
    if (strcmp(new_name, "B") == 0 || strcmp(new_name, "b") == 0)
    {
        return;
    }

    // 3. 通用名（选填，空白字符串表示不修改）
    get_safe_string("请输入新通用名（留空不修改，输入 B 返回上一级）: ", new_generic_name, MAX_GENERIC_NAME_LEN);
    if (strcmp(new_generic_name, "B") == 0 || strcmp(new_generic_name, "b") == 0)
    {
        return;
    }

    // 4. 别名（选填，输入 B 返回上一级，空字符串表示清空）
    get_safe_string("请输入新别名（留空不修改，输入 B 返回上一级，输入空字符串清空别名）: ", new_alias, MAX_ALIAS_LEN);
    if (strcmp(new_alias, "B") == 0 || strcmp(new_alias, "b") == 0)
    {
        return;
    }
    if (!is_blank_string(new_alias))
    {
        alias_ptr = new_alias;
    }

    // 5. 单价（选填，-1 表示不修改）
    while (1)
    {
        char price_str[32];
        get_safe_string("请输入新单价（留空不修改，输入 B 返回上一级）: ", price_str, sizeof(price_str));
        if (strcmp(price_str, "B") == 0 || strcmp(price_str, "b") == 0)
        {
            continue;
        }

        // 4. 别名（选填，空字符串表示不修改）
        get_safe_string("请输入新别名（留空不修改）：", new_alias, MAX_ALIAS_LEN);
        if (my_strcasecmp(new_alias, "Q") == 0)
        {
            printf("已取消操作\n");
            return;
        }
        if (my_strcasecmp(new_alias, "B") == 0)
        {
            continue;
        }

        // 5. 单价（选填，-1 表示不修改）
        double new_price = -1.0;
        while (1)
        {
            char price_str[32];
            char* endptr;
            get_safe_string("请输入新单价（留空不修改）：", price_str, sizeof(price_str));
            if (my_strcasecmp(price_str, "Q") == 0)
            {
                printf("已取消操作\n");
                return;
            }
            if (my_strcasecmp(price_str, "B") == 0)
            {
                continue;
            }
            if (is_blank_string(price_str))
            {
                new_price = -1.0;
                break;
            }
            errno = 0;
            new_price = strtod(price_str, &endptr);
            if (*endptr != '\0' || errno != 0 || new_price <= 0)
            {
                printf("单价必须是大于 0 的数字，请重新输入\n");
                continue;
            }
            break;
        }

        // 6. 效期（选填，空白字符串表示不修改）
        while (1)
        {
            get_safe_string("请输入新效期（留空不修改）：", new_expiry_date, MAX_DATE_LEN);
            if (my_strcasecmp(new_expiry_date, "Q") == 0)
            {
                printf("已取消操作\n");
                return;
            }
            if (my_strcasecmp(new_expiry_date, "B") == 0)
            {
                continue;
            }
            if (is_blank_string(new_expiry_date))
            {
                break;
            }
            if (!is_valid_date_string(new_expiry_date))
            {
                printf("提示：药品效期格式非法，请使用 YYYY-MM-DD。\n");
                continue;
            }
            if (!is_future_date(new_expiry_date))
            {
                printf("提示：药品效期必须晚于今天。\n");
                continue;
            }
            break;
        }

        if (!is_blank_string(new_alias))
        {
            alias_ptr = new_alias;
        }

        // 调用 update_medicine_basic_info 函数
        update_medicine_basic_info(
            med_id,
            is_blank_string(new_name) ? NULL : new_name,
            alias_ptr,
            is_blank_string(new_generic_name) ? NULL : new_generic_name,
            new_price,
            is_blank_string(new_expiry_date) ? NULL : new_expiry_date
        );
        printf("按任意键返回...\n");
        system("pause");
        return;
    }
}

void handle_medicine_stock_update(void)
{
    char med_id[MAX_ID_LEN];
    char stock_str[32];
    int new_stock;
    MedicineNode* medicine = NULL;

    while (1)
    {
        system("cls");
        printf("\n================ 修改药品库存 ===============-\n");
        printf("提示：输入 B 返回上一步操作，输入 Q 取消该操作\n\n");

        get_safe_string("请输入药品编号：", med_id, MAX_ID_LEN);

        if (my_strcasecmp(med_id, "Q") == 0)
        {
            printf("已取消操作\n");
            return;
        }
        if (my_strcasecmp(med_id, "B") == 0)
        {
            printf("已取消操作\n");
            return;
        }

        if (is_blank_string(med_id))
        {
            printf("药品编号不能为空，请重新输入\n");
            printf("\n按任意键继续...\n");
            system("pause");
            continue;
        }

        medicine = find_medicine_by_id(g_medicine_list, med_id);
        if (medicine == NULL)
        {
            printf("未找到编号为 %s 的药品\n", med_id);
            printf("\n按任意键继续...\n");
            system("pause");
            continue;
        }

        printf("\n---------- 当前药品信息 ----------\n");
        printf("药品编号：%s\n", medicine->id);
        printf("商品名称：%s\n", medicine->name);
        printf("当前库存：%d\n", medicine->stock);
        printf("--------------------------------\n\n");

        while (1)
        {
            char* endptr;
            get_safe_string("请输入新库存数量：", stock_str, sizeof(stock_str));

            if (my_strcasecmp(stock_str, "Q") == 0)
            {
                printf("已取消操作\n");
                return;
            }
            if (my_strcasecmp(stock_str, "B") == 0)
            {
                break;
            }
            if (is_blank_string(stock_str))
            {
                printf("库存数量不能为空，请重新输入\n");
                continue;
            }
            errno = 0;
            new_stock = strtol(stock_str, &endptr, 10);
            if (*endptr != '\0' || errno != 0 || new_stock < 0)
            {
                printf("库存数量必须是大于等于 0 的整数，请重新输入\n");
                continue;
            }
            break;
        }

        if (my_strcasecmp(stock_str, "B") == 0)
        {
            continue;
        }

        update_medicine_stock(med_id, new_stock);
        printf("按任意键返回...\n");
        system("pause");
        return;
    }
}

void handle_medicine_search(void)
{
    char keyword[MAX_MED_NAME_LEN];

    printf("\n================ 查询药品 ================\n");
    printf("输入药品编号或关键词查询，输入 Q 退出\n\n");

    printf("请输入药品编号或关键词: ");
    fflush(stdout);
    if (fgets(keyword, MAX_MED_NAME_LEN, stdin) == NULL) return;
    keyword[strcspn(keyword, "\n")] = '\0';

    if (my_strcasecmp(keyword, "Q") == 0)
    {
        printf("已退出药品查询\n");
        return;
    }

    if (keyword[0] == '\0')
    {
        printf("关键词不能为空\n");
        return;
    }

    search_medicine_by_keyword(keyword);
    printf("------------------------------------------\n");
}

void handle_medicine_show_all(void)
{
    show_all_medicines();
}

void handle_medicine_low_stock(void)
{
    char threshold_str[32];

    printf("\n================ 查看低库存药品 ===============\n");
    printf("提示：输入 Q 取消操作返回菜单页\n\n");

    get_safe_string("请输入低库存阈值：", threshold_str, sizeof(threshold_str));

    if (my_strcasecmp(threshold_str, "Q") == 0)
    {
        printf("已取消操作\n");
        return;
    }

    int threshold = atoi(threshold_str);
    if (threshold <= 0)
    {
        printf("⚠️ 输入无效：库存阈值必须大于 0！\n");
        system("pause");
        return;
    }

    show_low_stock_medicines(threshold);
}

void handle_medicine_expiring(void)
{
    char today[MAX_DATE_LEN];
    int days_threshold;
    time_t now;
    struct tm* local_time;

    time(&now);
    local_time = localtime(&now);
    strftime(today, sizeof(today), "%Y-%m-%d", local_time);

    printf("\n当前系统日期：%s\n", today);
    days_threshold = get_safe_int("请输入近效期天数阈值: ");
    show_expiring_medicines(today, days_threshold);
}

void handle_medicine_dispense(void)
{
    char patient_id[MAX_ID_LEN];

    printf("\n================ 发药处理 ================\n");
    printf("提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
    show_paid_patients_waiting_for_dispense();
    printf("------------------------------------------------------\n");
    
    while (1)
    {
        get_safe_string("请输入要发药的患者编号: ", patient_id, MAX_ID_LEN);
        
        // 检查是否退出
        if (strcmp(patient_id, "00") == 0)
        {
            printf("操作取消！\n");
            return;
        }
        
        // 检查是否回退
        if (strcmp(patient_id, "0") == 0)
        {
            return;
        }
        
        // 检查患者编号格式是否合法
        if (validate_patient_id(patient_id))
        {
            break;
        }
        else
        {
            printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
            printf("提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
        }
    }
    
    dispense_medicine_for_patient(patient_id);
}

void handle_remove_medicine_from_shelf(void)
{
    char med_id[MAX_ID_LEN];
    char confirm_str[32];
    int confirm = 0;
    MedicineNode* medicine = NULL;

    while (1)
    {
        system("cls");
        printf("\n================ 下架药品 ===============-\n");
        printf("提示：输入 B 返回上一步操作，输入 Q 取消该操作\n\n");

        get_safe_string("请输入要下架的药品编号：", med_id, MAX_ID_LEN);

        if (my_strcasecmp(med_id, "Q") == 0)
        {
            printf("已取消操作\n");
            return;
        }
        if (my_strcasecmp(med_id, "B") == 0)
        {
            printf("已取消操作\n");
            return;
        }

        if (is_blank_string(med_id))
        {
            printf("药品编号不能为空，请重新输入\n");
            printf("\n按任意键继续...\n");
            system("pause");
            continue;
        }

        // 查找药品是否存在
        medicine = find_medicine_by_id(g_medicine_list, med_id);
        if (medicine == NULL)
        {
            printf("提示：未找到对应药品，下架失败。\n");
            printf("\n按任意键继续...\n");
            system("pause");
            continue;
        }

        // 显示药品基本信息
        printf("\n当前药品信息：\n");
        printf("药品编号：%s\n", medicine->id);
        printf("商品名：%s\n", medicine->name);
        printf("通用名：%s\n", medicine->generic_name);
        printf("别名：%s\n", medicine->alias[0] == '\0' ? "无" : medicine->alias);
        printf("单价：%.2f\n", medicine->price);
        printf("库存：%d\n", medicine->stock);
        // 显示医保类型
        switch (medicine->m_type)
        {
            case MEDICARE_NONE:
                printf("医保类型：非医保\n");
                break;
            case MEDICARE_CLASS_A:
                printf("医保类型：甲类医保\n");
                break;
            case MEDICARE_CLASS_B:
                printf("医保类型：乙类医保\n");
                break;
            default:
                printf("医保类型：未知\n");
                break;
        }
        printf("效期：%s\n", medicine->expiry_date);
        printf("----------------------------------------\n");

        // 二次确认
        printf("是否确定下架该药品？\n");
        printf("[1] 确认下架\n");
        printf("[0] 取消返回\n");

        get_safe_string("请输入操作编号：", confirm_str, sizeof(confirm_str));

        if (my_strcasecmp(confirm_str, "Q") == 0)
        {
            printf("已取消操作\n");
            return;
        }
        if (my_strcasecmp(confirm_str, "B") == 0 || strcmp(confirm_str, "0") == 0)
        {
            printf("已取消操作\n");
            return;
        }
        if (strcmp(confirm_str, "1") == 0)
        {
            // 执行下架操作
            remove_medicine(med_id);
            printf("按任意键返回...\n");
            system("pause");
            return;
        }

        printf("无效的输入，请重新选择\n");
        printf("\n按任意键继续...\n");
        system("pause");
    }

    // 显示药品基本信息
    printf("\n当前药品信息：\n");
    printf("药品编号：%s\n", medicine->id);
    printf("商品名：%s\n", medicine->name);
    printf("通用名：%s\n", medicine->generic_name);
    printf("别名：%s\n", medicine->alias[0] == '\0' ? "无" : medicine->alias);
    printf("单价：%.2f\n", medicine->price);
    printf("库存：%d\n", medicine->stock);
    // 显示医保类型
    switch (medicine->m_type)
    {
        case MEDICARE_NONE:       printf("医保类型：非医保\n");       break;
        case MEDICARE_CLASS_A:    printf("医保类型：甲类医保\n");     break;
        case MEDICARE_CLASS_B:    printf("医保类型：乙类医保\n");     break;
        case MEDICARE_BASIC:      printf("医保类型：基本医保\n");     break;
        case MEDICARE_GOVERNMENT: printf("医保类型：公务员医保\n");   break;
        case MEDICARE_COMMERCIAL: printf("医保类型：商业保险\n");     break;
        default:                  printf("医保类型：未知\n");         break;
    }
    printf("效期：%s\n", medicine->expiry_date);
    printf("----------------------------------------\n");

    // 二次确认
    printf("是否确定下架该药品？\n");
    printf("[1] 确定下架\n");
    printf("[0] 取消返回\n");
    confirm = get_safe_int("请输入操作编号: ");

    if (confirm != 1)
    {
        printf("提示：已取消下架操作。\n");
        return;
    }

    // 执行下架操作
    remove_medicine(med_id);
}

// 前向声明
extern int soft_remove_medicine_to_recycle(const char* med_id, const char* deleted_by, const char* reason);

// 删除药品（放入回收站）
void handle_medicine_remove(void)
{
    char med_id[MAX_ID_LEN];
    char reason[MAX_RECORD_LEN];
    int confirm = 0;
    MedicineNode* medicine = NULL;

    while (1)
    {
        system("cls");
        printf("\n================ 删除药品 ================\n");
        printf("提示：输入 B 返回上一步操作，输入 Q 取消该操作\n\n");

        get_safe_string("请输入要删除的药品编号：", med_id, MAX_ID_LEN);

        if (my_strcasecmp(med_id, "Q") == 0)
        {
            printf("已取消操作\n");
            return;
        }
        if (my_strcasecmp(med_id, "B") == 0)
        {
            printf("已取消操作\n");
            return;
        }

        if (is_blank_string(med_id))
        {
            printf("药品编号不能为空，请重新输入\n");
            printf("\n按任意键继续...\n");
            system("pause");
            continue;
        }

        // 查找药品是否存在
        medicine = find_medicine_by_id(g_medicine_list, med_id);
        if (medicine == NULL)
        {
            printf("提示：未找到对应药品，删除失败。\n");
            printf("\n按任意键继续...\n");
            system("pause");
            continue;
        }

        // 显示药品基本信息
        printf("\n当前药品信息：\n");
        printf("药品编号：%s\n", medicine->id);
        printf("商品名：%s\n", medicine->name);
        printf("通用名：%s\n", medicine->generic_name);
        printf("别名：%s\n", medicine->alias[0] == '\0' ? "无" : medicine->alias);
        printf("单价：%.2f\n", medicine->price);
        printf("库存：%d\n", medicine->stock);
        printf("效期：%s\n", medicine->expiry_date);
        printf("----------------------------------------\n");

        // 二次确认
        printf("是否确定删除该药品？删除后将放入回收站。\n");
        printf("[1] 确定删除\n");
        printf("[0] 取消返回\n");
        confirm = get_safe_int("请输入操作编号: ");

        if (confirm != 1)
        {
            printf("提示：已取消删除操作。\n");
            return;
        }

        // 输入删除原因
        get_safe_string("请输入删除原因：", reason, MAX_RECORD_LEN);
        if (is_blank_string(reason))
        {
            strcpy(reason, "管理员删除");
        }

        // 执行删除操作（放入回收站）
        if (soft_remove_medicine_to_recycle(med_id, "admin", reason))
        {
            printf("提示：药品已成功删除并放入回收站。\n");
        }
        else
        {
            printf("提示：删除药品失败。\n");
        }
        return;
    }
}

// 初始化日志列表
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
        printf("\n请按任意键返回...\n");
        system("pause");
        return;
    }

    // 使用固定列宽确保对齐（考虑中文字符占用2个字符宽度）
    const int col1_width = 22;  // 时间列（"2026-04-30 17:58:04" = 19字符）
    const int col2_width = 16;  // 操作列（中文操作名）
    const int col3_width = 16;  // 目标列（日期或编号）
    const int total_width = col1_width + col2_width + col3_width + 40;

    // 打印分隔线
    for (int i = 0; i < total_width; i++) {
        printf("-");
    }
    printf("\n");

    // 打印表头（左对齐）
    printf("%-*s%-*s%-*s%s\n", col1_width, "时间", col2_width, "操作", col3_width, "目标", "描述");

    // 打印分隔线
    for (int i = 0; i < total_width; i++) {
        printf("-");
    }
    printf("\n");

    // 打印日志内容
    LogNode* curr = g_log_list->next;
    int count = 0;
    while (curr != NULL)
    {
        printf("%-*s%-*s%-*s%s\n", 
               col1_width, curr->timestamp,
               col2_width, curr->operation,
               col3_width, curr->target,
               curr->description);
        count++;
        curr = curr->next;
    }

    // 打印底部分隔线
    for (int i = 0; i < total_width; i++) {
        printf("-");
    }
    printf("\n");
    
    printf("日志总数：%d\n", count);
    printf("==============================================================\n");
    printf("\n请按任意键返回...\n");
    system("pause");
}

// 处理近效期药品检查
void handle_expiring_medicine_check(void)
{
    char today[MAX_DATE_LEN];
    char days_str[32];
    time_t now;
    struct tm* local_time;

    time(&now);
    local_time = localtime(&now);
    strftime(today, sizeof(today), "%Y-%m-%d", local_time);

    printf("\n================ 近效期药品检查 ===============\n");
    printf("系统当前日期：%s\n", today);
    printf("提示：输入 Q 取消该操作返回上一级\n\n");

    get_safe_string("请输入预警天数：", days_str, sizeof(days_str));

    if (my_strcasecmp(days_str, "Q") == 0)
    {
        printf("已取消操作\n");
        return;
    }

    int days_threshold = atoi(days_str);
    if (days_threshold <= 0)
    {
        printf("预警天数必须是大于 0 的整数\n");
        return;
    }

    show_expiring_medicines(today, days_threshold);

    add_log("近效期检查", today, "执行近效期药品检查");
    printf("==============================================================\n");
}

// 显示所有检查项目字典
void show_all_check_items(void)
{
    CheckItemNode* curr = NULL;
    int count = 0;

    if (g_check_item_list == NULL || g_check_item_list->next == NULL)
    {
        printf("当前暂无检查项目数据\n");
        return;
    }

    printf("\n========================================================================\n");
    printf("                            检查项目字典\n");
    printf("========================================================================\n");
    printf("%-10s %-20s %12s %10s   %-10s\n",
           "项目编号", "项目名称", "所属科室", "价格(元)", "医保类型");
    printf("------------------------------------------------------------------------\n");

    curr = g_check_item_list->next;
    while (curr != NULL)
    {
        const char* m_type_name = NULL;
        switch (curr->m_type)
        {
            case MEDICARE_CLASS_A:    m_type_name = "甲类";       break;
            case MEDICARE_CLASS_B:    m_type_name = "乙类";       break;
            case MEDICARE_NONE:       m_type_name = "自费";       break;
            case MEDICARE_BASIC:      m_type_name = "基本医保";   break;
            case MEDICARE_GOVERNMENT: m_type_name = "公务员医保"; break;
            case MEDICARE_COMMERCIAL: m_type_name = "商业保险";   break;
            default:                  m_type_name = "未知";       break;
        }
        printf("%-10s %-20s %12s %10.2f   %-10s\n",
               curr->item_id,
               curr->item_name,
               curr->dept,
               curr->price,
               m_type_name);
        count++;
        curr = curr->next;
    }

    printf("------------------------------------------------------------------------\n");
    printf("检查项目总数：%d\n", count);
    printf("========================================================================\n");
}

static void generate_check_item_id(char* new_id)
{
    int max_no = 0;
    int current_no = 0;
    CheckItemNode* curr = NULL;

    if (new_id == NULL) return;

    if (g_check_item_list != NULL)
    {
        curr = g_check_item_list->next;
        while (curr != NULL)
        {
            if (strncmp(curr->item_id, "C-", 2) == 0)
            {
                current_no = atoi(curr->item_id + 2);
                if (current_no > max_no) max_no = current_no;
            }
            curr = curr->next;
        }
    }

    snprintf(new_id, MAX_ID_LEN, "C-%03d", max_no + 1);
}

static int is_check_item_referenced_by_active_record(const char* item_id)
{
    if (is_blank_string(item_id) || g_check_record_list == NULL) return 0;

    CheckRecordNode* curr = g_check_record_list->next;
    while (curr != NULL)
    {
        if (curr->is_completed == 0 && strcmp(curr->item_id, item_id) == 0)
            return 1;
        curr = curr->next;
    }
    return 0;
}

void search_check_item_by_keyword(const char* keyword)
{
    int found = 0;
    CheckItemNode* curr = NULL;

    if (is_blank_string(keyword))
    {
        printf("提示：查询关键字不能为空。\n");
        return;
    }

    if (g_check_item_list == NULL || g_check_item_list->next == NULL)
    {
        printf("未找到匹配检查项目\n");
        return;
    }

    printf("\n========================================================================\n");
    printf("                        检查项目查询结果\n");
    printf("========================================================================\n");
    printf("%-12s %-28s %-16s %-12s %-8s\n",
           "项目编号", "项目名称", "所属科室", "价格(元)", "医保类型");
    printf("------------------------------------------------------------------------\n");

    curr = g_check_item_list->next;
    while (curr != NULL)
    {
        if (strstr(curr->item_id, keyword) != NULL ||
            strstr(curr->item_name, keyword) != NULL ||
            strstr(curr->dept, keyword) != NULL)
        {
            const char* m_type_name = NULL;
            switch (curr->m_type)
            {
                case MEDICARE_CLASS_A:    m_type_name = "甲类";       break;
                case MEDICARE_CLASS_B:    m_type_name = "乙类";       break;
                case MEDICARE_NONE:       m_type_name = "自费";       break;
                case MEDICARE_BASIC:      m_type_name = "基本医保";   break;
                case MEDICARE_GOVERNMENT: m_type_name = "公务员医保"; break;
                case MEDICARE_COMMERCIAL: m_type_name = "商业保险";   break;
                default:                  m_type_name = "未知";       break;
            }
            printf("%-12s %-28s %-16s %-12.2f %-8s\n",
                   curr->item_id, curr->item_name, curr->dept,
                   curr->price, m_type_name);
            found = 1;
        }
        curr = curr->next;
    }

    if (!found) printf("未找到匹配检查项目\n");
}

void handle_check_item_register(void)
{
    char item_name[MAX_NAME_LEN] = "";
    char dept[MAX_NAME_LEN] = "";
    double price = 0.0;
    int medicare_type = -1;
    char new_id[MAX_ID_LEN];
    char input_buf[64];
    int step = 1;

    printf("\n================ 新增检查项目 ================\n");
    printf("提示：输入 B 返回上一步，输入 Q 取消操作并返回菜单\n\n");

    while (1)
    {
        switch (step)
        {
            case 1:
                printf("请输入检查项目名称：");
                get_safe_string("", input_buf, sizeof(input_buf));
                if (my_strcasecmp(input_buf, "Q") == 0)
                {
                    return;
                }
                if (my_strcasecmp(input_buf, "B") == 0)
                {
                    printf("已取消操作。\n");
                    return;
                }
                if (is_blank_string(input_buf))
                {
                    printf("检查项目名称不能为空，请重新输入\n\n");
                    continue;
                }
                strncpy(item_name, input_buf, MAX_NAME_LEN - 1);
                if (g_check_item_list != NULL && g_check_item_list->next != NULL)
                {
                    CheckItemNode* curr = g_check_item_list->next;
                    while (curr != NULL)
                    {
                        if (strstr(curr->item_name, item_name) != NULL ||
                            strstr(item_name, curr->item_name) != NULL)
                        {
                            printf("⚠️ 拦截：系统中已存在相似项目【%s %s】，请勿重复添加！\n\n", curr->item_id, curr->item_name);
                            memset(item_name, 0, MAX_NAME_LEN);
                            break;
                        }
                        curr = curr->next;
                    }
                }
                if (is_blank_string(item_name))
                {
                    continue;
                }
                step++;
                break;

            case 2:
                printf("请输入所属科室：");
                get_safe_string("", input_buf, sizeof(input_buf));
                if (my_strcasecmp(input_buf, "Q") == 0)
                {
                    return;
                }
                if (my_strcasecmp(input_buf, "B") == 0)
                {
                    step = 1;
                    printf("\n");
                    continue;
                }
                if (is_blank_string(input_buf))
                {
                    printf("所属科室不能为空，请重新输入\n\n");
                    continue;
                }
                if (!department_exists(input_buf))
                {
                    printf("⚠️ 错误：系统中不存在科室【%s】！请核对后重新输入。\n\n", input_buf);
                    continue;
                }
                strncpy(dept, input_buf, MAX_NAME_LEN - 1);
                step++;
                break;

            case 3:
                printf("请输入检查费用：");
                get_safe_string("", input_buf, sizeof(input_buf));
                if (my_strcasecmp(input_buf, "Q") == 0)
                {
                    return;
                }
                if (my_strcasecmp(input_buf, "B") == 0)
                {
                    step = 2;
                    printf("\n");
                    continue;
                }
                if (is_blank_string(input_buf))
                {
                    printf("检查费用不能为空，请重新输入\n\n");
                    continue;
                }
                {
                    char* endptr = NULL;
                    price = strtod(input_buf, &endptr);
                    if (endptr == input_buf || *endptr != '\0' || price <= 0)
                    {
                        printf("检查费用必须大于 0，请重新输入\n\n");
                        continue;
                    }
                }
                step++;
                break;

            case 4:
                printf("医保类型：0=自费 1=甲类 2=乙类\n");
                printf("请输入医保类型编号：");
                get_safe_string("", input_buf, sizeof(input_buf));
                if (my_strcasecmp(input_buf, "Q") == 0)
                {
                    return;
                }
                if (my_strcasecmp(input_buf, "B") == 0)
                {
                    step = 3;
                    printf("\n");
                    continue;
                }
                if (is_blank_string(input_buf))
                {
                    printf("医保类型不能为空，请重新输入\n\n");
                    continue;
                }
                {
                    char* endptr = NULL;
                    medicare_type = (int)strtol(input_buf, &endptr, 10);
                    if (endptr == input_buf || *endptr != '\0' || medicare_type < 0 || medicare_type > 2)
                    {
                        printf("医保类型输入非法，请重新输入\n\n");
                        continue;
                    }
                }
                step++;
                break;

            case 5:
                if (g_check_item_list == NULL)
                {
                    printf("提示：检查项目链表尚未初始化，无法新增。\n");
                    return;
                }
                generate_check_item_id(new_id);
                {
                    CheckItemNode* new_node = create_check_item_node(new_id, item_name, dept, price, (MedicareType)medicare_type);
                    if (new_node == NULL)
                    {
                        printf("提示：检查项目节点创建失败。\n");
                        return;
                    }
                    insert_check_item_tail(g_check_item_list, new_node);
                    printf("\n检查项目新增成功！\n");
                    printf("项目编号：%s\n", new_node->item_id);
                    printf("项目名称：%s\n", new_node->item_name);
                    printf("所属科室：%s\n", new_node->dept);
                    printf("检查费用：%.2f\n", new_node->price);
                    printf("医保类型：%s\n",
                           new_node->m_type == MEDICARE_CLASS_A ? "甲类" :
                           new_node->m_type == MEDICARE_CLASS_B ? "乙类" : "自费");
                }
                return;

            default:
                return;
        }
    }
}

void handle_check_item_update(void)
{
    char item_id[MAX_ID_LEN];
    char new_name[MAX_NAME_LEN];
    char new_dept[MAX_NAME_LEN];
    double new_price = -1.0;
    int new_m_type = -1;

    printf("\n================ 修改检查项目信息 ================\n");
    printf("提示：输入 B 返回上一步，输入 Q 返回菜单页\n");
    printf("提示：留空表示不修改该字段\n\n");

    CheckItemNode* target = NULL;
    while (1)
    {
        get_safe_string("请输入检查项目编号：", item_id, MAX_ID_LEN);
        if (my_strcasecmp(item_id, "Q") == 0 || my_strcasecmp(item_id, "B") == 0) return;
        if (is_blank_string(item_id))
        {
            printf("检查项目编号不能为空，请重新输入\n\n");
            continue;
        }
        target = find_check_item_by_id(g_check_item_list, item_id);
        if (target == NULL)
        {
            printf("未找到编号为 %s 的检查项目，修改流程结束\n", item_id);
            return;
        }
        break;
    }

    printf("\n------------- 当前检查项目信息 ------------\n");
    printf("项目编号：%s\n", target->item_id);
    printf("项目名称：%s\n", target->item_name);
    printf("所属科室：%s\n", target->dept);
    printf("检查费用：%.2f\n", target->price);
    switch (target->m_type)
    {
        case MEDICARE_CLASS_A:    printf("医保类型：甲类\n"); break;
        case MEDICARE_CLASS_B:    printf("医保类型：乙类\n"); break;
        case MEDICARE_BASIC:      printf("医保类型：基本医保\n"); break;
        case MEDICARE_GOVERNMENT: printf("医保类型：公务员医保\n"); break;
        case MEDICARE_COMMERCIAL: printf("医保类型：商业保险\n"); break;
        case MEDICARE_NONE:       printf("医保类型：自费\n"); break;
    }
    printf("----------------------------------------\n\n");

    int has_modified = 0;

    get_safe_string("请输入新项目名称：", new_name, MAX_NAME_LEN);
    if (my_strcasecmp(new_name, "Q") == 0 || my_strcasecmp(new_name, "B") == 0) return;
    if (!is_blank_string(new_name) && strcmp(new_name, target->item_name) != 0)
    {
        strncpy(target->item_name, new_name, MAX_NAME_LEN - 1);
        has_modified = 1;
    }

    get_safe_string("请输入新所属科室：", new_dept, MAX_NAME_LEN);
    if (my_strcasecmp(new_dept, "Q") == 0 || my_strcasecmp(new_dept, "B") == 0) return;
    if (!is_blank_string(new_dept) && strcmp(new_dept, target->dept) != 0)
    {
        strncpy(target->dept, new_dept, MAX_NAME_LEN - 1);
        has_modified = 1;
    }

    while (1)
    {
        char price_str[32];
        get_safe_string("请输入新检查费用：", price_str, sizeof(price_str));
        if (my_strcasecmp(price_str, "Q") == 0 || my_strcasecmp(price_str, "B") == 0) return;
        if (is_blank_string(price_str)) break;
        char* endptr = NULL;
        new_price = strtod(price_str, &endptr);
        if (endptr == price_str || *endptr != '\0' || new_price <= 0)
        {
            printf("检查费用必须为大于 0 的数字，请重新输入\n\n");
            continue;
        }
        if (new_price != target->price)
        {
            target->price = new_price;
            has_modified = 1;
        }
        break;
    }

    printf("医保类型：0=自费 1=甲类 2=乙类\n");
    while (1)
    {
        char type_str[32];
        get_safe_string("请输入新医保类型：", type_str, sizeof(type_str));
        if (my_strcasecmp(type_str, "Q") == 0 || my_strcasecmp(type_str, "B") == 0) return;
        if (is_blank_string(type_str)) break;
        char* endptr = NULL;
        new_m_type = (int)strtol(type_str, &endptr, 10);
        if (endptr == type_str || *endptr != '\0' || new_m_type < 0 || new_m_type > 2)
        {
            printf("医保类型必须为 0/1/2，请重新输入\n\n");
            continue;
        }
        if (new_m_type != target->m_type)
        {
            target->m_type = (MedicareType)new_m_type;
            has_modified = 1;
        }
        break;
    }

    if (has_modified == 1)
    {
        printf("\n✅ 检查项目信息修改成功！\n");
    }
    else
    {
        printf("\n未进行任何修改！\n");
    }
}

void handle_check_item_remove(void)
{
    char item_id[MAX_ID_LEN];
    CheckItemNode* target = NULL;
    int confirm;
    char input_buf[64];

    while (1)
    {
        printf("\n================ 下架检查项目 ===============\n");
        printf("提示：输入 B 返回上一步，输入 Q 退出该操作\n\n");

        get_safe_string("请输入要下架的检查项目编号：", input_buf, sizeof(input_buf));
        if (my_strcasecmp(input_buf, "Q") == 0 || my_strcasecmp(input_buf, "B") == 0)
        {
            return;
        }
        if (is_blank_string(input_buf))
        {
            printf("⚠️ 输入不能为空或纯空格，请重新输入！\n");
            system("pause");
            continue;
        }
        strncpy(item_id, input_buf, MAX_ID_LEN - 1);

        target = find_check_item_by_id(g_check_item_list, item_id);
        if (target == NULL)
        {
            printf("提示：未找到对应检查项目，请重新输入。\n\n");
            continue;
        }

        printf("\n当前检查项目信息：\n");
        printf("项目编号：%s\n", target->item_id);
        printf("项目名称：%s\n", target->item_name);
        printf("所属科室：%s\n", target->dept);
        printf("检查费用：%.2f\n", target->price);
        switch (target->m_type)
        {
            case MEDICARE_CLASS_A:    printf("医保类型：甲类\n"); break;
            case MEDICARE_CLASS_B:    printf("医保类型：乙类\n"); break;
            case MEDICARE_NONE:       printf("医保类型：自费\n"); break;
            case MEDICARE_BASIC:      printf("医保类型：基本医保\n"); break;
            case MEDICARE_GOVERNMENT: printf("医保类型：公务员医保\n"); break;
            case MEDICARE_COMMERCIAL: printf("医保类型：商业保险\n"); break;
            default:                  printf("医保类型：未知\n"); break;
        }
        printf("----------------------------------------\n");

        if (is_check_item_referenced_by_active_record(item_id))
        {
            printf("提示：当前检查项目仍被未完成检查记录引用，暂不能下架。\n");
            printf("请先完成相关患者的检查流程后再重试。\n");
            return;
        }

        printf("是否确定下架该检查项目？\n");
        printf("[1] 确定下架\n");
        printf("[0] 取消返回\n");
        printf("提示：输入 B 返回上一步重新输入，输入 Q 直接退出\n");
        
        get_safe_string("请输入操作编号：", input_buf, sizeof(input_buf));
        if (my_strcasecmp(input_buf, "Q") == 0)
        {
            return;
        }
        if (my_strcasecmp(input_buf, "B") == 0)
        {
            printf("\n");
            continue;
        }
        
        confirm = atoi(input_buf);
        if (confirm != 1)
        {
            printf("提示：已取消下架操作，返回重新输入。\n\n");
            continue;
        }

        if (delete_check_item_by_id(g_check_item_list, item_id))
            printf("提示：检查项目下架成功。\n");
        else
            printf("提示：检查项目下架失败。\n");
        return;
    }

    printf("\n当前检查项目信息：\n");
    printf("项目编号：%s\n", target->item_id);
    printf("项目名称：%s\n", target->item_name);
    printf("所属科室：%s\n", target->dept);
    printf("检查费用：%.2f\n", target->price);
    switch (target->m_type)
    {
        case MEDICARE_CLASS_A:    printf("医保类型：甲类\n"); break;
        case MEDICARE_CLASS_B:    printf("医保类型：乙类\n"); break;
        case MEDICARE_BASIC:      printf("医保类型：基本医保\n"); break;
        case MEDICARE_GOVERNMENT: printf("医保类型：公务员医保\n"); break;
        case MEDICARE_COMMERCIAL: printf("医保类型：商业保险\n"); break;
        case MEDICARE_NONE:       printf("医保类型：自费\n"); break;
    }
    printf("----------------------------------------\n");

    if (is_check_item_referenced_by_active_record(item_id))
    {
        printf("提示：当前检查项目仍被未完成检查记录引用，暂不能下架。\n");
        printf("请先完成相关患者的检查流程后再重试。\n");
        return;
    }

    printf("是否确定下架该检查项目？\n");
    printf("[1] 确定下架\n");
    printf("[0] 取消返回\n");
    confirm = get_safe_int("请输入操作编号: ");

    if (confirm != 1)
    {
        printf("提示：已取消下架操作。\n");
        return;
    }

    if (delete_check_item_by_id(g_check_item_list, item_id))
        printf("提示：检查项目下架成功。\n");
    else
        printf("提示：检查项目下架失败。\n");
}

static void set_console_color(int color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

static void reset_console_color(void)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}

static void print_red(const char* text)
{
    set_console_color(12);
    printf("%s", text);
    reset_console_color();
}

static void print_yellow(const char* text)
{
    set_console_color(14);
    printf("%s", text);
    reset_console_color();
}

static void print_green(const char* text)
{
    set_console_color(10);
    printf("%s", text);
    reset_console_color();
}

static void print_cyan(const char* text)
{
    set_console_color(11);
    printf("%s", text);
    reset_console_color();
}

static void print_progress_bar(const char* label, int value, int total, int width)
{
    int completed;
    double ratio = 0.0;
    int i;

    printf("%-12s [", label);

    if (total <= 0)
    {
        completed = 0;
        ratio = 0.0;
    }
    else
    {
        completed = value * width / total;
        if (completed < 0) completed = 0;
        if (completed > width) completed = width;
        ratio = value * 100.0 / total;
    }

    if (completed == width && total > 0)
        set_console_color(12);
    else if (total > 0 && ratio >= 90.0)
        set_console_color(12);
    else if (total > 0 && ratio >= 60.0)
        set_console_color(14);
    else
        set_console_color(10);

    for (i = 0; i < completed; i++)
        printf("█");
    for (; i < width; i++)
        printf("-");

    reset_console_color();

    if (total > 0)
        printf("] %d/%d\n", value, total);
    else
        printf("] 0/0\n");
}

static void print_progress_bar_busy(const char* label, int value, int total, int width, int green_pct, int yellow_pct)
{
    int completed;
    double ratio = 0.0;
    int i;

    printf("%-12s [", label);

    if (total <= 0)
    {
        completed = 0;
        ratio = 0.0;
    }
    else
    {
        completed = value * width / total;
        if (completed < 0) completed = 0;
        if (completed > width) completed = width;
        ratio = value * 100.0 / total;
    }

    if (completed == width && total > 0)
        set_console_color(12);
    else if (total > 0 && ratio >= (double)yellow_pct)
        set_console_color(12);
    else if (total > 0 && ratio >= (double)green_pct)
        set_console_color(14);
    else
        set_console_color(10);

    for (i = 0; i < completed; i++)
        printf("█");
    for (; i < width; i++)
        printf("-");

    reset_console_color();

    if (total > 0)
        printf("] %d/%d\n", value, total);
    else
        printf("] 0/0\n");
}

static void print_progress_bar_cyan(const char* label, int value, int total, int width)
{
    int completed;
    double ratio = 0.0;
    int i;

    printf("%-12s [", label);

    if (total <= 0)
    {
        completed = 0;
        ratio = 0.0;
    }
    else
    {
        completed = value * width / total;
        if (completed < 0) completed = 0;
        if (completed > width) completed = width;
        ratio = value * 100.0 / total;
    }

    if (completed == width && total > 0)
        set_console_color(12);
    else if (total > 0 && ratio >= 90.0)
        set_console_color(12);
    else if (total > 0 && ratio >= 60.0)
        set_console_color(14);
    else
        set_console_color(10);

    for (i = 0; i < completed; i++)
        printf("█");
    for (; i < width; i++)
        printf("-");

    reset_console_color();

    if (total > 0)
        printf("] %d/%d\n", value, total);
    else
        printf("] 0/0\n");
}

 void show_public_medical_big_screen(void)
 {
     int running = 1;

     while (running)
     {
         system("cls");

         /* ===== 1. 门诊候诊概况 ===== */
         int patient_total = 0;
         int pending_count = 0;
         int examining_count = 0;
         int wait_med_count = 0;

         if (g_patient_list != NULL)
         {
             PatientNode* curr = g_patient_list->next;
             while (curr != NULL)
             {
                 patient_total++;
                 switch (curr->status)
                 {
                     case STATUS_PENDING:          pending_count++;   break;
                     case STATUS_EXAMINING:        examining_count++; break;
                     case STATUS_WAIT_MED:         wait_med_count++;  break;
                     default:                                         break;
                 }
                 curr = curr->next;
             }
         }

         /* ===== 2. 预约挂号概况 ===== */
         int appointment_total = 0;
         int checked_in_count = 0;
         int walk_in_count = 0;
         int appointment_count = 0;

         if (g_appointment_list != NULL)
         {
             AppointmentNode* curr = g_appointment_list->next;
             while (curr != NULL)
             {
                 appointment_total++;
                 if (curr->appointment_status == CHECKED_IN)
                     checked_in_count++;
                 if (curr->is_walk_in == 1)
                     walk_in_count++;
                 else
                     appointment_count++;
                 curr = curr->next;
             }
         }

         /* ===== 3. 床位资源 ===== */
         int bed_total = 0;
         int bed_occupied = 0;
         int bed_free = 0;
         double bed_occupancy_rate = 0.0;

         if (g_ward_list != NULL)
         {
             WardNode* curr = g_ward_list->next;
             while (curr != NULL)
             {
                 bed_total++;
                 if (curr->is_occupied != 0)
                     bed_occupied++;
                 else
                     bed_free++;
                 curr = curr->next;
             }
         }
         if (bed_total > 0)
             bed_occupancy_rate = bed_occupied * 100.0 / bed_total;

         /* ===== 4. 医护在岗 ===== */
         int doctor_total = 0;
         int nurse_total = 0;
         int pharmacist_total = 0;
         int doctor_on_duty = 0;
         int nurse_on_duty = 0;
         int pharmacist_on_duty = 0;

         if (g_account_list != NULL)
         {
             AccountNode* curr = g_account_list->next;
             while (curr != NULL)
             {
                 switch (curr->role)
                 {
                     case ROLE_DOCTOR:
                         doctor_total++;
                         if (curr->is_on_duty == 1) doctor_on_duty++;
                         break;
                     case ROLE_NURSE:
                         nurse_total++;
                         if (curr->is_on_duty == 1) nurse_on_duty++;
                         break;
                     case ROLE_PHARMACIST:
                         pharmacist_total++;
                         if (curr->is_on_duty == 1) pharmacist_on_duty++;
                         break;
                     default:
                         break;
                 }
                 curr = curr->next;
             }
         }

         /* ===== 输出界面 ===== */
         printf("\n");
         printf("==============================================================\n");
         printf("         📊 社区智慧医疗管理系统 - 医疗大屏（公共版）\n");
         printf("==============================================================\n");

         /* ---- 门诊候诊概况 ---- */
         {
             printf("\n");
             printf("---------------------  【门诊候诊概况】  ---------------------\n");
             printf("患者总数：%-7d 待诊人数：%-7d\n", patient_total, pending_count);
             printf("检查中：%-9d 待取药：%-9d\n", examining_count, wait_med_count);
         }

         /* ---- 医生候诊队列 ---- */
         {
             printf("\n");
             printf("---------------------  【医生候诊队列】  ---------------------\n");

             if (g_doctor_list != NULL)
             {
                 DoctorNode* doc = g_doctor_list->next;
                 int has_doc = 0;
                 while (doc != NULL)
                 {
                     int queue = 0;
                     if (g_patient_list != NULL)
                     {
                         PatientNode* p = g_patient_list->next;
                         while (p != NULL)
                         {
                             if (p->status == STATUS_PENDING &&
                                 strcmp(p->doctor_id, doc->id) == 0)
                                 queue++;
                             p = p->next;
                         }
                     }

                     char label[24];
                     snprintf(label, sizeof(label), "%s(%s)", doc->name, doc->department);
                     printf("  %-20s ", label);

                     if (doc->is_on_duty == 1)
                     {
                         const char* busy_text;
                         int color;
                         if (queue <= 5)      { busy_text = "空闲";    color = 10; }
                         else if (queue <= 12) { busy_text = "较忙";  color = 14; }
                         else                 { busy_text = "拥挤";    color = 12; }

                         set_console_color(color);
                         printf("%-4s", busy_text);
                         reset_console_color();
                         printf(" [");

                         int bar_width = 20;
                         int filled = (queue > 20) ? 20 : queue;
                         set_console_color(color);
                         int j;
                         for (j = 0; j < filled; j++) printf("█");
                         for (; j < bar_width; j++) printf("-");
                         reset_console_color();
                         printf("] %d人\n", queue);
                     }
                     else
                     {
                         set_console_color(8);
                         printf("不在岗");
                         reset_console_color();
                         printf(" [--------------------] 0人\n");
                     }

                     has_doc = 1;
                     doc = doc->next;
                 }
                 if (!has_doc)
                     printf("  暂无医生数据\n");
             }
             else
             {
                 printf("  暂无医生数据\n");
             }
         }

         /* ---- 预约挂号概况 ---- */
         {
             printf("\n");
             printf("---------------------  【预约挂号概况】  ---------------------\n");
             printf("预约总数：%-7d 已签到/已挂号：%-5d\n",
                    appointment_total, checked_in_count);
             printf("现场号：%-9d 预约号：%-7d\n",
                    walk_in_count, appointment_count);

             print_progress_bar_cyan("已签到/挂号", checked_in_count, appointment_total, 20);
         }

         /* ---- 床位资源 ---- */
         {
             char buf[64];

             printf("\n");
             printf("---------------------  【床位资源】  -------------------------\n");
             printf("床位总数：%-7d 已占用：%-7d 空闲：%-7d\n",
                    bed_total, bed_occupied, bed_free);

             printf("床位占用率：");
             snprintf(buf, sizeof(buf), "%.1f%%", bed_occupancy_rate);
             if (bed_occupancy_rate >= 90.0) print_red(buf);
             else if (bed_occupancy_rate >= 70.0) print_yellow(buf);
             else print_green(buf);
             printf("\n");

             print_progress_bar_busy("床位占用", bed_occupied, bed_total, 20, 70, 90);
         }

         /* ---- 医护在岗 ---- */
         {
             printf("\n");
             printf("---------------------  【医护在岗】  -------------------------\n");
             printf("医生：%d / 在岗 %d\n", doctor_total, doctor_on_duty);
             printf("护士：%d / 在岗 %d\n", nurse_total, nurse_on_duty);
             printf("药师：%d / 在岗 %d\n", pharmacist_total, pharmacist_on_duty);
         }

         printf("\n");
         printf("------------------------------------------------------------\n");
         printf("提示：以上为公共脱敏数据，完整风险统计请管理员登录后查看。\n");
         printf("按 R 刷新大屏，按 0 返回主菜单\n");

         char ch = get_single_char("请输入操作: ");

         if (ch == 'R' || ch == 'r')
         {
             continue;
         }
         else if (ch == '0')
         {
             running = 0;
         }
         else
         {
             printf("\n无效输入，请重新输入。\n");
             system("pause");
         }
     }
 }

 void show_admin_medical_big_screen(void)
{
    int running = 1;

    while (running)
    {
        system("cls");

        /* ===== 1. 门诊运行统计 ===== */
        int patient_total = 0;
        int pending_count = 0;
        int examining_count = 0;
        int recheck_pending_count = 0;
        int unpaid_count = 0;
        int wait_med_count = 0;
        int need_hospitalize_count = 0;
        int hospitalized_count = 0;
        int completed_count = 0;
        int no_show_count = 0;
        int emergency_count = 0;
        int blacklisted_count = 0;
        double emergency_debt_total = 0.0;

        if (g_patient_list != NULL)
        {
            PatientNode* curr = g_patient_list->next;
            while (curr != NULL)
            {
                patient_total++;

                switch (curr->status)
                {
                    case STATUS_PENDING:          pending_count++;          break;
                    case STATUS_EXAMINING:        examining_count++;        break;
                    case STATUS_RECHECK_PENDING:  recheck_pending_count++;  break;
                    case STATUS_UNPAID:           unpaid_count++;           break;
                    case STATUS_WAIT_MED:         wait_med_count++;         break;
                    case STATUS_NEED_HOSPITALIZE: need_hospitalize_count++;  break;
                    case STATUS_HOSPITALIZED:     hospitalized_count++;     break;
                    case STATUS_COMPLETED:        completed_count++;        break;
                    case STATUS_NO_SHOW:          no_show_count++;          break;
                    default:                                                break;
                }

                if (curr->is_emergency == 1)
                    emergency_count++;
                if (curr->is_blacklisted != 0)
                    blacklisted_count++;
                emergency_debt_total += curr->emergency_debt;

                curr = curr->next;
            }
        }

        /* ===== 2. 预约挂号统计 ===== */
        int appointment_total = 0;
        int reserved_count = 0;
        int checked_in_count = 0;
        int cancelled_count = 0;
        int missed_count = 0;
        int walk_in_count = 0;
        int appointment_count = 0;
        int fee_paid_count = 0;
        int fee_unpaid_count = 0;

        if (g_appointment_list != NULL)
        {
            AppointmentNode* curr = g_appointment_list->next;
            while (curr != NULL)
            {
                appointment_total++;

                switch (curr->appointment_status)
                {
                    case RESERVED:    reserved_count++;    break;
                    case CHECKED_IN:  checked_in_count++;  break;
                    case CANCELLED:   cancelled_count++;   break;
                    case MISSED:      missed_count++;      break;
                    default:                               break;
                }

                if (curr->is_walk_in == 1)
                    walk_in_count++;
                else
                    appointment_count++;

                if (curr->fee_paid == 1)
                    fee_paid_count++;
                else
                    fee_unpaid_count++;

                curr = curr->next;
            }
        }

        /* ===== 3. 医护药人员统计 ===== */
        int admin_count = 0;
        int doctor_total = 0;
        int nurse_total = 0;
        int pharmacist_total = 0;
        int doctor_on_duty = 0;
        int nurse_on_duty = 0;
        int pharmacist_on_duty = 0;

        if (g_account_list != NULL)
        {
            AccountNode* curr = g_account_list->next;
            while (curr != NULL)
            {
                switch (curr->role)
                {
                    case ROLE_ADMIN:
                        admin_count++;
                        break;
                    case ROLE_DOCTOR:
                        doctor_total++;
                        if (curr->is_on_duty == 1)
                            doctor_on_duty++;
                        break;
                    case ROLE_NURSE:
                        nurse_total++;
                        if (curr->is_on_duty == 1)
                            nurse_on_duty++;
                        break;
                    case ROLE_PHARMACIST:
                        pharmacist_total++;
                        if (curr->is_on_duty == 1)
                            pharmacist_on_duty++;
                        break;
                    default:
                        break;
                }
                curr = curr->next;
            }
        }

        /* ===== 4. 床位与住院统计 ===== */
        int bed_total = 0;
        int bed_occupied = 0;
        int bed_free = 0;
        double bed_occupancy_rate = 0.0;

        int inpatient_total = 0;
        int inpatient_active = 0;
        int inpatient_discharged = 0;
        int deposit_low_count = 0;
        double deposit_balance_total = 0.0;

        if (g_ward_list != NULL)
        {
            WardNode* curr = g_ward_list->next;
            while (curr != NULL)
            {
                bed_total++;
                if (curr->is_occupied != 0)
                    bed_occupied++;
                else
                    bed_free++;
                curr = curr->next;
            }
        }
        if (bed_total > 0)
            bed_occupancy_rate = bed_occupied * 100.0 / bed_total;

        if (g_inpatient_list != NULL)
        {
            InpatientRecord* curr = g_inpatient_list->next;
            while (curr != NULL)
            {
                inpatient_total++;
                if (curr->is_active == 1)
                {
                    inpatient_active++;
                    deposit_balance_total += curr->deposit_balance;
                    if (curr->deposit_balance < 1000.0)
                        deposit_low_count++;
                }
                else
                {
                    inpatient_discharged++;
                }
                curr = curr->next;
            }
        }

        /* ===== 5. 药品库存统计 ===== */
        int medicine_kind_total = 0;
        int medicine_stock_total = 0;
        int low_stock_count = 0;
        int expiring_count = 0;
        int expired_count = 0;

        time_t now = time(NULL);
        struct tm now_tm;
        localtime_s(&now_tm, &now);
        char today_str[11];
        snprintf(today_str, sizeof(today_str), "%d-%02d-%02d",
                 now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday);

        if (g_medicine_list != NULL)
        {
            MedicineNode* curr = g_medicine_list->next;
            while (curr != NULL)
            {
                medicine_kind_total++;
                medicine_stock_total += curr->stock;

                if (curr->stock < 10)
                    low_stock_count++;

                int days_left = days_between_dates(today_str, curr->expiry_date);
                if (days_left < 0)
                    expired_count++;
                else if (days_left >= 0 && days_left <= 30)
                    expiring_count++;

                curr = curr->next;
            }
        }

        /* ===== 6. 检查与服务统计 ===== */
        int check_total = 0;
        int check_completed = 0;
        int check_uncompleted = 0;
        int check_paid = 0;
        int check_unpaid = 0;

        int alert_total = 0;
        int complaint_total = 0;
        int complaint_pending = 0;
        int complaint_replied = 0;
        int log_total = 0;

        if (g_check_record_list != NULL)
        {
            CheckRecordNode* curr = g_check_record_list->next;
            while (curr != NULL)
            {
                check_total++;
                if (curr->is_completed == 1)
                    check_completed++;
                else
                    check_uncompleted++;
                if (curr->is_paid == 1)
                    check_paid++;
                else
                    check_unpaid++;
                curr = curr->next;
            }
        }

        if (g_alert_list != NULL)
        {
            AlertNode* curr = g_alert_list->next;
            while (curr != NULL)
            {
                alert_total++;
                curr = curr->next;
            }
        }

        if (g_complaint_list != NULL)
        {
            ComplaintNode* curr = g_complaint_list->next;
            while (curr != NULL)
            {
                complaint_total++;
                if (curr->status == 0)
                    complaint_pending++;
                else if (curr->status == 1)
                    complaint_replied++;
                curr = curr->next;
            }
        }

        if (g_log_list != NULL)
        {
            LogNode* curr = g_log_list->next;
            while (curr != NULL)
            {
                log_total++;
                curr = curr->next;
            }
        }

        /* ===== 系统状态判断 ===== */
        int is_high_risk = (expired_count > 0 || bed_occupancy_rate >= 90.0 || blacklisted_count > 0);
        int is_warning = (!is_high_risk && (alert_total > 0 || low_stock_count > 0 ||
                          deposit_low_count > 0 || complaint_pending > 0));

        /* ===== 输出界面 ===== */
        printf("\n");
        printf("==============================================================\n");
        printf("             📊 社区智慧医疗管理系统 - 医疗大屏\n");
        printf("==============================================================\n");

        printf("  系统状态：");
        if (is_high_risk)
            print_red("🔴 高风险，请立即处理");
        else if (is_warning)
            print_yellow("🟡 注意运行，存在待处理事项");
        else
            print_green("🟢 运行正常");
        printf("\n");

        /* ===== 风险汇总卡片 ===== */
        {
            int has_red_risk = (expired_count > 0 || bed_occupancy_rate >= 90.0 || blacklisted_count > 0);
            int has_yellow_risk = (low_stock_count > 0 || expiring_count > 0 || 
                                  deposit_low_count > 0 || complaint_pending > 0 || 
                                  alert_total > 0 || emergency_debt_total > 0);
            int has_risk = has_red_risk || has_yellow_risk;

            printf("\n");
            printf("┌──────────────────────────────────────────────────────────────┐\n");
            printf("│                      ⚠️  风险汇总                            │\n");
            printf("├──────────────────────────────────────────────────────────────┤\n");

            if (!has_risk)
            {
                printf("│  ");
                print_green("✅ 当前无明显风险，系统运行正常");
                printf("                                      │\n");
            }
            else
            {
                if (has_red_risk)
                {
                    printf("│  ");
                    print_red("⚠️ 当前存在以下高风险：");
                    printf("                                    │\n");
                }
                else
                {
                    printf("│  ");
                    print_yellow("⚠️ 当前存在以下风险：");
                    printf("                                      │\n");
                }

                char buf[32];

                if (low_stock_count > 0)
                {
                    printf("│  ");
                    print_yellow("- 低库存药品：");
                    snprintf(buf, sizeof(buf), "%d", low_stock_count);
                    print_yellow(buf);
                    print_yellow(" 种");
                    printf("                                      │\n");
                }

                if (expiring_count > 0)
                {
                    printf("│  ");
                    print_yellow("- 近效期药品：");
                    snprintf(buf, sizeof(buf), "%d", expiring_count);
                    print_yellow(buf);
                    print_yellow(" 种");
                    printf("                                      │\n");
                }

                if (expired_count > 0)
                {
                    printf("│  ");
                    print_red("- 已过期药品：");
                    snprintf(buf, sizeof(buf), "%d", expired_count);
                    print_red(buf);
                    print_red(" 种");
                    printf("                                      │\n");
                }

                if (deposit_low_count > 0)
                {
                    printf("│  ");
                    print_yellow("- 押金不足患者：");
                    snprintf(buf, sizeof(buf), "%d", deposit_low_count);
                    print_yellow(buf);
                    print_yellow(" 人");
                    printf("                                    │\n");
                }

                if (complaint_pending > 0)
                {
                    printf("│  ");
                    print_yellow("- 待处理投诉：");
                    snprintf(buf, sizeof(buf), "%d", complaint_pending);
                    print_yellow(buf);
                    print_yellow(" 条");
                    printf("                                      │\n");
                }

                if (alert_total > 0)
                {
                    printf("│  ");
                    print_yellow("- 系统预警：");
                    snprintf(buf, sizeof(buf), "%d", alert_total);
                    print_yellow(buf);
                    print_yellow(" 条");
                    printf("                                        │\n");
                }

                if (emergency_debt_total > 0)
                {
                    printf("│  ");
                    print_yellow("- 急诊欠费：");
                    snprintf(buf, sizeof(buf), "%.2f", emergency_debt_total);
                    print_yellow(buf);
                    print_yellow(" 元");
                    printf("                                      │\n");
                }

                if (bed_occupancy_rate >= 90.0)
                {
                    printf("│  ");
                    print_red("- 床位占用率：");
                    snprintf(buf, sizeof(buf), "%.1f", bed_occupancy_rate);
                    print_red(buf);
                    print_red("%");
                    printf("                                          │\n");
                }
            }

            printf("└──────────────────────────────────────────────────────────────┘\n");
        }

        /* ===== 输出：门诊运行 ===== */
        {
            char buf[64];

            printf("\n");
            printf("--------------------------  【门诊运行】  -------------------------\n");

            printf("患者总数：%-7d 待诊：%-7d 检查中：%-7d\n",
                   patient_total, pending_count, examining_count);
            printf("复诊待处理：%-5d 待缴费：", recheck_pending_count);
            snprintf(buf, sizeof(buf), "%-7d", unpaid_count);
            if (unpaid_count > 0) print_red(buf); else print_green(buf);
            printf(" 待取药：%-7d\n", wait_med_count);
            printf("需住院登记：%-5d 住院中：%-7d 就诊结束：%-5d\n",
                   need_hospitalize_count, hospitalized_count, completed_count);
            printf("过号作废：%-7d 急诊患者：%-5d 黑名单：",
                   no_show_count, emergency_count);
            snprintf(buf, sizeof(buf), "%-7d", blacklisted_count);
            if (blacklisted_count > 0) print_red(buf); else print_green(buf);
            printf("\n");

            printf("急诊欠费总额：");
            snprintf(buf, sizeof(buf), "%.2f", emergency_debt_total);
            if (emergency_debt_total > 0) print_yellow(buf); else print_green(buf);
            printf(" 元\n");

            print_progress_bar("待诊患者", pending_count, patient_total, 20);
            print_progress_bar("待缴费患者", unpaid_count, patient_total, 20);
            print_progress_bar("待取药患者", wait_med_count, patient_total, 20);
            print_progress_bar("急诊患者", emergency_count, patient_total, 20);
        }

        /* ===== 输出：预约挂号 ===== */
        {
            printf("\n");
            printf("------------------------  【预约挂号】  -------------------------\n");
            printf("预约总数：%-7d 已预约：%-7d 已签到/已挂号：%-5d\n",
                   appointment_total, reserved_count, checked_in_count);
            printf("已取消：%-9d 已过号：%-7d\n",
                   cancelled_count, missed_count);
            printf("现场号：%-9d 预约号：%-7d\n",
                   walk_in_count, appointment_count);
            printf("已缴挂号费：%-5d 未缴挂号费：%-5d\n",
                   fee_paid_count, fee_unpaid_count);

            print_progress_bar("已签到/挂号", checked_in_count, appointment_total, 20);
            print_progress_bar("已取消预约", cancelled_count, appointment_total, 20);
            print_progress_bar("已过号预约", missed_count, appointment_total, 20);
        }

        /* ===== 输出：医护药人员 ===== */
        {
            printf("\n");
            printf("-----------------------  【医护药人员】  ------------------------\n");
            printf("管理员：%d\n", admin_count);
            printf("医生：%d / 在岗 %d\n", doctor_total, doctor_on_duty);
            printf("护士：%d / 在岗 %d\n", nurse_total, nurse_on_duty);
            printf("药师：%d / 在岗 %d\n", pharmacist_total, pharmacist_on_duty);
        }

        /* ===== 输出：床位住院 ===== */
        {
            char buf[64];

            printf("\n");
            printf("------------------------  【床位住院】  -------------------------\n");
            printf("床位总数：%-7d 已占用：%-7d 空闲：%-7d\n",
                   bed_total, bed_occupied, bed_free);

            printf("床位占用率：");
            snprintf(buf, sizeof(buf), "%.1f%%", bed_occupancy_rate);
            if (bed_occupancy_rate >= 90.0) print_red(buf); else print_green(buf);
            printf("\n");

            printf("住院记录：%-7d 当前住院：%-5d 已出院：%-7d\n",
                   inpatient_total, inpatient_active, inpatient_discharged);

            printf("押金不足：");
            snprintf(buf, sizeof(buf), "%-7d", deposit_low_count);
            if (deposit_low_count > 0) print_yellow(buf); else print_green(buf);
            printf(" 住院押金总余额：%.2f 元\n", deposit_balance_total);

            print_progress_bar("床位占用", bed_occupied, bed_total, 20);
        }

        /* ===== 输出：药品库存 ===== */
        {
            char buf[64];

            printf("\n");
            printf("------------------------  【药品库存】  -------------------------\n");
            printf("药品种类：%-7d 库存总量：%-7d\n",
                   medicine_kind_total, medicine_stock_total);

            printf("低库存药品：");
            snprintf(buf, sizeof(buf), "%-5d", low_stock_count);
            if (low_stock_count > 0) print_yellow(buf); else print_green(buf);

            printf(" 近效期药品：");
            snprintf(buf, sizeof(buf), "%-5d", expiring_count);
            if (expiring_count > 0) print_yellow(buf); else print_green(buf);

            printf(" 已过期药品：");
            snprintf(buf, sizeof(buf), "%-5d", expired_count);
            if (expired_count > 0) print_red(buf); else print_green(buf);
            printf("\n");
        }

        /* ===== 输出：检查与服务 ===== */
        {
            char buf[64];

            printf("\n");
            printf("-----------------------  【检查与服务】  ------------------------\n");

            printf("检查记录：%-7d 已完成：%-7d 未完成：%-7d\n",
                   check_total, check_completed, check_uncompleted);

            printf("已缴费检查：%-5d 未缴费检查：",
                   check_paid);
            snprintf(buf, sizeof(buf), "%-5d", check_unpaid);
            if (check_unpaid > 0) print_red(buf); else print_green(buf);
            printf("\n");

            printf("系统预警：");
            snprintf(buf, sizeof(buf), "%-7d", alert_total);
            if (alert_total > 0) print_yellow(buf); else print_green(buf);

            printf(" 投诉总数：%-5d 待处理投诉：", complaint_total);
            snprintf(buf, sizeof(buf), "%-5d", complaint_pending);
            if (complaint_pending > 0) print_yellow(buf); else print_green(buf);
            printf("\n");

            printf("已回复投诉：%-5d 操作日志：%-7d\n",
                   complaint_replied, log_total);
        }

        printf("\n");
        printf("------------------------------------------------------------\n");
        printf("系统提示：数据来自当前内存链表，保存退出时会写入本地 txt 文件。\n");
        printf("按 R 刷新大屏，按 0 返回主菜单：\n");
        printf("==============================================================\n");

        char ch = get_single_char("👉 请输入操作: ");

        if (ch == 'R' || ch == 'r')
        {
            continue;
        }
        else if (ch == '0')
        {
            running = 0;
        }
        else
        {
            printf("\n❌ 无效输入，请重新输入。\n");
            system("pause");
        }
    }
}

// 确保报表目录存在
static void ensure_report_dir(void)
{
    _mkdir("report");
}

// 获取当前时间字符串
static void get_current_time_string(char* buffer, int size)
{
    time_t now = time(NULL);
    struct tm local_tm;
    localtime_s(&local_tm, &now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &local_tm);
}

// 生成带时间戳的报表文件名
static void build_report_filename(char* buffer, int size, const char* prefix)
{
    time_t now = time(NULL);
    struct tm local_tm;
    localtime_s(&local_tm, &now);
    snprintf(buffer, size, "report/%s_%04d%02d%02d_%02d%02d%02d.txt",
             prefix,
             local_tm.tm_year + 1900,
             local_tm.tm_mon + 1,
             local_tm.tm_mday,
             local_tm.tm_hour,
             local_tm.tm_min,
             local_tm.tm_sec);
}

static void write_utf8_bom(FILE* fp)
{
    if (fp != NULL)
    {
        // 分别写入三个字节，避免 \x 贪婪解析问题
        fprintf(fp, "%c%c%c", 0xEF, 0xBB, 0xBF);
    }
}

// 安全截取 UTF-8 字符串（不截断多字节字符）
static void safe_utf8_truncate(char* dest, const char* src, int max_bytes)
{
    if (dest == NULL || src == NULL || max_bytes <= 0)
    {
        if (dest != NULL) dest[0] = '\0';
        return;
    }
    
    int i = 0;
    while (i < max_bytes && src[i] != '\0')
    {
        // UTF-8 编码规则：
        // 单字节: 0xxxxxxx (0x00-0x7F)
        // 双字节: 110xxxxx 10xxxxxx (0xC2-0xDF)
        // 三字节: 1110xxxx 10xxxxxx 10xxxxxx (0xE0-0xEF)
        
        if ((src[i] & 0xE0) == 0xE0 && max_bytes - i >= 3)
        {
            // 三字节字符（中文）
            dest[i] = src[i];
            dest[i+1] = src[i+1];
            dest[i+2] = src[i+2];
            i += 3;
        }
        else if ((src[i] & 0xC0) == 0xC0 && max_bytes - i >= 2)
        {
            // 双字节字符
            dest[i] = src[i];
            dest[i+1] = src[i+1];
            i += 2;
        }
        else if ((src[i] & 0x80) == 0x00)
        {
            // 单字节字符
            dest[i] = src[i];
            i++;
        }
        else
        {
            // 无法完整包含当前多字节字符，停止
            break;
        }
    }
    dest[i] = '\0';
}

// 写报表头
static void write_report_header(FILE* fp, const char* title)
{
    char time_str[32];
    get_current_time_string(time_str, sizeof(time_str));
    
    fprintf(fp, "============================================================\n");
    fprintf(fp, "社区智慧医疗管理系统 - %s\n", title);
    fprintf(fp, "============================================================\n");
    fprintf(fp, "导出时间：%s\n\n", time_str);
}

// 日期处理辅助函数 - 解析 YYYY-MM-DD 格式
static int parse_ymd(const char* date_str, int* y, int* m, int* d)
{
    if (date_str == NULL || strlen(date_str) < 10)
        return 0;
    
    return sscanf(date_str, "%d-%d-%d", y, m, d) == 3;
}

// 日期处理辅助函数 - 计算日期对应的天数
static int date_to_days(int y, int m, int d)
{
    if (m < 3) {
        y--;
        m += 12;
    }
    return 365 * y + y / 4 - y / 100 + y / 400 + (153 * m - 457) / 5 + d - 306;
}

// 日期处理辅助函数 - 计算与今天的天数差
static int days_from_today(const char* date_str)
{
    int y, m, d;
    if (!parse_ymd(date_str, &y, &m, &d))
        return -9999;
    
    time_t now = time(NULL);
    struct tm local_tm;
    localtime_s(&local_tm, &now);
    
    int today_days = date_to_days(local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday);
    int target_days = date_to_days(y, m, d);
    
    return target_days - today_days;
}

// 患者就诊统计报表
static int export_patient_report(void)
{
    char filename[128];
    build_report_filename(filename, sizeof(filename), "patient_report");
    
    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("❌ 报表导出失败，请检查 report 目录权限或文件是否被占用。\n");
        return 0;
    }
    write_utf8_bom(fp);
    
    // 写入报表头
    write_report_header(fp, "患者就诊统计报表");
    
    // 统计变量
    int patient_total = 0;
    int pending_count = 0;
    int examining_count = 0;
    int recheck_pending_count = 0;
    int unpaid_count = 0;
    int wait_med_count = 0;
    int need_hospitalize_count = 0;
    int hospitalized_count = 0;
    int completed_count = 0;
    int no_show_count = 0;
    int emergency_count = 0;
    int blacklisted_count = 0;
    double emergency_debt_total = 0.0;
    
    // 遍历患者链表
    if (g_patient_list != NULL)
    {
        PatientNode* curr = g_patient_list->next;
        while (curr != NULL)
        {
            patient_total++;
            
            switch (curr->status)
            {
                case STATUS_PENDING:
                    pending_count++;
                    break;
                case STATUS_EXAMINING:
                    examining_count++;
                    break;
                case STATUS_RECHECK_PENDING:
                    recheck_pending_count++;
                    break;
                case STATUS_UNPAID:
                    unpaid_count++;
                    break;
                case STATUS_WAIT_MED:
                    wait_med_count++;
                    break;
                case STATUS_NEED_HOSPITALIZE:
                    need_hospitalize_count++;
                    break;
                case STATUS_HOSPITALIZED:
                    hospitalized_count++;
                    break;
                case STATUS_COMPLETED:
                    completed_count++;
                    break;
                case STATUS_NO_SHOW:
                    no_show_count++;
                    break;
            }
            
            if (curr->is_emergency == 1)
                emergency_count++;
            if (curr->is_blacklisted != 0)
                blacklisted_count++;
            emergency_debt_total += curr->emergency_debt;
            
            curr = curr->next;
        }
    }
    
    // 写入统计内容
    fprintf(fp, "【统计汇总】\n");
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "患者总数：%d\n", patient_total);
    fprintf(fp, "待诊人数：%d\n", pending_count);
    fprintf(fp, "检查中人数：%d\n", examining_count);
    fprintf(fp, "检查后待复诊人数：%d\n", recheck_pending_count);
    fprintf(fp, "待缴费人数：%d\n", unpaid_count);
    fprintf(fp, "待取药人数：%d\n", wait_med_count);
    fprintf(fp, "需住院待登记人数：%d\n", need_hospitalize_count);
    fprintf(fp, "住院中人数：%d\n", hospitalized_count);
    fprintf(fp, "就诊结束人数：%d\n", completed_count);
    fprintf(fp, "过号作废人数：%d\n", no_show_count);
    fprintf(fp, "急诊患者人数：%d\n", emergency_count);
    fprintf(fp, "黑名单患者人数：%d\n", blacklisted_count);
    fprintf(fp, "急诊欠费总金额：%.2f\n", emergency_debt_total);
    fprintf(fp, "------------------------------------------------------------\n\n");
    
    // 写入明细内容
    fprintf(fp, "【患者明细】\n");
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "%-12s | %-8s | %-4s | %-4s | %-12s | %-10s | %-10s | %-8s\n",
            "患者编号", "姓名", "性别", "年龄", "科室", "医生编号", "当前状态", "余额");
    fprintf(fp, "------------------------------------------------------------\n");
    
    if (g_patient_list != NULL)
    {
        PatientNode* curr = g_patient_list->next;
        while (curr != NULL)
        {
            fprintf(fp, "%-12s | %-8s | %-4s | %-4d | %-12s | %-10s | %-10s | %-8.2f\n",
                    curr->id,
                    curr->name,
                    curr->gender,
                    curr->age,
                    strlen(curr->target_dept) > 0 ? curr->target_dept : "无",
                    strlen(curr->doctor_id) > 0 ? curr->doctor_id : "无",
                    get_patient_status_text(curr->status),
                    curr->balance);
            curr = curr->next;
        }
    }
    
    fprintf(fp, "------------------------------------------------------------\n");
    fclose(fp);
    
    printf("✅ 报表导出成功：%s\n", filename);
    return 1;
}

// 预约挂号统计报表
static int export_appointment_report(void)
{
    char filename[128];
    build_report_filename(filename, sizeof(filename), "appointment_report");
    
    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("❌ 报表导出失败，请检查 report 目录权限或文件是否被占用。\n");
        return 0;
    }
    write_utf8_bom(fp);
    
    // 写入报表头
    write_report_header(fp, "预约挂号统计报表");
    
    // 统计变量
    int appointment_total = 0;
    int reserved_count = 0;
    int checked_in_count = 0;
    int cancelled_count = 0;
    int missed_count = 0;
    int walk_in_count = 0;
    int prebook_count = 0;
    int fee_paid_count = 0;
    int fee_unpaid_count = 0;
    
    // 遍历预约链表
    if (g_appointment_list != NULL)
    {
        AppointmentNode* curr = g_appointment_list->next;
        while (curr != NULL)
        {
            appointment_total++;
            
            switch (curr->appointment_status)
            {
                case RESERVED:
                    reserved_count++;
                    break;
                case CHECKED_IN:
                    checked_in_count++;
                    break;
                case CANCELLED:
                    cancelled_count++;
                    break;
                case MISSED:
                    missed_count++;
                    break;
            }
            
            if (curr->is_walk_in == 1)
                walk_in_count++;
            else
                prebook_count++;
            
            if (curr->fee_paid == 1)
                fee_paid_count++;
            else
                fee_unpaid_count++;
            
            curr = curr->next;
        }
    }
    
    // 写入统计内容
    fprintf(fp, "【统计汇总】\n");
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "预约总数：%d\n", appointment_total);
    fprintf(fp, "已预约：%d\n", reserved_count);
    fprintf(fp, "已签到/已挂号：%d\n", checked_in_count);
    fprintf(fp, "已取消：%d\n", cancelled_count);
    fprintf(fp, "已过号：%d\n", missed_count);
    fprintf(fp, "现场号数量：%d\n", walk_in_count);
    fprintf(fp, "预约号数量：%d\n", prebook_count);
    fprintf(fp, "已缴挂号费数量：%d\n", fee_paid_count);
    fprintf(fp, "未缴挂号费数量：%d\n", fee_unpaid_count);
    fprintf(fp, "------------------------------------------------------------\n\n");
    
    // 写入明细内容
    fprintf(fp, "【预约明细】\n");
    fprintf(fp, "----------------------------------------------------------------------------------------------------\n");
    fprintf(fp, "%-12s | %-10s | %-12s | %-10s | %-10s | %-10s | %-8s | %-6s | %-8s\n",
            "预约编号", "患者编号", "预约日期", "预约时段", "医生编号", "科室", "状态", "来源", "挂号费状态");
    fprintf(fp, "----------------------------------------------------------------------------------------------------\n");
    
    if (g_appointment_list != NULL)
    {
        AppointmentNode* curr = g_appointment_list->next;
        while (curr != NULL)
        {
            const char* source = curr->is_walk_in == 1 ? "现场号" : "预约号";
            const char* fee_status = curr->fee_paid == 1 ? "已缴费" : "未缴费";
            
            fprintf(fp, "%-12s | %-10s | %-12s | %-10s | %-10s | %-10s | %-8s | %-6s | %-8s\n",
                    strlen(curr->appointment_id) > 0 ? curr->appointment_id : curr->id,
                    curr->patient_id,
                    strlen(curr->appointment_date) > 0 ? curr->appointment_date : "无",
                    strlen(curr->appointment_slot) > 0 ? curr->appointment_slot : "无",
                    strlen(curr->doctor_id) > 0 ? curr->doctor_id : "无",
                    strlen(curr->department) > 0 ? curr->department : (strlen(curr->appoint_dept) > 0 ? curr->appoint_dept : "无"),
                    get_appointment_status_text(curr->appointment_status),
                    source,
                    fee_status);
            curr = curr->next;
        }
    }
    
    fprintf(fp, "----------------------------------------------------------------------------------------------------\n");
    fclose(fp);
    
    printf("✅ 报表导出成功：%s\n", filename);
    return 1;
}

// 药品库存预警报表
static int export_medicine_report(void)
{
    char filename[128];
    build_report_filename(filename, sizeof(filename), "medicine_report");
    
    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("❌ 报表导出失败，请检查 report 目录权限或文件是否被占用。\n");
        return 0;
    }
    write_utf8_bom(fp);
    
    // 写入报表头
    write_report_header(fp, "药品库存预警报表");
    
    // 统计变量
    int medicine_total = 0;
    int stock_total = 0;
    int low_stock_count = 0;
    int expiring_count = 0;
    int expired_count = 0;
    
    // 遍历药品链表
    if (g_medicine_list != NULL)
    {
        MedicineNode* curr = g_medicine_list->next;
        while (curr != NULL)
        {
            medicine_total++;
            stock_total += curr->stock;
            
            if (curr->stock < 10)
                low_stock_count++;
            
            int days = days_from_today(curr->expiry_date);
            if (days < 0)
                expired_count++;
            else if (days >= 0 && days <= 30)
                expiring_count++;
            
            curr = curr->next;
        }
    }
    
    // 写入统计内容
    fprintf(fp, "【统计汇总】\n");
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "药品种类总数：%d\n", medicine_total);
    fprintf(fp, "库存总量：%d\n", stock_total);
    fprintf(fp, "低库存药品数量：%d\n", low_stock_count);
    fprintf(fp, "近效期药品数量：%d\n", expiring_count);
    fprintf(fp, "已过期药品数量：%d\n", expired_count);
    fprintf(fp, "------------------------------------------------------------\n\n");
    
    // 写入低库存药品明细
    fprintf(fp, "【低库存药品明细】\n");
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "%-12s | %-16s | %-12s | %-16s | %-8s | %-6s | %-12s\n",
            "药品编号", "药品名称", "别名", "通用名", "单价", "库存", "有效期");
    fprintf(fp, "------------------------------------------------------------\n");
    
    if (g_medicine_list != NULL)
    {
        MedicineNode* curr = g_medicine_list->next;
        while (curr != NULL)
        {
            if (curr->stock < 10)
            {
                fprintf(fp, "%-12s | %-16s | %-12s | %-16s | %-8.2f | %-6d | %-12s\n",
                        curr->id,
                        curr->name,
                        strlen(curr->alias) > 0 ? curr->alias : "无",
                        strlen(curr->generic_name) > 0 ? curr->generic_name : "无",
                        curr->price,
                        curr->stock,
                        strlen(curr->expiry_date) > 0 ? curr->expiry_date : "无");
            }
            curr = curr->next;
        }
    }
    fprintf(fp, "------------------------------------------------------------\n\n");
    
    // 写入近效期药品明细
    fprintf(fp, "【近效期药品明细】\n");
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "%-12s | %-16s | %-12s | %-16s | %-8s | %-6s | %-12s\n",
            "药品编号", "药品名称", "别名", "通用名", "单价", "库存", "有效期");
    fprintf(fp, "------------------------------------------------------------\n");
    
    if (g_medicine_list != NULL)
    {
        MedicineNode* curr = g_medicine_list->next;
        while (curr != NULL)
        {
            int days = days_from_today(curr->expiry_date);
            if (days >= 0 && days <= 30)
            {
                fprintf(fp, "%-12s | %-16s | %-12s | %-16s | %-8.2f | %-6d | %-12s\n",
                        curr->id,
                        curr->name,
                        strlen(curr->alias) > 0 ? curr->alias : "无",
                        strlen(curr->generic_name) > 0 ? curr->generic_name : "无",
                        curr->price,
                        curr->stock,
                        strlen(curr->expiry_date) > 0 ? curr->expiry_date : "无");
            }
            curr = curr->next;
        }
    }
    fprintf(fp, "------------------------------------------------------------\n\n");
    
    // 写入已过期药品明细
    fprintf(fp, "【已过期药品明细】\n");
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "%-12s | %-16s | %-12s | %-16s | %-8s | %-6s | %-12s\n",
            "药品编号", "药品名称", "别名", "通用名", "单价", "库存", "有效期");
    fprintf(fp, "------------------------------------------------------------\n");
    
    if (g_medicine_list != NULL)
    {
        MedicineNode* curr = g_medicine_list->next;
        while (curr != NULL)
        {
            int days = days_from_today(curr->expiry_date);
            if (days < 0)
            {
                fprintf(fp, "%-12s | %-16s | %-12s | %-16s | %-8.2f | %-6d | %-12s\n",
                        curr->id,
                        curr->name,
                        strlen(curr->alias) > 0 ? curr->alias : "无",
                        strlen(curr->generic_name) > 0 ? curr->generic_name : "无",
                        curr->price,
                        curr->stock,
                        strlen(curr->expiry_date) > 0 ? curr->expiry_date : "无");
            }
            curr = curr->next;
        }
    }
    fprintf(fp, "------------------------------------------------------------\n");
    fclose(fp);
    
    printf("✅ 报表导出成功：%s\n", filename);
    return 1;
}

// 床位住院统计报表
static int export_ward_inpatient_report(void)
{
    char filename[128];
    build_report_filename(filename, sizeof(filename), "ward_inpatient_report");
    
    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("❌ 报表导出失败，请检查 report 目录权限或文件是否被占用。\n");
        return 0;
    }
    write_utf8_bom(fp);
    
    // 写入报表头
    write_report_header(fp, "床位住院统计报表");
    
    // 床位统计变量
    int total_beds = 0;
    int occupied_beds = 0;
    int free_beds = 0;
    double occupancy_rate = 0.0;
    
    // 遍历床位链表
    if (g_ward_list != NULL)
    {
        WardNode* curr = g_ward_list->next;
        while (curr != NULL)
        {
            total_beds++;
            if (curr->is_occupied != 0)
                occupied_beds++;
            else
                free_beds++;
            curr = curr->next;
        }
    }
    
    if (total_beds > 0)
        occupancy_rate = occupied_beds * 100.0 / total_beds;
    
    // 住院统计变量
    int inpatient_total = 0;
    int active_inpatient = 0;
    int discharged_inpatient = 0;
    int deposit_warning_count = 0;
    double deposit_total = 0.0;
    
    // 遍历住院记录链表
    if (g_inpatient_list != NULL)
    {
        InpatientRecord* curr = g_inpatient_list->next;
        while (curr != NULL)
        {
            inpatient_total++;
            if (curr->is_active == 1)
            {
                active_inpatient++;
                deposit_total += curr->deposit_balance;
                if (curr->deposit_balance < 1000.0)
                    deposit_warning_count++;
            }
            else
            {
                discharged_inpatient++;
            }
            curr = curr->next;
        }
    }
    
    // 写入床位统计
    fprintf(fp, "【床位统计】\n");
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "床位总数：%d\n", total_beds);
    fprintf(fp, "已占用床位：%d\n", occupied_beds);
    fprintf(fp, "空闲床位：%d\n", free_beds);
    fprintf(fp, "床位占用率：%.1f%%\n", occupancy_rate);
    fprintf(fp, "------------------------------------------------------------\n\n");
    
    // 写入住院统计
    fprintf(fp, "【住院统计】\n");
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "住院记录总数：%d\n", inpatient_total);
    fprintf(fp, "当前住院中记录：%d\n", active_inpatient);
    fprintf(fp, "已出院记录：%d\n", discharged_inpatient);
    fprintf(fp, "押金不足患者数量：%d\n", deposit_warning_count);
    fprintf(fp, "住院押金总余额：%.2f\n", deposit_total);
    fprintf(fp, "------------------------------------------------------------\n\n");
    
    // 写入床位明细
    fprintf(fp, "【床位明细】\n");
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "%-12s | %-10s | %-12s | %-8s | %-6s | %-12s\n",
            "病房编号", "床位编号", "科室", "类型", "状态", "患者编号");
    fprintf(fp, "------------------------------------------------------------\n");
    
    if (g_ward_list != NULL)
    {
        WardNode* curr = g_ward_list->next;
        while (curr != NULL)
        {
            const char* ward_type_str = "";
            switch (curr->ward_type)
            {
                case WARD_GENERAL: ward_type_str = "普通"; break;
                case WARD_ICU: ward_type_str = "ICU"; break;
                case WARD_ISOLATION: ward_type_str = "隔离"; break;
                case WARD_SINGLE: ward_type_str = "单间"; break;
                default: ward_type_str = "未知";
            }
            const char* status_str = curr->is_occupied != 0 ? "占用" : "空闲";
            
            fprintf(fp, "%-12s | %-10s | %-12s | %-8s | %-6s | %-12s\n",
                    curr->room_id,
                    curr->bed_id,
                    strlen(curr->dept) > 0 ? curr->dept : "无",
                    ward_type_str,
                    status_str,
                    curr->is_occupied != 0 ? curr->patient_id : "无");
            curr = curr->next;
        }
    }
    fprintf(fp, "------------------------------------------------------------\n\n");
    
    // 写入住院明细
    fprintf(fp, "【住院明细】\n");
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "%-14s | %-12s | %-10s | %-12s | %-8s\n",
            "住院编号", "患者编号", "床位编号", "押金余额", "是否在院");
    fprintf(fp, "------------------------------------------------------------\n");
    
    if (g_inpatient_list != NULL)
    {
        InpatientRecord* curr = g_inpatient_list->next;
        while (curr != NULL)
        {
            const char* active_str = curr->is_active == 1 ? "在院" : "已出院";
            
            fprintf(fp, "%-14s | %-12s | %-10s | %-12.2f | %-8s\n",
                    curr->inpatient_id,
                    curr->patient_id,
                    strlen(curr->bed_id) > 0 ? curr->bed_id : "无",
                    curr->deposit_balance,
                    active_str);
            curr = curr->next;
        }
    }
    fprintf(fp, "------------------------------------------------------------\n");
    fclose(fp);
    
    printf("✅ 报表导出成功：%s\n", filename);
    return 1;
}

// 投诉服务处理报表
static int export_service_report(void)
{
    char filename[128];
    build_report_filename(filename, sizeof(filename), "service_report");
    
    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("❌ 报表导出失败，请检查 report 目录权限或文件是否被占用。\n");
        return 0;
    }
    write_utf8_bom(fp);
    
    // 写入报表头
    write_report_header(fp, "投诉服务处理报表");
    
    // 投诉统计变量
    int complaint_total = 0;
    int pending_count = 0;
    int replied_count = 0;
    
    // 遍历投诉链表
    if (g_complaint_list != NULL)
    {
        ComplaintNode* curr = g_complaint_list->next;
        while (curr != NULL)
        {
            complaint_total++;
            if (curr->status == 0)
                pending_count++;
            else if (curr->status == 1)
                replied_count++;
            curr = curr->next;
        }
    }
    
    // 写入投诉统计
    fprintf(fp, "【投诉统计】\n");
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "投诉总数：%d\n", complaint_total);
    fprintf(fp, "待处理投诉：%d\n", pending_count);
    fprintf(fp, "已回复投诉：%d\n", replied_count);
    fprintf(fp, "------------------------------------------------------------\n\n");
    
    // 写入投诉明细
    fprintf(fp, "【投诉明细】\n");
    
    if (g_complaint_list != NULL)
    {
        ComplaintNode* curr = g_complaint_list->next;
        while (curr != NULL)
        {
            const char* status_str = curr->status == 0 ? "待处理" : "已回复";
            const char* content_str = (strlen(curr->content) > 0) ? curr->content : "无投诉内容";
            const char* response_str = (strlen(curr->response) > 0) ? curr->response : "暂无回复";
            
            fprintf(fp, "------------------------------------------------------------\n");
            fprintf(fp, "投诉编号：%s\n", curr->complaint_id);
            fprintf(fp, "患者编号：%s\n", curr->patient_id);
            fprintf(fp, "处理状态：%s\n", status_str);
            fprintf(fp, "投诉内容：%s\n", content_str);
            fprintf(fp, "回复内容：%s\n", response_str);
            curr = curr->next;
        }
    }
    fprintf(fp, "------------------------------------------------------------\n\n");
    
    fclose(fp);
    
    printf("✅ 投诉服务处理报表导出成功：%s\n", filename);
    return 1;
}

// 收费财务统计报表
static int export_finance_report(void)
{
    char filename[128];
    build_report_filename(filename, sizeof(filename), "finance_report");
    
    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("❌ 收费财务统计报表导出失败，请检查 report 目录权限或文件是否被占用。\n");
        return 0;
    }
    write_utf8_bom(fp);
    
    // 写入报表头
    write_report_header(fp, "收费财务统计报表");
    
    // ========== 1. 患者账户统计 ==========
    int patient_total = 0;
    double total_balance = 0.0;
    int unpaid_count = 0;
    int wait_med_count = 0;
    int emergency_debt_count = 0;
    double total_emergency_debt = 0.0;
    
    if (g_patient_list != NULL)
    {
        PatientNode* curr = g_patient_list->next;
        while (curr != NULL)
        {
            patient_total++;
            total_balance += curr->balance;
            
            if (curr->status == STATUS_UNPAID)
                unpaid_count++;
            if (curr->status == STATUS_WAIT_MED)
                wait_med_count++;
            if (curr->emergency_debt > 0)
            {
                emergency_debt_count++;
                total_emergency_debt += curr->emergency_debt;
            }
            curr = curr->next;
        }
    }
    
    double avg_balance = (patient_total > 0) ? (total_balance / patient_total) : 0.0;
    
    // ========== 2. 挂号缴费统计 ==========
    int appointment_total = 0;
    int fee_paid_count = 0;
    int fee_unpaid_count = 0;
    
    if (g_appointment_list != NULL)
    {
        AppointmentNode* curr = g_appointment_list->next;
        while (curr != NULL)
        {
            appointment_total++;
            if (curr->fee_paid == 1)
                fee_paid_count++;
            else
                fee_unpaid_count++;
            curr = curr->next;
        }
    }
    
    // ========== 3. 检查缴费统计 ==========
    int check_total = 0;
    int check_paid_count = 0;
    int check_unpaid_count = 0;
    
    if (g_check_record_list != NULL)
    {
        CheckRecordNode* curr = g_check_record_list->next;
        while (curr != NULL)
        {
            check_total++;
            if (curr->is_paid == 1)
                check_paid_count++;
            else
                check_unpaid_count++;
            curr = curr->next;
        }
    }
    
    // ========== 4. 住院押金统计 ==========
    int inpatient_total = 0;
    int inpatient_active = 0;
    int inpatient_discharged = 0;
    double total_deposit = 0.0;
    int deposit_insufficient_count = 0;
    
    if (g_inpatient_list != NULL)
    {
        InpatientRecord* curr = g_inpatient_list->next;
        while (curr != NULL)
        {
            inpatient_total++;
            if (curr->is_active == 1)
            {
                inpatient_active++;
                total_deposit += curr->deposit_balance;
                if (curr->deposit_balance < 1000.0)
                    deposit_insufficient_count++;
            }
            else
            {
                inpatient_discharged++;
            }
            curr = curr->next;
        }
    }
    
    // ========== 写入统计汇总 ==========
    fprintf(fp, "【财务统计汇总】\n");
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "患者总数：%d\n", patient_total);
    fprintf(fp, "患者账户余额总额：%.2f 元\n", total_balance);
    fprintf(fp, "患者平均账户余额：%.2f 元\n", avg_balance);
    fprintf(fp, "待缴费患者人数：%d\n", unpaid_count);
    fprintf(fp, "待取药患者人数：%d\n", wait_med_count);
    fprintf(fp, "急诊欠费患者人数：%d\n", emergency_debt_count);
    fprintf(fp, "急诊欠费总金额：%.2f 元\n", total_emergency_debt);
    fprintf(fp, "------------------------------------------------------------\n\n");
    
    fprintf(fp, "【挂号缴费统计】\n");
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "预约/挂号记录总数：%d\n", appointment_total);
    fprintf(fp, "已缴挂号费记录：%d\n", fee_paid_count);
    fprintf(fp, "未缴挂号费记录：%d\n", fee_unpaid_count);
    fprintf(fp, "------------------------------------------------------------\n\n");
    
    fprintf(fp, "【检查缴费统计】\n");
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "检查记录总数：%d\n", check_total);
    fprintf(fp, "已缴费检查记录：%d\n", check_paid_count);
    fprintf(fp, "未缴费检查记录：%d\n", check_unpaid_count);
    fprintf(fp, "------------------------------------------------------------\n\n");
    
    fprintf(fp, "【住院押金统计】\n");
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "住院记录总数：%d\n", inpatient_total);
    fprintf(fp, "当前住院中：%d\n", inpatient_active);
    fprintf(fp, "已出院：%d\n", inpatient_discharged);
    fprintf(fp, "当前住院押金总余额：%.2f 元\n", total_deposit);
    fprintf(fp, "押金不足患者数量：%d\n", deposit_insufficient_count);
    fprintf(fp, "------------------------------------------------------------\n\n");
    
    // ========== 待缴费/欠费患者明细 ==========
    fprintf(fp, "【待缴费/欠费患者明细】\n");
    
    int debt_patient_count = 0;
    if (g_patient_list != NULL)
    {
        PatientNode* curr = g_patient_list->next;
        while (curr != NULL)
        {
            if (curr->status == STATUS_UNPAID || curr->emergency_debt > 0 || curr->balance < 0)
            {
                debt_patient_count++;
                fprintf(fp, "------------------------------------------------------------\n");
                fprintf(fp, "患者编号：%s\n", curr->id);
                fprintf(fp, "患者姓名：%s\n", curr->name);
                fprintf(fp, "当前状态：%s\n", get_patient_status_text(curr->status));
                fprintf(fp, "账户余额：%.2f 元\n", curr->balance);
                fprintf(fp, "急诊欠费：%.2f 元\n", curr->emergency_debt);
            }
            curr = curr->next;
        }
    }
    
    if (debt_patient_count == 0)
    {
        fprintf(fp, "暂无待缴费或欠费患者。\n");
    }
    fprintf(fp, "------------------------------------------------------------\n\n");
    
    // ========== 押金不足住院患者明细 ==========
    fprintf(fp, "【押金不足住院患者明细】\n");
    
    int deposit_insufficient_found = 0;
    if (g_inpatient_list != NULL)
    {
        InpatientRecord* curr = g_inpatient_list->next;
        while (curr != NULL)
        {
            if (curr->is_active == 1 && curr->deposit_balance < 1000.0)
            {
                deposit_insufficient_found++;
                fprintf(fp, "------------------------------------------------------------\n");
                fprintf(fp, "住院编号：%s\n", curr->inpatient_id);
                fprintf(fp, "患者编号：%s\n", curr->patient_id);
                fprintf(fp, "押金余额：%.2f 元\n", curr->deposit_balance);
                fprintf(fp, "状态：住院中\n");
            }
            curr = curr->next;
        }
    }
    
    if (deposit_insufficient_found == 0)
    {
        fprintf(fp, "暂无押金不足住院患者。\n");
    }
    fprintf(fp, "------------------------------------------------------------\n\n");
    
    fclose(fp);
    
    printf("✅ 收费财务统计报表导出成功：%s\n", filename);
    return 1;
}

// 操作日志报表
static int export_log_report(void)
{
    char filename[128];
    build_report_filename(filename, sizeof(filename), "log_report");
    
    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("❌ 报表导出失败，请检查 report 目录权限或文件是否被占用。\n");
        return 0;
    }
    write_utf8_bom(fp);
    
    // 写入报表头
    write_report_header(fp, "操作日志报表");
    
    // 日志统计变量
    int log_total = 0;
    
    // 遍历日志链表
    if (g_log_list != NULL)
    {
        LogNode* curr = g_log_list->next;
        while (curr != NULL)
        {
            log_total++;
            curr = curr->next;
        }
    }
    
    // 写入统计内容
    fprintf(fp, "【统计汇总】\n");
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "操作日志总数：%d\n", log_total);
    fprintf(fp, "------------------------------------------------------------\n\n");
    
    // 写入日志明细
    fprintf(fp, "【日志明细】\n");
    fprintf(fp, "------------------------------------------------------------\n");
    fprintf(fp, "%-18s | %-12s | %-15s | %-30s\n",
            "操作时间", "操作类型", "操作对象", "操作说明");
    fprintf(fp, "------------------------------------------------------------\n");
    
    if (g_log_list != NULL)
    {
        LogNode* curr = g_log_list->next;
        int log_num = 1;
        while (curr != NULL)
        {
            char desc[31];
            safe_utf8_truncate(desc, curr->description, 30);
            
            fprintf(fp, "%-18s | %-12s | %-15s | %-30s\n",
                    curr->timestamp,
                    curr->operation,
                    curr->target,
                    desc);
            log_num++;
            curr = curr->next;
        }
    }
    fprintf(fp, "------------------------------------------------------------\n");
    
    fclose(fp);
    
    printf("✅ 报表导出成功：%s\n", filename);
    return 1;
}

// 一键导出全部报表
static int export_all_reports(void)
{
    int success_count = 0;
    int fail_count = 0;
    
    printf("正在导出全部报表...\n\n");
    
    // 导出患者就诊统计报表
    if (export_patient_report())
        success_count++;
    else
        fail_count++;
    
    // 导出预约挂号统计报表
    if (export_appointment_report())
        success_count++;
    else
        fail_count++;
    
    // 导出药品库存预警报表
    if (export_medicine_report())
        success_count++;
    else
        fail_count++;
    
    // 导出床位住院统计报表
    if (export_ward_inpatient_report())
        success_count++;
    else
        fail_count++;
    
    // 导出投诉服务处理报表
    if (export_service_report())
        success_count++;
    else
        fail_count++;
    
    // 导出操作日志报表
    if (export_log_report())
        success_count++;
    else
        fail_count++;
    
    // 导出收费财务统计报表
    if (export_finance_report())
        success_count++;
    else
        fail_count++;
    
    printf("\n一键导出完成：成功 %d 个，失败 %d 个。\n", success_count, fail_count);
    return success_count > 0 ? 1 : 0;
}

void admin_report_menu(void)
{
    int running = 1;
    
    // 确保报表目录存在
    ensure_report_dir();
    
    while (running)
    {
        system("cls");
        printf("========== 报表导出中心 ==========\n");
        printf("报表导出中心用于将系统中的核心业务数据生成本地 txt 报表，便于归档、打印和统计分析。\n");
        printf("==================================\n");
        printf("[1] 导出患者就诊统计报表\n");
        printf("[2] 导出预约挂号统计报表\n");
        printf("[3] 导出药品库存预警报表\n");
        printf("[4] 导出床位住院统计报表\n");
        printf("[5] 导出投诉服务处理报表\n");
        printf("[6] 导出操作日志报表\n");
        printf("[7] 导出收费财务统计报表\n");
        printf("[8] 一键导出全部报表\n");
        printf("[0] 返回管理员菜单\n");
        printf("==================================\n");
        
        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                printf("\n");
                if (export_patient_report())
                    printf("\n可在项目目录下的 report 文件夹中查看生成的报表文件。\n");
                system("pause");
                break;
            case 2:
                printf("\n");
                if (export_appointment_report())
                    printf("\n可在项目目录下的 report 文件夹中查看生成的报表文件。\n");
                system("pause");
                break;
            case 3:
                printf("\n");
                if (export_medicine_report())
                    printf("\n可在项目目录下的 report 文件夹中查看生成的报表文件。\n");
                system("pause");
                break;
            case 4:
                printf("\n");
                if (export_ward_inpatient_report())
                    printf("\n可在项目目录下的 report 文件夹中查看生成的报表文件。\n");
                system("pause");
                break;
            case 5:
                printf("\n");
                if (export_service_report())
                    printf("\n可在项目目录下的 report 文件夹中查看生成的报表文件。\n");
                system("pause");
                break;
            case 6:
                printf("\n");
                if (export_log_report())
                    printf("\n可在项目目录下的 report 文件夹中查看生成的报表文件。\n");
                system("pause");
                break;
            case 7:
                printf("\n");
                if (export_finance_report())
                    printf("\n可在项目目录下的 report 文件夹中查看生成的报表文件。\n");
                system("pause");
                break;
            case 8:
                printf("\n");
                export_all_reports();
                system("pause");
                break;
            case 0:
                running = 0;
                break;
            default:
                printf("\n⚠️ 无效选项，请重新输入\n");
                system("pause");
                break;
        }
    }
}

static const char* get_role_text(RoleType role)
{
    switch (role)
    {
        case ROLE_ADMIN: return "管理员";
        case ROLE_DOCTOR: return "医生";
        case ROLE_NURSE: return "护士";
        case ROLE_PHARMACIST: return "药师";
        case ROLE_PATIENT: return "患者";
        default: return "未知";
    }
}

static void show_account_info(AccountNode* account)
{
    if (account == NULL) return;
    
    system("cls");
    printf("========== 我的账号信息 ==========\n");
    printf("登录账号：%s\n", account->username);
    printf("真实姓名：%s\n", account->real_name);
    printf("性别：%s\n", account->gender);
    printf("角色：%s\n", get_role_text(account->role));
    printf("==================================\n");
    system("pause");
}

static void change_password(AccountNode* account)
{
    if (account == NULL) return;
    
    char old_password[MAX_ID_LEN] = "";
    char new_password[MAX_ID_LEN] = "";
    char confirm_password[MAX_ID_LEN] = "";
    
    system("cls");
    printf("========== 修改登录密码 ==========\n");
    
    get_password_with_toggle("请输入旧密码：", old_password, MAX_ID_LEN);
    
    if (strcmp(old_password, account->password) != 0)
    {
        printf("\n❌ 旧密码错误，修改失败。\n");
        system("pause");
        return;
    }
    
    get_password_with_toggle("请输入新密码：", new_password, MAX_ID_LEN);
    
    if (strlen(new_password) == 0)
    {
        printf("\n❌ 新密码不能为空。\n");
        system("pause");
        return;
    }
    
    get_password_with_toggle("请再次确认新密码：", confirm_password, MAX_ID_LEN);
    
    if (strcmp(new_password, confirm_password) != 0)
    {
        printf("\n❌ 两次输入的新密码不一致，修改失败。\n");
        system("pause");
        return;
    }
    
    strncpy(account->password, new_password, MAX_ID_LEN - 1);
    
    save_account_list(g_account_list);
    
    printf("\n✅ 密码修改成功，请牢记新密码。\n");
    
    system("pause");
}

static void show_duty_status(AccountNode* account)
{
    if (account == NULL) return;
    
    system("cls");
    printf("========== 我的值班状态 ==========\n");
    
    if (account->role == ROLE_ADMIN)
    {
        printf("管理员账号无固定值班状态。\n");
    }
    else if (account->role == ROLE_DOCTOR || account->role == ROLE_NURSE || account->role == ROLE_PHARMACIST)
    {
        printf("当前值班状态：%s\n", account->is_on_duty ? "在岗" : "离岗");
    }
    else
    {
        printf("该角色无值班状态信息。\n");
    }
    
    printf("==================================\n");
    system("pause");
}

void user_profile_menu(AccountNode* current_account)
{
    if (current_account == NULL)
    {
        printf("⚠️ 未找到当前登录账号信息！\n");
        system("pause");
        return;
    }

    int running = 1;
    int choice;
    
    while (running)
    {
        system("cls");
        printf("========== 个人中心 ==========\n");
        printf("[1] 查看我的账号信息\n");
        printf("[2] 修改登录密码\n");
        printf("[3] 查看我的值班状态\n");
        printf("[0] 返回上一级\n");
        printf("==============================\n");
        printf("请输入选择：");
        
        choice = get_safe_int("");
        
        switch (choice)
        {
            case 1:
                show_account_info(current_account);
                break;
            case 2:
                change_password(current_account);
                break;
            case 3:
                show_duty_status(current_account);
                break;
            case 0:
                running = 0;
                break;
            default:
                printf("\n⚠️ 无效选项，请重新输入\n");
                system("pause");
                break;
        }
    }
}
