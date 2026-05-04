#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h> // 用于 isnan, isinf 函数
#include "admin_service.h"
#include "list_ops.h"
#include "medicine_service.h"
#include "pharmacy_service.h"
#include "patient_service.h"
#include "utils.h"

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
        case WARD_TYPE_ICU:
            return ICU_WARD_RATE;
        case WARD_TYPE_ISOLATION:
            return ISOLATION_WARD_RATE;
        case WARD_TYPE_SINGLE:
            return SINGLE_WARD_RATE;
        case WARD_TYPE_GENERAL:
        default:
            return GENERAL_WARD_RATE;
    }
}

// 前置声明
static void show_expiring_medicine_warning(void);
static int days_between_dates(const char* start_date, const char* end_date);

// 检查药品是否即将过期
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
            // ASCII 字符，宽度为1
            width++;
        }
        else
        {
            // 非 ASCII 字符（如中文），宽度为 2
            width += 2;
            // 跳过后续字节（UTF-8 编码）
            if (*p >= 0xE0) p += 2; // 3字节UTF-8
            else p += 1; // 2字节UTF-8
        }
        p++;
    }
    
    return width;
}

// 按指定宽度打印文本并填充空格
static void print_padded_text(const char* str, int target_width)
{
    if (str == NULL) str = "";

    int current_width = get_display_width(str);
    printf("%s", str);

    // 计算需要填充的空格数
    int spaces = target_width - current_width;
    if (spaces > 0)
    {
        for (int i = 0; i < spaces; i++)
        {
            printf(" ");
        }
    }
}

// 根据病房类型估算每日费用
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
    
    // 第二步：按动态列宽输出表格格式
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
    
    // 输出表头下划线
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
    
    // 输出底部分隔线
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

// 楠岃瘉璐﹀彿
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

// 淇敼鍛樺伐璧勬枡
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

// 鍒犻櫎鍛樺伐璐﹀彿
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

    // 第二步：按动态列宽输出表格格式
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

    // 输出表头下划线
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

    // 输出底部分隔线
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

    // 第二步：按动态列宽输出表格格式
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

    // 输出表头下划线
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

    // 输出底部分隔线
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

    // 第二步：按动态列宽输出表格格式
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

    // 输出表头下划线
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

    // 输出底部分隔线
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

// 显示管理员控制面板
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
            // STATUS_WAIT_MED = 5 表示"已开处方待取药"
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
    printf("患者总人数：%d\n", patient_count);
    printf("当前待发药患者数量：%d\n", waiting_dispense_count);
    printf("药品总数：%d\n", medicine_count);
    printf("低库存药品数量：%d\n", low_stock_count);
    printf("近效期药品数量：%d\n", expiring_medicine_count);
    printf("医生总数：%d\n", doctor_count);
    printf("护士总数：%d\n", nurse_count);
    printf("药师总数：%d\n", pharmacist_count);
    printf("==============================================================\n");
}

// 瑙ｆ瀽鏃ユ湡瀛楃涓蹭负 tm 缁撴瀯
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
    tm_out->tm_hour = 12; // 閬垮厤澶忎护鏃堕棶棰?

    return 1;
}

// 璁＄畻涓や釜鏃ユ湡涔嬮棿鐨勫ぉ鏁板樊
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
    int id_width = get_display_width("药品编码");
    int name_width = get_display_width("商品名称");
    int generic_name_width = get_display_width("通用名称");
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
    
    // 第二步：按动态列宽输出表格格式
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
        print_padded_text("药品编码", id_width);
        printf("    ");
        print_padded_text("商品名称", name_width);
        printf("    ");
        print_padded_text("通用名称", generic_name_width);
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
        
        // 输出表头下划线
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
}

// ==========================================
// 系统预警查看模块（整合所有类型预警）
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
        printf("  [1] 查看事件预警（不良事件、纠纷、投诉）\n");
        printf("  [2] 查看药品预警（低库存、近效期药）\n");
        printf("  [3] 查看住院预警（押金不足、欠费）\n");
        printf("  [4] 查看床位资源预警\n");
        printf("  [5] 查看所有预警（综合视图）\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");
        
        switch (get_safe_int("请输入操作编号："))
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
                printf("\n无效的选项，请重新输入！\n");
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
    int general_occupied = 0, general_free = 0;
    int special_occupied = 0, special_free = 0;
    int icu_occupied = 0, icu_free = 0;
    
    printf("\n======================================================\n");
    printf("                床位资源预警\n");
    printf("======================================================\n");
    
    // 统计床位使用情况
    if (g_ward_list != NULL && g_ward_list->next != NULL)
    {
        WardNode* ward_curr = g_ward_list->next;
        while (ward_curr != NULL)
        {
            total_beds++;
            if (ward_curr->is_occupied)
            {
                occupied_beds++;
            }
            
            if (ward_curr->ward_type == WARD_TYPE_GENERAL)
            {
                if (ward_curr->is_occupied)
                    general_occupied++;
                else
                    general_free++;
            }
            else if (ward_curr->ward_type == WARD_TYPE_SINGLE)
            {
                if (ward_curr->is_occupied)
                    special_occupied++;
                else
                    special_free++;
            }
            else if (ward_curr->ward_type == WARD_TYPE_ICU)
            {
                if (ward_curr->is_occupied)
                    icu_occupied++;
                else
                    icu_free++;
            }
            else if (ward_curr->ward_type == WARD_TYPE_ISOLATION)
            {
                // 隔离病房单独统计
                if (ward_curr->is_occupied)
                    occupied_beds++;
            }
            
            ward_curr = ward_curr->next;
        }
        
        free_beds = total_beds - occupied_beds;
        
        // 显示床位资源预警概要
        printf("【床位资源预警概要】\n");
        printf("------------------------------------------------------\n");
        printf("总床位数：%d\n", total_beds);
        printf("已使用床位：%d\n", occupied_beds);
        printf("剩余床位：" RED "%d" RESET "\n", free_beds);
        printf("床位使用率：%.2f%%\n", total_beds > 0 ? (float)occupied_beds / total_beds * 100 : 0);
        printf("------------------------------------------------------\n");
        
        // 显示各类型病房床位情况
        printf("【各类型病房床位情况】\n");
        printf("------------------------------------------------------\n");
        printf("普通病房：已使用 %d, 剩余 " RED "%d" RESET "\n", general_occupied, general_free);
        printf("特殊病房：已使用 %d, 剩余 " RED "%d" RESET "\n", special_occupied, special_free);
        printf("ICU：已使用 %d, 剩余 " RED "%d" RESET "\n", icu_occupied, icu_free);
        printf("------------------------------------------------------\n");
        
        // 显示床位资源预警
        if (free_beds == 0)
        {
            printf("" RED "⚠️ 预警：所有床位已满，请及时处理！" RESET "\n");
        }
        else if (free_beds < 5)
        {
            printf("" RED "⚠️ 预警：床位紧张，仅剩 %d 个空闲床位！" RESET "\n", free_beds);
        }
        else
        {
            printf("✅ 床位资源充足\n");
        }
    }
    else
    {
        printf("当前无病房数据\n");
    }
    
    printf("======================================================\n");
}

