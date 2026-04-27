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

// 外部函数声明
extern void query_patient_complaints(const char* patient_id);

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

void admin_complaint_menu(void)
{
    int choice;
    
    do
    {
        printf("\n==============================================================\n");
        printf("                      投诉管理\n");
        printf("==============================================================\n");
        printf("[1] 查看所有投诉\n");
        printf("[2] 处理投诉\n");
        printf("[3] 按患者查询投诉\n");
        printf("[4] 按工单号查询投诉\n");
        printf("[0] 返回上一级\n");
        printf("==============================================================\n");
        printf("请输入选择：");
        
        choice = get_safe_int("");
        
        switch (choice)
        {
            case 1:
                show_all_complaints();
                break;
            case 2:
                handle_complaint_response();
                break;
            case 3:
                query_patient_complaints_by_id();
                break;
            case 4:
                query_complaint_by_id();
                break;
            case 0:
                break;
            default:
                printf("输入错误，请重新选择！\n");
                break;
        }
    } while (choice != 0);
}

void admin_evaluation_menu(void)
{
    int choice;
    
    do
    {
        printf("\n==============================================================\n");
        printf("                      评价管理\n");
        printf("==============================================================\n");
        printf("[1] 查看所有评价\n");
        printf("[2] 评价统计\n");
        printf("[0] 返回上一级\n");
        printf("==============================================================\n");
        printf("请输入选择：");
        
        choice = get_safe_int("");
        
        switch (choice)
        {
            case 1:
                show_all_evaluations();
                break;
            case 2:
                show_evaluation_statistics();
                break;
            case 0:
                break;
            default:
                printf("输入错误，请重新选择！\n");
                break;
        }
    } while (choice != 0);
}

void show_all_complaints(void)
{
    printf("\n==============================================================\n");
    printf("                      所有投诉\n");
    printf("==============================================================\n");
    printf("（暂无投诉记录）\n");
    printf("==============================================================\n");
    printf("按任意键返回...\n");
    get_single_char("");
}

void handle_complaint_response(void)
{
    printf("\n==============================================================\n");
    printf("                      处理投诉\n");
    printf("==============================================================\n");
    printf("（暂无待处理投诉）\n");
    printf("==============================================================\n");
    printf("按任意键返回...\n");
    get_single_char("");
}

void query_patient_complaints_by_id(void)
{
    char patient_id[MAX_ID_LEN];
    
    printf("\n==============================================================\n");
    printf("                      按患者查询投诉\n");
    printf("==============================================================\n");
    get_safe_string("请输入患者编号：", patient_id, sizeof(patient_id));
    query_patient_complaints(patient_id);
}

void query_complaint_by_id(void)
{
    printf("\n==============================================================\n");
    printf("                      按工单号查询投诉\n");
    printf("==============================================================\n");
    printf("（功能开发中）\n");
    printf("==============================================================\n");
    printf("按任意键返回...\n");
    get_single_char("");
}

void show_all_evaluations(void)
{
    printf("\n==============================================================\n");
    printf("                      所有评价\n");
    printf("==============================================================\n");
    printf("（暂无评价记录）\n");
    printf("==============================================================\n");
    printf("按任意键返回...\n");
    get_single_char("");
}