// 显示事件预警（不良事件、纠纷、投诉）
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

// 显示药品预警（低库存、近效期药）
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
                printf("药品编码：%s\n", med_curr->id);
                printf("商品名称：%s\n", med_curr->name);
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
    
    // 显示近效期药品预警
    printf("【近效期药品预警】\n");
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
                printf("药品编码：%s\n", med_curr->id);
                printf("商品名称：%s\n", med_curr->name);
                printf("效期：%s\n", med_curr->expiry_date);
                printf("距离过期：" RED "%d天" RESET "\n", diff_days);
                printf("------------------------------------------------------\n");
                expiring_count++;
            }
            med_curr = med_curr->next;
        }
        if (expiring_count == 0)
        {
            printf("当前无近效期药品预警\n");
        }
        printf("共找到 %d 种近效期药品\n", expiring_count);
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
                        printf("预警类型：" RED "%s" RESET "\n", warn_msg);
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
    
    // 显示欠费患者预警（需启用）
    printf("【欠费患者预警】\n");
    printf("------------------------------------------------------\n");
    printf("当前版本欠费患者预警功能需待收费/结算模块完全接入后启用。\n");
    
    printf("======================================================\n");
}

// 显示系统预警查看
void show_system_alerts(void)
{
    int has_alerts = 0;
    
    printf("\n======================================================\n");
    printf("                  系统预警查看\n");
    printf("======================================================\n");
    
    // 1. 显示医疗事故触发、纠纷爆发、投诉处理执行（来自预警列表）
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
                printf("药品编码：%s\n", med_curr->id);
                printf("商品名称：%s\n", med_curr->name);
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
    
    // 3. 显示近效期药品预警
    printf("【近效期药品预警】\n");
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
                printf("药品编码：%s\n", med_curr->id);
                printf("商品名称：%s\n", med_curr->name);
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
            printf("当前无近效期药品预警\n");
        }
        printf("共找到 %d 种近效期药品\n", expiring_count);
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
                        printf("预警类型：" RED "%s" RESET "\n", warn_msg);
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
    
    // 5. 显示欠费患者预警（需启用）
    printf("【欠费患者预警】\n");
    printf("------------------------------------------------------\n");
    printf("当前版本欠费患者预警功能需待收费/结算模块完全接入后启用。\n");
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

// ==========================================
// 投诉管理模块
// ==========================================

// 管理员投诉处理子菜单
void admin_complaint_menu()
{
    int running = 1;
    
    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               📝 投诉管理\n");
        printf("======================================================\n");
        printf("  [1] 查看待处理投诉\n");
        printf("  [2] 处理投诉\n");
        printf("  [3] 按患者编号查询投诉记录\n");
        printf("  [4] 按投诉编号查询投诉详情\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");
        
        switch (get_safe_int("请输入操作编号："))
        {
            case 1:
                show_all_complaints();
                system("pause");
                break;
            case 2:
                handle_complaint_response();
                system("pause");
                break;
            case 3:
                query_patient_complaints_by_id();
                system("pause");
                break;
            case 4:
                query_complaint_by_id();
                system("pause");
                break;
            case 0:
                running = 0;
                break;
            default:
                printf("\n无效的选项，请重新输入！\n");
                system("pause");
                break;
        }
    }
}

// 显示待处理投诉
void show_all_complaints()
{
    if (g_complaint_list == NULL || g_complaint_list->next == NULL)
    {
        printf("\n当前无投诉记录\n");
        return;
    }
    
    printf("\n======================================================\n");
    printf("                    待处理投诉列表\n");
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
                case 3: printf("对药房\n"); break;
                default: printf("未知\n"); break;
            }
            
            printf("被投诉人：%s（工号：%s）\n", curr->target_name, curr->target_id);
            printf("投诉内容：%s\n", curr->content);
            
            printf("处理状态：待回复\n");
            printf("处理意见：%s\n", curr->response);
            printf("------------------------------------------------------\n");
        }
        curr = curr->next;
    }
    
    if (!has_processed)
    {
        printf("\n当前没有待处理的投诉\n");
    }
    
    printf("======================================================\n");
}

// 处理投诉
void handle_complaint_response()
{
    if (g_complaint_list == NULL || g_complaint_list->next == NULL)
    {
        printf("\n当前无投诉记录\n");
        return;
    }
    
    // 先显示待处理的投诉
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
                case 3: printf("对药房\n"); break;
                default: printf("未知\n"); break;
            }
            printf("被投诉人：%s（工号：%s）\n", curr->target_name, curr->target_id);
            printf("投诉内容：%s\n", curr->content);
            printf("------------------------------------------------------\n");
        }
        curr = curr->next;
    }
    
    if (!has_pending)
    {
        printf("\n✅ 当前没有待处理的投诉\n");
        return;
    }
    
    // 输入要处理的投诉编号
    char complaint_id[MAX_ID_LEN];
    get_safe_string("请输入要处理的投诉工单编号：", complaint_id, MAX_ID_LEN);
    
    // 查找投诉
    curr = g_complaint_list->next;
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
        printf("\n未找到该投诉工单\n");
        return;
    }
    
    if (target_complaint->status != 0)
    {
        printf("\n该投诉工单已经处理过了！\n");
        return;
    }
    
    // 输入处理意见
    char response[MAX_RECORD_LEN];
    get_safe_string("请输入处理意见：", response, MAX_RECORD_LEN);
    
    // 更新投诉状态和回复
    target_complaint->status = 1;
    strncpy(target_complaint->response, response, MAX_RECORD_LEN - 1);
    target_complaint->response[MAX_RECORD_LEN - 1] = '\0';
    
    printf("\n✅ 投诉处理成功\n");
    printf("工单编号：%s\n", target_complaint->complaint_id);
    printf("处理意见：%s\n", target_complaint->response);
}