void show_evaluation_statistics(void)
{
    printf("\n==============================================================\n");
    printf("                      评价统计\n");
    printf("==============================================================\n");
    printf("（暂无统计数据）\n");
    printf("==============================================================\n");
    printf("按任意键返回...\n");
    get_single_char("");
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

// 显示近效期药品摘要
void show_expiring_medicines_summary(void)
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
    
    // 第一步：统计每一列的最大显示宽度
    int id_width = get_display_width("患者编号");
    int name_width = get_display_width("姓名");
    int age_width = get_display_width("年龄");
    int status_width = get_display_width("当前状态");
    int dept_width = get_display_width("就诊科室");
    int total_width = id_width + name_width + age_width + status_width + dept_width + 20; // 20是列间距总和
    
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
    
    // 输出底部横线
    for (int i = 0; i < total_width; i++) printf("-");
    printf("\n");
    
    // 输出统计信息
    char count_str[50];
    snprintf(count_str, sizeof(count_str), "待发药患者数量：%d", count);
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

// 查看已占用床位信息
static void show_occupied_beds(void)
{
    int count = 0;
    
    // 第一步：统计每一列的最大显示宽度
    int room_bed_width = get_display_width("病房号 / 床位号");
    int type_width = get_display_width("病房类型");
    int patient_id_width = get_display_width("患者编号");
    int patient_name_width = get_display_width("患者姓名");
    int status_width = get_display_width("当前状态");
    
    // 统计已占用床位并计算列宽
    if (g_ward_list != NULL && g_ward_list->next != NULL)
    {
        WardNode* curr = g_ward_list->next;
        while (curr != NULL)
        {
            if (curr->is_occupied)
            {
                count++;
                
                const char* ward_type_name = NULL;
                switch (curr->ward_type)
                {
                    case WARD_TYPE_GENERAL:
                        ward_type_name = "普通病房";
                        break;
                    case WARD_TYPE_ICU:
                        ward_type_name = "ICU";
                        break;
                    case WARD_TYPE_ISOLATION:
                        ward_type_name = "隔离病房";
                        break;
                    case WARD_TYPE_SINGLE:
                        ward_type_name = "单人病房";
                        break;
                    default:
                        ward_type_name = "未知";
                        break;
                }
                
                int current_room_bed_width = get_display_width(curr->room_id) + get_display_width(" / ") + get_display_width(curr->bed_id);
                int current_type_width = get_display_width(ward_type_name);
                int current_patient_id_width = get_display_width(curr->patient_id);
                int current_patient_name_width = 0;
                
                // 查找患者姓名
                if (g_patient_list != NULL && g_patient_list->next != NULL)
                {
                    PatientNode* patient_curr = g_patient_list->next;
                    while (patient_curr != NULL)
                    {
                        if (strcmp(patient_curr->id, curr->patient_id) == 0)
                        {
                            current_patient_name_width = get_display_width(patient_curr->name);
                            break;
                        }
                        patient_curr = patient_curr->next;
                    }
                }
                
                if (current_room_bed_width > room_bed_width)
                    room_bed_width = current_room_bed_width;
                if (current_type_width > type_width)
                    type_width = current_type_width;
                if (current_patient_id_width > patient_id_width)
                    patient_id_width = current_patient_id_width;
                if (current_patient_name_width > patient_name_width)
                    patient_name_width = current_patient_name_width;
            }
            curr = curr->next;
        }
    }
    
    // 第二步：按动态列宽输出表格
    printf("\n");
    int total_width = room_bed_width + type_width + patient_id_width + patient_name_width + status_width + 20; // 20是列间距总和
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    // 输出标题
    int title_width = get_display_width("已占用床位信息");
    int title_padding = (total_width - title_width) / 2;
    for (int i = 0; i < title_padding; i++) printf(" ");
    printf("已占用床位信息");
    for (int i = 0; i < total_width - title_width - title_padding; i++) printf(" ");
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    if (count == 0)
    {
        printf("当前无已占用床位\n");
    }
    else
    {
        // 输出表头
        print_padded_text("病房号 / 床位号", room_bed_width);
        printf("    ");
        print_padded_text("病房类型", type_width);
        printf("    ");
        print_padded_text("患者编号", patient_id_width);
        printf("    ");
        print_padded_text("患者姓名", patient_name_width);
        printf("    ");
        print_padded_text("当前状态", status_width);
        printf("\n");
        
        // 输出表头下横线
        for (int i = 0; i < total_width; i++) printf("-");
        printf("\n");
        
        // 输出数据行
        if (g_ward_list != NULL && g_ward_list->next != NULL)
        {
            WardNode* curr = g_ward_list->next;
            while (curr != NULL)
            {
                if (curr->is_occupied)
                {
                    const char* ward_type_name = NULL;
                    switch (curr->ward_type)
                    {
                        case WARD_TYPE_GENERAL:
                            ward_type_name = "普通病房";
                            break;
                        case WARD_TYPE_ICU:
                            ward_type_name = "ICU";
                            break;
                        case WARD_TYPE_ISOLATION:
                            ward_type_name = "隔离病房";
                            break;
                        case WARD_TYPE_SINGLE:
                            ward_type_name = "单人病房";
                            break;
                        default:
                            ward_type_name = "未知";
                            break;
                    }
                    
                    char room_bed_str[100];
                    snprintf(room_bed_str, sizeof(room_bed_str), "%s / %s", curr->room_id, curr->bed_id);
                    
                    char patient_name[50] = "";
                    if (g_patient_list != NULL && g_patient_list->next != NULL)
                    {
                        PatientNode* patient_curr = g_patient_list->next;
                        while (patient_curr != NULL)
                        {
                            if (strcmp(patient_curr->id, curr->patient_id) == 0)
                            {
                                strncpy(patient_name, patient_curr->name, sizeof(patient_name) - 1);
                                break;
                            }
                            patient_curr = patient_curr->next;
                        }
                    }
                    
                    print_padded_text(room_bed_str, room_bed_width);
                    printf("    ");
                    print_padded_text(ward_type_name, type_width);
                    printf("    ");
                    print_padded_text(curr->patient_id, patient_id_width);
                    printf("    ");
                    print_padded_text(patient_name, patient_name_width);
                    printf("    ");
                    print_padded_text("已占用", status_width);
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
    snprintf(count_str, sizeof(count_str), "已占用床位数量：%d", count);
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

// 查看空闲床位信息
static void show_free_beds(void)
{
    int count = 0;
    
    // 第一步：统计每一列的最大显示宽度
    int room_bed_width = get_display_width("病房号 / 床位号");
    int type_width = get_display_width("病房类型");
    int status_width = get_display_width("当前状态");
    
    // 统计空闲床位并计算列宽
    if (g_ward_list != NULL && g_ward_list->next != NULL)
    {
        WardNode* curr = g_ward_list->next;
        while (curr != NULL)
        {
            if (!curr->is_occupied)
            {
                count++;
                
                const char* ward_type_name = NULL;
                switch (curr->ward_type)
                {
                    case WARD_TYPE_GENERAL:
                        ward_type_name = "普通病房";
                        break;
                    case WARD_TYPE_ICU:
                        ward_type_name = "ICU";
                        break;
                    case WARD_TYPE_ISOLATION:
                        ward_type_name = "隔离病房";
                        break;
                    case WARD_TYPE_SINGLE:
                        ward_type_name = "单人病房";
                        break;
                    default:
                        ward_type_name = "未知";
                        break;
                }
                
                int current_room_bed_width = get_display_width(curr->room_id) + get_display_width(" / ") + get_display_width(curr->bed_id);
                int current_type_width = get_display_width(ward_type_name);
                
                if (current_room_bed_width > room_bed_width)
                    room_bed_width = current_room_bed_width;
                if (current_type_width > type_width)
                    type_width = current_type_width;
            }
            curr = curr->next;
        }
    }
    
    // 第二步：按动态列宽输出表格
    printf("\n");
    int total_width = room_bed_width + type_width + status_width + 12; // 12是列间距总和
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    // 输出标题
    int title_width = get_display_width("空闲床位信息");
    int title_padding = (total_width - title_width) / 2;
    for (int i = 0; i < title_padding; i++) printf(" ");
    printf("空闲床位信息");
    for (int i = 0; i < total_width - title_width - title_padding; i++) printf(" ");
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    if (count == 0)
    {
        printf("当前无空闲床位\n");
    }
    else
    {
        // 输出表头
        print_padded_text("病房号 / 床位号", room_bed_width);
        printf("    ");
        print_padded_text("病房类型", type_width);
        printf("    ");
        print_padded_text("当前状态", status_width);
        printf("\n");
        
        // 输出表头下横线
        for (int i = 0; i < total_width; i++) printf("-");
        printf("\n");
        
        // 输出数据行
        if (g_ward_list != NULL && g_ward_list->next != NULL)
        {
            WardNode* curr = g_ward_list->next;
            while (curr != NULL)
            {
                if (!curr->is_occupied)
                {
                    const char* ward_type_name = NULL;
                    switch (curr->ward_type)
                    {
                        case WARD_TYPE_GENERAL:
                            ward_type_name = "普通病房";
                            break;
                        case WARD_TYPE_ICU:
                            ward_type_name = "ICU";
                            break;
                        case WARD_TYPE_ISOLATION:
                            ward_type_name = "隔离病房";
                            break;
                        case WARD_TYPE_SINGLE:
                            ward_type_name = "单人病房";
                            break;
                        default:
                            ward_type_name = "未知";
                            break;
                    }
                    
                    char room_bed_str[100];
                    snprintf(room_bed_str, sizeof(room_bed_str), "%s / %s", curr->room_id, curr->bed_id);
                    
                    print_padded_text(room_bed_str, room_bed_width);
                    printf("    ");
                    print_padded_text(ward_type_name, type_width);
                    printf("    ");
                    print_padded_text("空闲", status_width);
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
    snprintf(count_str, sizeof(count_str), "空闲床位数量：%d", count);
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

// 查看床位占用汇总
static void show_bed_occupancy_summary(void)
{
    int total_beds = 0;
    int occupied_beds = 0;
    int free_beds = 0;
    
    // 统计床位信息
    if (g_ward_list != NULL && g_ward_list->next != NULL)
    {
        WardNode* curr = g_ward_list->next;
        while (curr != NULL)
        {
            total_beds++;
            if (curr->is_occupied)
            {
                occupied_beds++;
            }
            else
            {
                free_beds++;
            }
            curr = curr->next;
        }
    }
    
    // 计算床位占用率
    double occupancy_rate = 0.0;
    if (total_beds > 0)
    {
        occupancy_rate = (double)occupied_beds / total_beds * 100;
    }
    
    // 输出汇总信息
    printf("\n==============================================================\n");
    printf("                    床位占用汇总\n");
    printf("==============================================================\n");
    printf("总床位数：%d\n", total_beds);
    printf("当前已占用床位数：%d\n", occupied_beds);
    printf("当前空闲床位数：%d\n", free_beds);
    printf("床位占用率：%.2f%%\n", occupancy_rate);
    printf("==============================================================\n");
    printf("按任意键返回...\n");
    get_single_char("");
}

// 床位占用预警
static void show_bed_occupancy_warning(void)
{
    int choice;
    
    do
    {
        printf("\n==============================================================\n");
        printf("                    床位占用预警\n");
        printf("==============================================================\n");
        printf("[1] 查看已占用床位信息\n");
        printf("[2] 查看空闲床位信息\n");
        printf("[3] 查看床位占用汇总\n");
        printf("[0] 返回上一级\n");
        printf("==============================================================\n");
        printf("请输入选择：");
        
        choice = get_safe_int("");
        
        switch (choice)
        {
            case 1:
                show_occupied_beds();
                break;
            case 2:
                show_free_beds();
                break;
            case 3:
                show_bed_occupancy_summary();
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

// 住院押金预警详情
static void show_deposit_warning(void)
{
    int count = 0;
    
    // 第一步：统计每一列的最大显示宽度
    int inpatient_id_width = get_display_width("住院号");
    int patient_id_width = get_display_width("患者编号");
    int patient_name_width = get_display_width("患者姓名");
    int ward_type_width = get_display_width("病房类型");
    int deposit_width = get_display_width("当前押金余额");
    int daily_cost_width = get_display_width("预计日费用");
    int reason_width = get_display_width("预警原因");
    
    // 统计押金预警患者并计算列宽
    if (g_inpatient_list != NULL && g_inpatient_list->next != NULL)
    {
        InpatientRecord* curr = g_inpatient_list->next;
        while (curr != NULL)
        {
            if (curr->is_active)
            {
                double daily_cost = estimate_daily_cost_by_ward_type(curr->ward_type);
                double threshold = daily_cost * 3;
                if (curr->deposit_balance < threshold)
                {
                    count++;
                    
                    const char* ward_type_name = NULL;
                    switch (curr->ward_type)
                    {
                        case WARD_TYPE_GENERAL:
                            ward_type_name = "普通病房";
                            break;
                        case WARD_TYPE_ICU:
                            ward_type_name = "ICU";
                            break;
                        case WARD_TYPE_ISOLATION:
                            ward_type_name = "隔离病房";
                            break;
                        case WARD_TYPE_SINGLE:
                            ward_type_name = "单人病房";
                            break;
                        default:
                            ward_type_name = "未知";
                            break;
                    }
                    
                    int current_inpatient_id_width = get_display_width(curr->inpatient_id);
                    int current_patient_id_width = get_display_width(curr->patient_id);
                    int current_ward_type_width = get_display_width(ward_type_name);
                    
                    if (current_inpatient_id_width > inpatient_id_width)
                        inpatient_id_width = current_inpatient_id_width;
                    if (current_patient_id_width > patient_id_width)
                        patient_id_width = current_patient_id_width;
                    if (current_ward_type_width > ward_type_width)
                        ward_type_width = current_ward_type_width;
                }
            }
            curr = curr->next;
        }
    }
    
    // 第二步：按动态列宽输出表格
    printf("\n");
    int total_width = inpatient_id_width + patient_id_width + patient_name_width + ward_type_width + deposit_width + daily_cost_width + reason_width + 28; // 28是列间距总和
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    // 输出标题
    int title_width = get_display_width("住院押金预警详情");
    int title_padding = (total_width - title_width) / 2;
    for (int i = 0; i < title_padding; i++) printf(" ");
    printf("住院押金预警详情");
    for (int i = 0; i < total_width - title_width - title_padding; i++) printf(" ");
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    if (count == 0)
    {
        printf("当前无住院押金预警\n");
    }
    else
    {
        // 输出表头
        print_padded_text("住院号", inpatient_id_width);
        printf("    ");
        print_padded_text("患者编号", patient_id_width);
        printf("    ");
        print_padded_text("患者姓名", patient_name_width);
        printf("    ");
        print_padded_text("病房类型", ward_type_width);
        printf("    ");
        print_padded_text("当前押金余额", deposit_width);
        printf("    ");
        print_padded_text("预计日费用", daily_cost_width);
        printf("    ");
        print_padded_text("预警原因", reason_width);
        printf("\n");
        
        // 输出表头下横线
        for (int i = 0; i < total_width; i++) printf("-");
        printf("\n");
        
        // 输出数据行
        if (g_inpatient_list != NULL && g_inpatient_list->next != NULL)
        {
            InpatientRecord* curr = g_inpatient_list->next;
            while (curr != NULL)
            {
                if (curr->is_active)
                {
                    double daily_cost = estimate_daily_cost_by_ward_type(curr->ward_type);
                    double threshold = daily_cost * 3;
                    if (curr->deposit_balance < threshold)
                    {
                        const char* ward_type_name = NULL;
                        switch (curr->ward_type)
                        {
                            case WARD_TYPE_GENERAL:
                                ward_type_name = "普通病房";
                                break;
                            case WARD_TYPE_ICU:
                                ward_type_name = "ICU";
                                break;
                            case WARD_TYPE_ISOLATION:
                                ward_type_name = "隔离病房";
                                break;
                            case WARD_TYPE_SINGLE:
                                ward_type_name = "单人病房";
                                break;
                            default:
                                ward_type_name = "未知";
                                break;
                        }
                        
                        char patient_name[50] = "";
                        if (g_patient_list != NULL && g_patient_list->next != NULL)
                        {
                            PatientNode* patient_curr = g_patient_list->next;
                            while (patient_curr != NULL)
                            {
                                if (strcmp(patient_curr->id, curr->patient_id) == 0)
                                {
                                    strncpy(patient_name, patient_curr->name, sizeof(patient_name) - 1);
                                    break;
                                }
                                patient_curr = patient_curr->next;
                            }
                        }
                        
                        print_padded_text(curr->inpatient_id, inpatient_id_width);
                        printf("    ");
                        print_padded_text(curr->patient_id, patient_id_width);
                        printf("    ");
                        print_padded_text(patient_name, patient_name_width);
                        printf("    ");
                        print_padded_text(ward_type_name, ward_type_width);
                        printf("    ");
                        printf("%.2f", curr->deposit_balance);
                        printf("    ");
                        printf("%.2f", daily_cost);
                        printf("    ");
                        print_padded_text("押金不足", reason_width);
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
    snprintf(count_str, sizeof(count_str), "押金预警住院患者数量：%d", count);
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

// 欠费 / 待缴费患者预警详情
static void show_unpaid_patient_warning(void)
{
    int count = 0;
    
    // 第一步：统计每一列的最大显示宽度
    int patient_id_width = get_display_width("患者编号");
    int patient_name_width = get_display_width("姓名");
    int age_width = get_display_width("年龄");
    int status_width = get_display_width("当前状态");
    int department_width = get_display_width("就诊科室");
    
    // 统计欠费/待缴费患者并计算列宽
    if (g_patient_list != NULL && g_patient_list->next != NULL)
    {
        PatientNode* curr = g_patient_list->next;
        while (curr != NULL)
        {
            if (curr->status == STATUS_UNPAID)
            {
                count++;
                
                int current_patient_id_width = get_display_width(curr->id);
                int current_patient_name_width = get_display_width(curr->name);
                
                char age_str[10];
                snprintf(age_str, sizeof(age_str), "%d", curr->age);
                int current_age_width = get_display_width(age_str);
                
                const char* status_name = "欠费/待缴费";
                int current_status_width = get_display_width(status_name);
                
                const char* department_name = curr->target_dept;
                if (department_name == NULL || strlen(department_name) == 0)
                {
                    department_name = "未分配";
                }
                int current_department_width = get_display_width(department_name);
                
                if (current_patient_id_width > patient_id_width)
                    patient_id_width = current_patient_id_width;
                if (current_patient_name_width > patient_name_width)
                    patient_name_width = current_patient_name_width;
                if (current_age_width > age_width)
                    age_width = current_age_width;
                if (current_status_width > status_width)
                    status_width = current_status_width;
                if (current_department_width > department_width)
                    department_width = current_department_width;
            }
            curr = curr->next;
        }
    }
    
    // 第二步：按动态列宽输出表格
    printf("\n");
    int total_width = patient_id_width + patient_name_width + age_width + status_width + department_width + 20; // 20是列间距总和
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    // 输出标题
    int title_width = get_display_width("欠费 / 待缴费患者预警详情");
    int title_padding = (total_width - title_width) / 2;
    for (int i = 0; i < title_padding; i++) printf(" ");
    printf("欠费 / 待缴费患者预警详情");
    for (int i = 0; i < total_width - title_width - title_padding; i++) printf(" ");
    printf("\n");
    
    for (int i = 0; i < total_width; i++) printf("=");
    printf("\n");
    
    if (count == 0)
    {
        printf("当前无欠费 / 待缴费患者预警\n");
    }
    else
    {
        // 输出表头
        print_padded_text("患者编号", patient_id_width);
        printf("    ");
        print_padded_text("姓名", patient_name_width);
        printf("    ");
        print_padded_text("年龄", age_width);
        printf("    ");
        print_padded_text("当前状态", status_width);
        printf("    ");
        print_padded_text("就诊科室", department_width);
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
                if (curr->status == STATUS_UNPAID)
                {
                    char age_str[10];
                    snprintf(age_str, sizeof(age_str), "%d", curr->age);
                    
                    const char* status_name = "欠费/待缴费";
                    
                    const char* department_name = curr->target_dept;
                    if (department_name == NULL || strlen(department_name) == 0)
                    {
                        department_name = "未分配";
                    }
                    
                    print_padded_text(curr->id, patient_id_width);
                    printf("    ");
                    print_padded_text(curr->name, patient_name_width);
                    printf("    ");
                    print_padded_text(age_str, age_width);
                    printf("    ");
                    print_padded_text(status_name, status_width);
                    printf("    ");
                    print_padded_text(department_name, department_width);
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
    snprintf(count_str, sizeof(count_str), "欠费 / 待缴费患者数量：%d", count);
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

// 统计待发药患者数量
static int get_waiting_dispense_count(void)
{
    int count = 0;
    if (g_patient_list != NULL && g_patient_list->next != NULL)
    {
        PatientNode* curr = g_patient_list->next;
        while (curr != NULL)
        {
            if (curr->status == STATUS_WAIT_MED)
            {
                count++;
            }
            curr = curr->next;
        }
    }
    return count;
}

// 统计床位占用情况
static void get_bed_occupancy(int* occupied, int* free)
{
    *occupied = 0;
    *free = 0;
    if (g_ward_list != NULL && g_ward_list->next != NULL)
    {
        WardNode* curr = g_ward_list->next;
        while (curr != NULL)
        {
            if (curr->is_occupied)
            {
                (*occupied)++;
            }
            else
            {
                (*free)++;
            }
            curr = curr->next;
        }
    }
}

// 显示资源预警
void show_resource_warnings(void)
{
    int choice;
    
    do
    {
        int low_stock_count = 0;
        int expiring_medicine_count = 0;
        int deposit_warning_count = 0;
        int waiting_dispense_count = get_waiting_dispense_count();
        int occupied_beds = 0;
        int free_beds = 0;
        int unpaid_patient_count = 0;
        get_bed_occupancy(&occupied_beds, &free_beds);

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
        printf("[6] 欠费 / 待缴费患者预警\n");
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
                show_unpaid_patient_warning();
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

// 显示负载监控
void show_load_monitoring(void)
{
    printf("\n==============================================================\n");
    printf("                      负载监控\n");
    printf("==============================================================\n");
    
    // 统计各角色在线人数
    int doctor_on_duty = 0;
    int nurse_on_duty = 0;
    int pharmacist_on_duty = 0;
    
    if (g_account_list != NULL && g_account_list->next != NULL)
    {
        AccountNode* curr = g_account_list->next;
        while (curr != NULL)
        {
            if (curr->is_on_duty)
            {
                switch (curr->role)
                {
                    case ROLE_DOCTOR:
                        doctor_on_duty++;
                        break;
                    case ROLE_NURSE:
                        nurse_on_duty++;
                        break;
                    case ROLE_PHARMACIST:
                        pharmacist_on_duty++;
                        break;
                    default:
                        break;
                }
            }
            curr = curr->next;
        }
    }
    
    printf("当前在岗医生：%d\n", doctor_on_duty);
    printf("当前在岗护士：%d\n", nurse_on_duty);
    printf("当前在岗药师：%d\n", pharmacist_on_duty);
    
    // 简单的负载评估
    if (doctor_on_duty < 2)
    {
        printf("警告：医生人数不足\n");
    }
    if (nurse_on_duty < 3)
    {
        printf("警告：护士人数不足\n");
    }
    if (pharmacist_on_duty < 1)
    {
        printf("警告：药师人数不足\n");
    }
    
    printf("==============================================================\n");
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