// 按患者编号查询投诉记录
void query_patient_complaints_by_id()
{
    char patient_id[MAX_ID_LEN];
    get_safe_string("请输入患者编号：", patient_id, MAX_ID_LEN);
    
    // 调用现有的 query_patient_complaints 函数
    query_patient_complaints(patient_id);
}

// 按投诉编号查询投诉详情
void query_complaint_by_id()
{
    char complaint_id[MAX_ID_LEN];
    get_safe_string("请输入投诉编号：", complaint_id, MAX_ID_LEN);
    
    if (g_complaint_list == NULL || g_complaint_list->next == NULL)
    {
        printf("\n当前无投诉记录\n");
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
        printf("\n未找到该投诉工单\n");
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
        case 3: printf("对药房\n"); break;
        default: printf("未知\n"); break;
    }
    
    printf("被投诉人：%s（工号：%s）\n", target_complaint->target_name, target_complaint->target_id);
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
        
        switch (get_safe_int("请输入操作编号："))
        {
            case 1:
                show_all_evaluations();
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
                printf("\n无效的选项，请重新输入！\n");
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
        printf("\n当前无评价记录\n");
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
            printf("问诊医生：%s（编号：%s）\n", 
                   find_doctor_by_id(g_doctor_list, curr->doctor_id) ? 
                   find_doctor_by_id(g_doctor_list, curr->doctor_id)->name : "未知", 
                   curr->doctor_id);
            printf("问诊时间：%s\n", curr->consult_time);
            
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
                printf("文字评价：（空）\n");
            }
            printf("------------------------------------------------------\n");
        }
        curr = curr->next;
    }
    
    if (evaluation_count == 0)
    {
        printf("\n当前无评价记录\n");
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
        printf("\n问诊记录链表尚未初始化！\n");
        return;
    }
    
    int total_evaluations = 0;
    int star_counts[6] = {0}; // star_counts[0] 表示未评价，star_counts[1-5] 表示各星级
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
        printf("\n当前无评价记录\n");
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

// 显示负载监控查看
void show_load_monitoring(void)
{
    system("cls");
    printf("\n================================================================================\n");
    printf("                        综合负载监控\n");
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

    printf("【公共资源统计】\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("医生总数：%d\n", doctor_count);
    printf("患者总数：%d\n", patient_count);
    printf("床位总数：%d\n", bed_count);
    printf("已使用床位：%d\n", occupied_beds);
    printf("剩余床位：%d\n", bed_count - occupied_beds);
    printf("--------------------------------------------------------------------------------\n");

    printf("提示：此面板数据实时更新，可作为系统运行监控参考。\n");
    printf("================================================================================\n");
    printf("按任意键返回...\n");
    get_single_char("");
}

// 待发药患者预警
static void show_waiting_dispense_warning(void)
{
    int count = 0;
    int total_width = 0;
    
    int id_width = get_display_width("患者编号");
    int name_width = get_display_width("姓名");
    int age_width = get_display_width("年龄");
    int status_width = get_display_width("当前状态");
    int dept_width = get_display_width("问诊科室");
    
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
                        status_name = "已开方待取药";
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

    total_width = id_width + name_width + age_width + status_width + dept_width + 12;

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
    print_padded_text("问诊科室", dept_width);
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");
    
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
                        status_name = "已开方待取药";
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
    printf("按任意键返回...\n");
    get_single_char("");
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
                // 只统计当前仍在住院中的患者
                if (curr->is_active)
                {
                    occupied_beds++;
                    // 根据病房类型计算日费用，若押金不足3日费用则预警
                    double daily_cost = estimate_daily_cost_by_ward_type(curr->ward_type);
                    double threshold = daily_cost * 3; // 押金不足3日费用时预警
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

        // 计算剩余床位数量（假设共100个床位）
        free_beds = 100 - occupied_beds;
        if (free_beds < 0) free_beds = 0;

        printf("\n==============================================================\n");
        printf("                      资源预警\n");
        printf("==============================================================\n");
        printf("低库存药品数量：%d\n", low_stock_count);
        printf("近效期药品数量：%d\n", expiring_medicine_count);
        printf("待发药患者数量：%d\n", waiting_dispense_count);
        printf("当前已使用床位数量：%d\n", occupied_beds);
        printf("当前剩余床位数量：%d\n", free_beds);
        printf("押金预警住院患者数量：%d\n", deposit_warning_count);
        printf("欠费/待缴费患者数量：%d\n", unpaid_patient_count);
        printf("==============================================================\n");
        printf("请选择要查看的预警详情\n");
        printf("==============================================================\n");
        printf("[1] 低库存药品预警\n");
        printf("[2] 近效期药品预警\n");
        printf("[3] 待发药患者预警\n");
        printf("[4] 床位占用预警\n");
        printf("[5] 住院押金预警\n");
        printf("[6] 欠费/待缴费预警\n");
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

    // 计算剩余床位数量（假设共100个床位）
    free_count = 100 - occupied_count;
    if (free_count < 0) free_count = 0;

    if (occupied_count == 0)
    {
        printf("当前暂无床位占用预警。\n");
    }
    else
    {
        printf("已使用床位总数量：%d\n", occupied_count);
        printf("剩余床位总数量：%d\n", free_count);
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
                // 根据病房类型计算日费用，若押金不足3日费用则预警
                double daily_cost = estimate_daily_cost_by_ward_type(curr->ward_type);
                double threshold = daily_cost * 3; // 押金不足3日费用时预警
                
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
        printf("押金预警住院患者总数量：%d\n", count);
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
                printf("状态：%s\n", curr->balance < 0 ? "欠费" : "待缴费");
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
        printf("欠费 / 待缴费患者总数量：%d\n", count);
    }
    
    printf("======================================================\n");
    printf("按任意键返回...\n");
    get_single_char("");
}

// 显示公共状态统计信息
void show_public_status_statistics(void)
{
    printf("\n==============================================================\n");
    printf("                      公共状态统计信息\n");
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
    
    // 统计医务人员数量
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

    // 统计护士和药师数量
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

    // 显示公共状态统计信息
    printf("\n======================================================\n");
    printf("                  状态统计信息\n");
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

// 显示传染病预警提醒
void show_infectious_disease_alerts(void)
{
    printf("\n==============================================================\n");
    printf("                      传染病预警提醒\n");
    printf("==============================================================\n");
    printf("当前无传染病异常预警\n");
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
    printf("提示：输入'B' 返回上一步，第一步输入'B' 退出\n");

    while (step >= 1 && step <= 7)
    {
        switch (step)
        {
            case 1:
                while (1)
                {
                    if (!get_form_string("请输入商品名（输入 B 退出）: ", name, MAX_MED_NAME_LEN))
                    {
                        return;
                    }
                    if (is_blank_string(name))
                    {
                        printf("商品名不能为空，请重新输入\n");
                        continue;
                    }
                    // 预先检查商品名是否已存在，避免用户输入后再提示
                    if (is_medicine_name_exists(name))
                    {
                        printf("提示：已存在同名药品，请使用不同名称或修改已有药品\n");
                        continue;
                    }
                    step = 2;
                    break;
                }
                break;

            case 2:
                while (1)
                {
                    if (!get_form_string("请输入别名（可空，输入 B 返回上一步）: ", alias, MAX_ALIAS_LEN))
                    {
                        step = 1;
                        break;
                    }
                    step = 3;
                    break;
                }
                break;

            case 3:
                while (1)
                {
                    if (!get_form_string("请输入通用名（输入 B 返回上一步）: ", generic_name, MAX_GENERIC_NAME_LEN))
                    {
                        step = 2;
                        break;
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
                    if (!get_form_double("请输入药品单价（输入 B 返回上一步）: ", &price, 0, "单价必须是大于 0 的数字，请重新输入\n"))
                    {
                        step = 3;
                        break;
                    }
                    step = 5;
                    break;
                }
                break;

            case 5:
                while (1)
                {
                    if (!get_form_int("请输入初始库存（输入 B 返回上一步）: ", &stock, 0, 999999, "库存必须是大于等于 0 的整数，请重新输入\n"))
                    {
                        step = 4;
                        break;
                    }
                    step = 6;
                    break;
                }
                break;

            case 6:
                printf("医保类型：0=非医保 1=甲类医保 2=乙类医保\n");
                while (1)
                {
                    if (!get_form_int("请输入医保类型编号（输入 B 返回上一步）: ", &medicare_type, 0, 2, "医保类型输入无效，请重新输入\n"))
                    {
                        step = 5;
                        break;
                    }
                    step = 7;
                    break;
                }
                break;

            case 7:
                while (1)
                {
                    if (!get_form_date("请输入效期（YYYY-MM-DD，输入 B 返回上一步）: ", expiry_date, MAX_DATE_LEN))
                    {
                        step = 6;
                        break;
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
    const char* alias_ptr = NULL;
    double new_price = -1.0; // 初始化为-1，表示不修改

    printf("\n================ 修改药品基本信息 ===============-\n");

    // 1. 药品编号（必填）
    MedicineNode* medicine_node = NULL;
    while (1)
    {
        get_safe_string("请输入药品编号（输入 B 返回上一级）: ", med_id, MAX_ID_LEN);
        
        // 检查是否输入B/b返回上一级
        if (strcmp(med_id, "B") == 0 || strcmp(med_id, "b") == 0)
        {
            return;
        }
        
        // 检查是否为空
        if (is_blank_string(med_id))
        {
            printf("药品编号不能为空，请重新输入\n");
            continue;
        }
        
        // 检查药品是否存在
        medicine_node = find_medicine_by_id(g_medicine_list, med_id);
        if (medicine_node == NULL)
        {
            printf("未找到编号为 %s 的药品，修改流程终止\n", med_id);
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

    // 2. 商品名（可选，空白字符串表示不修改）
    get_safe_string("请输入新商品名（留空不修改，输入 B 返回上一级）: ", new_name, MAX_MED_NAME_LEN);
    if (strcmp(new_name, "B") == 0 || strcmp(new_name, "b") == 0)
    {
        return;
    }

    // 3. 通用名（可选，空白字符串表示不修改）
    get_safe_string("请输入新通用名（留空不修改，输入 B 返回上一级）: ", new_generic_name, MAX_GENERIC_NAME_LEN);
    if (strcmp(new_generic_name, "B") == 0 || strcmp(new_generic_name, "b") == 0)
    {
        return;
    }

    // 4. 别名（可选，输入B返回上一级，空字符串表示清空）
    get_safe_string("请输入新别名（空不修改，输入 B 返回上一级，输入空字符串清空别名）: ", new_alias, MAX_ALIAS_LEN);
    if (strcmp(new_alias, "B") == 0 || strcmp(new_alias, "b") == 0)
    {
        return;
    }
    if (!is_blank_string(new_alias))
    {
        alias_ptr = new_alias;
    }

    // 5. 单价（可选，-1 表示不修改）
    while (1)
    {
        char price_str[32];
        get_safe_string("请输入新单价（空不修改，输入 B 返回上一级）: ", price_str, sizeof(price_str));
        if (strcmp(price_str, "B") == 0 || strcmp(price_str, "b") == 0)
        {
            return;
        }
        if (is_blank_string(price_str))
        {
            new_price = -1.0; // 保持不变
            break;
        }
        if (sscanf(price_str, "%lf", &new_price) != 1 || new_price <= 0)
        {
            printf("单价必须是大于 0 的数字，请重新输入\n");
            continue;
        }
        break;
    }

    // 6. 效期（可选，空白字符串表示不修改）
    get_safe_string("请输入新效期（空不修改，输入 B 返回上一级）: ", new_expiry_date, MAX_DATE_LEN);
    if (strcmp(new_expiry_date, "B") == 0 || strcmp(new_expiry_date, "b") == 0)
    {
        return;
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
}

void handle_medicine_stock_update(void)
{
    char med_id[MAX_ID_LEN];
    int new_stock;

    printf("\n================ 修改药品库存 ===============-\n");
    printf("提示：输入'B' 可以返回上一级菜单\n");

    while (1)
    {
        get_safe_string("请输入药品编号（输入 B 返回上一级）: ", med_id, MAX_ID_LEN);
        if (strcmp(med_id, "B") == 0 || strcmp(med_id, "b") == 0)
        {
            return;
        }
        if (is_blank_string(med_id))
        {
            printf("药品编号不能为空，请重新输入\n");
            continue;
        }
        break;
    }

    while (1)
    {
        char stock_str[32];
        get_safe_string("请输入新库存数量（输入 B 返回上一级）: ", stock_str, sizeof(stock_str));
        if (strcmp(stock_str, "B") == 0 || strcmp(stock_str, "b") == 0)
        {
            return;
        }
        if (sscanf(stock_str, "%d", &new_stock) != 1 || new_stock < 0)
        {
            printf("库存数量必须是大于等于 0 的整数，请重新输入\n");
            continue;
        }
        break;
    }

    update_medicine_stock(med_id, new_stock);
}

void handle_medicine_search(void)
{
    char keyword[MAX_MED_NAME_LEN];

    printf("\n================ 查询药品 ================\n");
    printf("输入药品编号或关键词查询，输入 Q 退出\n");

    while (1)
    {
        printf("请输入药品编号或关键词：");
        fflush(stdout);
        if (fgets(keyword, MAX_MED_NAME_LEN, stdin) == NULL) break;
        keyword[strcspn(keyword, "\n")] = '\0';

        if (keyword[0] == '\0') continue;

        if (my_strcasecmp(keyword, "Q") == 0)
        {
            printf("已退出药品查询\n");
            break;
        }

        search_medicine_by_keyword(keyword);
        printf("------------------------------------------\n");
    }
}

void handle_medicine_show_all(void)
{
    show_all_medicines();
}

void handle_medicine_low_stock(void)
{
    int threshold;

    printf("\n================ 低库存药品 ===============-\n");
    threshold = get_safe_int("请输入低库存阈值：");
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
    days_threshold = get_safe_int("请输入近效期天数阈值：");
    show_expiring_medicines(today, days_threshold);
}

void handle_medicine_dispense(void)
{
    char patient_id[MAX_ID_LEN];

    printf("\n================ 发药处理 ================\n");
    printf("提示：输入'0' 可以返回上一步，输入 '00' 可以退出操作\n");
    show_paid_patients_waiting_for_dispense();
    printf("------------------------------------------------------\n");
    
    while (1)
    {
        get_safe_string("请输入要发药的患者编号：", patient_id, MAX_ID_LEN);
        
        // 检查是否退出
        if (strcmp(patient_id, "00") == 0)
        {
            printf("操作取消\n");
            return;
        }
        
        // 检查是否返回
        {
            return;
        }
        
        // 妫€鏌ユ偅鑰呯紪鍙锋牸寮忔槸鍚﹀悎娉?
        if (validate_patient_id(patient_id))
        {
            break;
        }
        else
        {
            printf("鈿狅笍 鎮ｈ€呯紪鍙锋牸寮忎笉鍚堟硶锛屾纭牸寮忎负 P-1001锛岃閲嶆柊杈撳叆锛乗n");
            printf("鎻愮ず锛氳緭鍏?'0' 鍙互鍥為€€涓婁竴姝ワ紝杈撳叆 '00' 鍙互閫€鍑烘搷浣淺n");
        }
    }
    
    dispense_medicine_for_patient(patient_id);
}

void handle_medicine_remove(void)
{
    char med_id[MAX_ID_LEN];
    MedicineNode* medicine = NULL;
    int confirm;

    printf("\n================ 涓嬫灦鑽搧 ===============-\n");
    get_safe_string("璇疯緭鍏ヨ涓嬫灦鐨勮嵂鍝佺紪鍙? ", med_id, MAX_ID_LEN);

    // 鏌ユ壘鑽搧鏄惁瀛樺湪
    medicine = find_medicine_by_id(g_medicine_list, med_id);
    if (medicine == NULL)
    {
        printf("鎻愮ず锛氭湭鎵惧埌瀵瑰簲鑽搧锛屼笅鏋跺け璐ャ€俓n");
        return;
    }

    // 鏄剧ず鑽搧鍩烘湰淇℃伅
    printf("\n褰撳墠鑽搧淇℃伅锛歕n");
    printf("鑽搧缂栧彿锛?s\n", medicine->id);
    printf("鍟嗗搧鍚嶏細%s\n", medicine->name);
    printf("閫氱敤鍚嶏細%s\n", medicine->generic_name);
    printf("鍒悕锛?s\n", medicine->alias[0] == '\0' ? "鏃? : medicine->alias);
    printf("鍗曚环锛?.2f\n", medicine->price);
    printf("搴撳瓨锛?d\n", medicine->stock);
    // 鏄剧ず鍖讳繚绫诲瀷
    switch (medicine->m_type)
    {
        case MEDICARE_NONE:
            printf("鍖讳繚绫诲瀷锛氶潪鍖讳繚\n");
            break;
        case MEDICARE_CLASS_A:
            printf("鍖讳繚绫诲瀷锛氱敳绫诲尰淇漒n");
            break;
        case MEDICARE_CLASS_B:
            printf("鍖讳繚绫诲瀷锛氫箼绫诲尰淇漒n");
            break;
        default:
            printf("鍖讳繚绫诲瀷锛氭湭鐭n");
            break;
    }
    printf("鏁堟湡锛?s\n", medicine->expiry_date);
    printf("----------------------------------------\n");

    // 浜屾纭
    printf("鏄惁纭畾涓嬫灦璇ヨ嵂鍝侊紵\n");
    printf("[1] 纭畾涓嬫灦\n");
    printf("[0] 鍙栨秷杩斿洖\n");
    confirm = get_safe_int("璇疯緭鍏ユ搷浣滅紪鍙? ");

    if (confirm != 1)
    {
        printf("鎻愮ず锛氬凡鍙栨秷涓嬫灦鎿嶄綔銆俓n");
        return;
    }

    // 鎵ц涓嬫灦鎿嶄綔
    remove_medicine(med_id);
}

// 鍒濆鍖栨棩蹇楀垪琛?
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

// 娣诲姞鏃ュ織
void add_log(const char* operation, const char* target, const char* description)
{
    if (operation == NULL || target == NULL || description == NULL)
        return;
    
    // 鍒濆鍖栨棩蹇楅摼琛紙濡傛灉鏈垵濮嬪寲锛?
    if (g_log_list == NULL)
    {
        init_log_list();
    }
    
    // 鍒涘缓鏂版棩蹇楄妭鐐?
    LogNode* new_node = (LogNode*)malloc(sizeof(LogNode));
    if (new_node == NULL)
        return;
    
    // 璁剧疆鏃ュ織鏃堕棿
    time_t now = time(NULL);
    struct tm now_tm;
    localtime_s(&now_tm, &now);
    snprintf(new_node->timestamp, sizeof(new_node->timestamp), 
             "%d-%02d-%02d %02d:%02d:%02d",
             now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday,
             now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec);
    
    // 璁剧疆鏃ュ織鍐呭
    strncpy(new_node->operation, operation, sizeof(new_node->operation) - 1);
    new_node->operation[sizeof(new_node->operation) - 1] = '\0';
    
    strncpy(new_node->target, target, sizeof(new_node->target) - 1);
    new_node->target[sizeof(new_node->target) - 1] = '\0';
    
    strncpy(new_node->description, description, sizeof(new_node->description) - 1);
    new_node->description[sizeof(new_node->description) - 1] = '\0';
    
    new_node->next = NULL;
    
    // 娣诲姞鍒伴摼琛ㄥ熬閮?
    LogNode* curr = g_log_list;
    while (curr->next != NULL)
    {
        curr = curr->next;
    }
    curr->next = new_node;
}

// 鏄剧ず鏃ュ織
void show_logs(void)
{
    printf("\n==============================================================\n");
    printf("                      绯荤粺鏃ュ織\n");
    printf("==============================================================\n");
    
    if (g_log_list == NULL || g_log_list->next == NULL)
    {
        printf("褰撳墠鏃犳棩蹇楄褰昞n");
        printf("==============================================================\n");
        return;
    }
    printf("\n==============================================================\n");
    printf("                        绯荤粺鏃ュ織\n");
    printf("==============================================================\n");
    print_field("鏃堕棿", 20); print_field("鎿嶄綔", 16); print_field("鐩爣", 8); printf("鎻忚堪\n");
    printf("--------------------------------------------------------------\n");

    LogNode* curr = g_log_list->next;
    int count = 0;
    
    while (curr != NULL)
    {
        printf("%-20s", curr->timestamp);
        print_field(curr->operation, 16);
        printf("%-8s %s\n", curr->target, curr->description);
        count++;
        curr = curr->next;
    }
    
    printf("--------------------------------------------------------------\n");
    printf("鏃ュ織鎬绘暟锛?d\n", count);
    printf("==============================================================\n");
}

// 澶勭悊鑽搧鍩烘湰淇℃伅鏇存柊
void handle_medicine_basic_info_update(void)
{
    char med_id[MAX_ID_LEN];
    char new_name[MAX_MED_NAME_LEN];
    char new_alias[MAX_ALIAS_LEN];
    char new_generic_name[MAX_GENERIC_NAME_LEN];
    double new_price;
    char new_expiry_date[MAX_DATE_LEN];
    
    printf("\n==============================================================\n");
    printf("                      鏇存柊鑽搧鍩烘湰淇℃伅\n");
    printf("==============================================================\n");
    
    get_safe_string("璇疯緭鍏ヨ嵂鍝佺紪鍙凤細", med_id, sizeof(med_id));
    
    // 妫€鏌ヨ嵂鍝佹槸鍚﹀瓨鍦?
    MedicineNode* medicine = find_medicine_by_id(g_medicine_list, med_id);
    if (medicine == NULL)
    {
        printf("鑽搧涓嶅瓨鍦紒\n");
        printf("==============================================================\n");
        return;
    }
    
    get_safe_string("璇疯緭鍏ユ柊鐨勮嵂鍝佸悕绉帮細", new_name, sizeof(new_name));
    get_safe_string("璇疯緭鍏ユ柊鐨勮嵂鍝佸埆鍚嶏細", new_alias, sizeof(new_alias));
    get_safe_string("璇疯緭鍏ユ柊鐨勮嵂鍝侀€氱敤鍚嶏細", new_generic_name, sizeof(new_generic_name));
    new_price = get_safe_double("璇疯緭鍏ユ柊鐨勮嵂鍝佷环鏍硷細");
    get_safe_string("璇疯緭鍏ユ柊鐨勬湁鏁堟湡锛圷YYY-MM-DD锛夛細", new_expiry_date, sizeof(new_expiry_date));
    
    // 璋冪敤鏇存柊鑽搧鍩烘湰淇℃伅鍑芥暟
    int result = update_medicine_basic_info(
        med_id, new_name, new_alias, new_generic_name, new_price, new_expiry_date
    );
    
    if (result == 1)
    {
        printf("\n鑽搧鍩烘湰淇℃伅鏇存柊鎴愬姛锛乗n");
        add_log("鑽搧淇℃伅鏇存柊", med_id, "鏇存柊鑽搧鍩烘湰淇℃伅");
    }
    else
    {
        printf("\n鑽搧鍩烘湰淇℃伅鏇存柊澶辫触锛乗n");
        add_log("鑽搧淇℃伅鏇存柊", med_id, "鏇存柊鑽搧鍩烘湰淇℃伅澶辫触");
    }
    
    printf("==============================================================\n");
}

// 澶勭悊杩戞晥鏈熻嵂鍝佹鏌?
void handle_expiring_medicine_check(void)
{
    char today[MAX_DATE_LEN];
    int days_threshold;
    
    printf("\n==============================================================\n");
    printf("                      杩戞晥鏈熻嵂鍝佹鏌n");
    printf("==============================================================\n");
    
    get_safe_string("璇疯緭鍏ュ綋鍓嶆棩鏈燂紙YYYY-MM-DD锛夛細", today, sizeof(today));
    days_threshold = get_safe_int("璇疯緭鍏ラ璀﹀ぉ鏁帮細");
    
    show_expiring_medicines(today, days_threshold);
    
    add_log("杩戞晥鏈熸鏌?, today, "鎵ц杩戞晥鏈熻嵂鍝佹鏌?);
    printf("==============================================================\n");
}

// 鏄剧ず鎵€鏈夋鏌ラ」鐩瓧鍏?
void show_all_check_items(void)
{
    CheckItemNode* curr = NULL;
    int count = 0;

    if (g_check_item_list == NULL || g_check_item_list->next == NULL)
    {
        printf("褰撳墠鏆傛棤妫€鏌ラ」鐩暟鎹甛n");
        return;
    }

    printf("\n==============================================================\n");
    printf("                    妫€鏌ラ」鐩瓧鍏竆n");
    printf("==============================================================\n");
    printf("%-10s %-20s %-12s %10s %8s\n",
           "椤圭洰缂栧彿", "椤圭洰鍚嶇О", "鎵€灞炵瀹?, "浠锋牸(鍏?", "鍖讳繚绫诲瀷");
    printf("--------------------------------------------------------------\n");

    curr = g_check_item_list->next;
    while (curr != NULL)
    {
        const char* m_type_name = NULL;
        switch (curr->m_type)
        {
            case MEDICARE_CLASS_A:
                m_type_name = "鐢茬被";
                break;
            case MEDICARE_CLASS_B:
                m_type_name = "涔欑被";
                break;
            case MEDICARE_NONE:
                m_type_name = "鑷垂";
                break;
            default:
                m_type_name = "鏈煡";
                break;
        }
        printf("%-10s %-20s %-12s %10.2f %8s\n",
               curr->item_id,
               curr->item_name,
               curr->dept,
               curr->price,
               m_type_name);
        count++;
        curr = curr->next;
    }

    printf("--------------------------------------------------------------\n");
    printf("妫€鏌ラ」鐩€绘暟锛?d\n", count);
    printf("==============================================================\n");
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
        printf("鎻愮ず锛氭煡璇㈠叧閿瓧涓嶈兘涓虹┖銆俓n");
        return;
    }

    if (g_check_item_list == NULL || g_check_item_list->next == NULL)
    {
        printf("鏈壘鍒板尮閰嶆鏌ラ」鐩甛n");
        return;
    }

    printf("\n================ 妫€鏌ラ」鐩煡璇㈢粨鏋?================\n");
    printf("%-10s %-20s %-12s %10s %8s\n",
           "椤圭洰缂栧彿", "椤圭洰鍚嶇О", "鎵€灞炵瀹?, "浠锋牸(鍏?", "鍖讳繚绫诲瀷");
    printf("------------------------------------------\n");

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
                case MEDICARE_CLASS_A: m_type_name = "鐢茬被"; break;
                case MEDICARE_CLASS_B: m_type_name = "涔欑被"; break;
                case MEDICARE_NONE:    m_type_name = "鑷垂"; break;
                default:               m_type_name = "鏈煡"; break;
            }
            printf("%-10s %-20s %-12s %10.2f %8s\n",
                   curr->item_id, curr->item_name, curr->dept,
                   curr->price, m_type_name);
            found = 1;
        }
        curr = curr->next;
    }

    if (!found) printf("鏈壘鍒板尮閰嶆鏌ラ」鐩甛n");
}

void handle_check_item_register(void)
{
    char item_name[MAX_NAME_LEN];
    char dept[MAX_NAME_LEN];
    double price;
    int medicare_type;
    char new_id[MAX_ID_LEN];

    printf("\n================ 鏂板妫€鏌ラ」鐩?===============-\n");
    printf("鎻愮ず锛氫换鎰忚緭鍏ラ」杈撳叆 'B' 鍙繑鍥炰笂涓€绾ц彍鍗昞n\n");

    while (1)
    {
        if (!get_form_string("璇疯緭鍏ユ鏌ラ」鐩悕绉帮紙杈撳叆 B 杩斿洖涓婁竴绾э級: ", item_name, MAX_NAME_LEN))
            return;
        if (is_blank_string(item_name))
        {
            printf("妫€鏌ラ」鐩悕绉颁笉鑳戒负绌猴紝璇烽噸鏂拌緭鍏n");
            continue;
        }
        break;
    }

    while (1)
    {
        if (!get_form_string("璇疯緭鍏ユ墍灞炵瀹わ紙杈撳叆 B 杩斿洖涓婁竴绾э級: ", dept, MAX_NAME_LEN))
            return;
        if (is_blank_string(dept))
        {
            printf("鎵€灞炵瀹や笉鑳戒负绌猴紝璇烽噸鏂拌緭鍏n");
            continue;
        }
        break;
    }

    while (1)
    {
        if (!get_form_double("璇疯緭鍏ユ鏌ヨ垂鐢紙杈撳叆 B 杩斿洖涓婁竴绾э級: ", &price, 0, "妫€鏌ヨ垂鐢ㄥ繀椤诲ぇ浜?0锛岃閲嶆柊杈撳叆\n"))
            return;
        break;
    }

    printf("鍖讳繚绫诲瀷锛?=鑷垂 1=鐢茬被 2=涔欑被\n");
    while (1)
    {
        if (!get_form_int("璇疯緭鍏ュ尰淇濈被鍨嬬紪鍙凤紙杈撳叆 B 杩斿洖涓婁竴绾э級: ", &medicare_type, 0, 2, "鍖讳繚绫诲瀷杈撳叆闈炴硶锛岃閲嶆柊杈撳叆\n"))
            return;
        break;
    }

    if (g_check_item_list == NULL)
    {
        printf("鎻愮ず锛氭鏌ラ」鐩摼琛ㄥ皻鏈垵濮嬪寲锛屾棤娉曟柊澧炪€俓n");
        return;
    }

    generate_check_item_id(new_id);
    CheckItemNode* new_node = create_check_item_node(new_id, item_name, dept, price, (MedicareType)medicare_type);
    if (new_node == NULL)
    {
        printf("鎻愮ず锛氭鏌ラ」鐩妭鐐瑰垱寤哄け璐ャ€俓n");
        return;
    }

    insert_check_item_tail(g_check_item_list, new_node);
    printf("妫€鏌ラ」鐩柊澧炴垚鍔燂紒\n");
    printf("椤圭洰缂栧彿锛?s\n", new_node->item_id);
    printf("椤圭洰鍚嶇О锛?s\n", new_node->item_name);
    printf("鎵€灞炵瀹わ細%s\n", new_node->dept);
    printf("妫€鏌ヨ垂鐢細%.2f\n", new_node->price);
    printf("鍖讳繚绫诲瀷锛?s\n",
           new_node->m_type == MEDICARE_CLASS_A ? "鐢茬被" :
           new_node->m_type == MEDICARE_CLASS_B ? "涔欑被" : "鑷垂");
}

void handle_check_item_update(void)
{
    char item_id[MAX_ID_LEN];
    char new_name[MAX_NAME_LEN];
    char new_dept[MAX_NAME_LEN];
    double new_price = -1.0;
    int new_m_type = -1;

    printf("\n================ 淇敼妫€鏌ラ」鐩俊鎭?===============-\n");

    CheckItemNode* target = NULL;
    while (1)
    {
        get_safe_string("璇疯緭鍏ユ鏌ラ」鐩紪鍙凤紙杈撳叆 B 杩斿洖涓婁竴绾э級: ", item_id, MAX_ID_LEN);
        if (strcmp(item_id, "B") == 0 || strcmp(item_id, "b") == 0) return;
        if (is_blank_string(item_id))
        {
            printf("妫€鏌ラ」鐩紪鍙蜂笉鑳戒负绌猴紝璇烽噸鏂拌緭鍏n");
            continue;
        }
        target = find_check_item_by_id(g_check_item_list, item_id);
        if (target == NULL)
        {
            printf("鏈壘鍒扮紪鍙蜂负 %s 鐨勬鏌ラ」鐩紝淇敼娴佺▼缁撴潫\n", item_id);
            return;
        }
        break;
    }

    printf("\n------------- 褰撳墠妫€鏌ラ」鐩俊鎭?------------\n");
    printf("椤圭洰缂栧彿锛?s\n", target->item_id);
    printf("椤圭洰鍚嶇О锛?s\n", target->item_name);
    printf("鎵€灞炵瀹わ細%s\n", target->dept);
    printf("妫€鏌ヨ垂鐢細%.2f\n", target->price);
    switch (target->m_type)
    {
        case MEDICARE_CLASS_A: printf("鍖讳繚绫诲瀷锛氱敳绫籠n"); break;
        case MEDICARE_CLASS_B: printf("鍖讳繚绫诲瀷锛氫箼绫籠n"); break;
        case MEDICARE_NONE:    printf("鍖讳繚绫诲瀷锛氳嚜璐筡n"); break;
    }
    printf("----------------------------------------\n");

    get_safe_string("璇疯緭鍏ユ柊椤圭洰鍚嶇О锛堢暀绌轰笉淇敼锛岃緭鍏?B 杩斿洖涓婁竴绾э級: ", new_name, MAX_NAME_LEN);
    if (strcmp(new_name, "B") == 0 || strcmp(new_name, "b") == 0) return;

    get_safe_string("璇疯緭鍏ユ柊鎵€灞炵瀹わ紙鐣欑┖涓嶄慨鏀癸紝杈撳叆 B 杩斿洖涓婁竴绾э級: ", new_dept, MAX_NAME_LEN);
    if (strcmp(new_dept, "B") == 0 || strcmp(new_dept, "b") == 0) return;

    while (1)
    {
        char price_str[32];
        get_safe_string("璇疯緭鍏ユ柊妫€鏌ヨ垂鐢紙鐣欑┖涓嶄慨鏀癸紝杈撳叆 B 杩斿洖涓婁竴绾э級: ", price_str, sizeof(price_str));
        if (strcmp(price_str, "B") == 0 || strcmp(price_str, "b") == 0) return;
        if (is_blank_string(price_str)) break;
        char* endptr = NULL;
        new_price = strtod(price_str, &endptr);
        if (endptr == price_str || *endptr != '\0' || new_price <= 0)
        {
            printf("妫€鏌ヨ垂鐢ㄥ繀椤讳负澶т簬 0 鐨勬暟瀛楋紝璇烽噸鏂拌緭鍏n");
            continue;
        }
        break;
    }

    printf("鍖讳繚绫诲瀷锛?=鑷垂 1=鐢茬被 2=涔欑被锛?1 涓嶄慨鏀癸級\n");
    while (1)
    {
        char type_str[32];
        get_safe_string("璇疯緭鍏ユ柊鍖讳繚绫诲瀷锛堢暀绌轰笉淇敼锛岃緭鍏?B 杩斿洖涓婁竴绾э級: ", type_str, sizeof(type_str));
        if (strcmp(type_str, "B") == 0 || strcmp(type_str, "b") == 0) return;
        if (is_blank_string(type_str)) break;
        char* endptr = NULL;
        new_m_type = (int)strtol(type_str, &endptr, 10);
        if (endptr == type_str || *endptr != '\0' || new_m_type < 0 || new_m_type > 2)
        {
            printf("鍖讳繚绫诲瀷蹇呴』涓?0/1/2锛岃閲嶆柊杈撳叆\n");
            continue;
        }
        break;
    }

    if (!is_blank_string(new_name)) strncpy(target->item_name, new_name, MAX_NAME_LEN - 1);
    if (!is_blank_string(new_dept)) strncpy(target->dept, new_dept, MAX_NAME_LEN - 1);
    if (new_price > 0) target->price = new_price;
    if (new_m_type >= 0) target->m_type = (MedicareType)new_m_type;

    printf("\n鎻愮ず锛氭鏌ラ」鐩俊鎭慨鏀规垚鍔熴€俓n");
}

void handle_check_item_remove(void)
{
    char item_id[MAX_ID_LEN];
    CheckItemNode* target = NULL;
    int confirm;

    printf("\n================ 涓嬫灦妫€鏌ラ」鐩?===============-\n");
    get_safe_string("璇疯緭鍏ヨ涓嬫灦鐨勬鏌ラ」鐩紪鍙? ", item_id, MAX_ID_LEN);

    target = find_check_item_by_id(g_check_item_list, item_id);
    if (target == NULL)
    {
        printf("鎻愮ず锛氭湭鎵惧埌瀵瑰簲妫€鏌ラ」鐩紝涓嬫灦澶辫触銆俓n");
        return;
    }

    printf("\n褰撳墠妫€鏌ラ」鐩俊鎭細\n");
    printf("椤圭洰缂栧彿锛?s\n", target->item_id);
    printf("椤圭洰鍚嶇О锛?s\n", target->item_name);
    printf("鎵€灞炵瀹わ細%s\n", target->dept);
    printf("妫€鏌ヨ垂鐢細%.2f\n", target->price);
    switch (target->m_type)
    {
        case MEDICARE_CLASS_A: printf("鍖讳繚绫诲瀷锛氱敳绫籠n"); break;
        case MEDICARE_CLASS_B: printf("鍖讳繚绫诲瀷锛氫箼绫籠n"); break;
        case MEDICARE_NONE:    printf("鍖讳繚绫诲瀷锛氳嚜璐筡n"); break;
    }
    printf("----------------------------------------\n");

    if (is_check_item_referenced_by_active_record(item_id))
    {
        printf("鎻愮ず锛氬綋鍓嶆鏌ラ」鐩粛琚湭瀹屾垚妫€鏌ヨ褰曞紩鐢紝鏆備笉鑳戒笅鏋躲€俓n");
        printf("璇峰厛瀹屾垚鐩稿叧鎮ｈ€呯殑妫€鏌ユ祦绋嬪悗鍐嶉噸璇曘€俓n");
        return;
    }

    printf("鏄惁纭畾涓嬫灦璇ユ鏌ラ」鐩紵\n");
    printf("[1] 纭畾涓嬫灦\n");
    printf("[0] 鍙栨秷杩斿洖\n");
    confirm = get_safe_int("璇疯緭鍏ユ搷浣滅紪鍙? ");

    if (confirm != 1)
    {
        printf("鎻愮ず锛氬凡鍙栨秷涓嬫灦鎿嶄綔銆俓n");
        return;
    }

    if (delete_check_item_by_id(g_check_item_list, item_id))
        printf("鎻愮ず锛氭鏌ラ」鐩笅鏋舵垚鍔熴€俓n");
    else
        printf("鎻愮ず锛氭鏌ラ」鐩笅鏋跺け璐ャ€俓n");
}


