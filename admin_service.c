#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
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

// 查看所有员工账号
void show_all_accounts(void)
{
    AccountNode* curr = NULL;
    int count = 0;

    if (g_account_list == NULL || g_account_list->next == NULL)
    {
        printf("当前暂无员工账号数据\n");
        return;
    }

    printf("\n==============================================================\n");
    printf("                        员工列表\n");
    printf("==============================================================\n");
    printf("登录账号            真实姓名            角色\n");
    printf("--------------------------------------------------------------\n");

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
        printf("%-18s %-18s %-8s\n", curr->username, curr->real_name, role_name);
        count++;
        curr = curr->next;
    }

    printf("--------------------------------------------------------------\n");
    printf("员工总数：%d\n", count);
    printf("==============================================================\n");
}

// 注册新员工账号
int register_account(const char* username, const char* password, const char* real_name, const char* gender, RoleType role)
{
    AccountNode* existing = NULL;
    AccountNode* new_account = NULL;
    char description[200];

    if (g_account_list == NULL)
    {
        printf("提示：账号链表尚未初始化，无法注册新账号。\n");
        return 0;
    }

    if (is_blank_string(username))
    {
        printf("提示：账号不能为空。\n");
        return 0;
    }

    if (is_blank_string(password))
    {
        printf("提示：密码不能为空。\n");
        return 0;
    }

    if (is_blank_string(real_name))
    {
        printf("提示：真实姓名不能为空。\n");
        return 0;
    }

    existing = find_account_by_username(g_account_list, username);
    if (existing != NULL)
    {
        printf("提示：账号已存在，注册失败。\n");
        return 0;
    }

    new_account = create_account_node(username, password, real_name, gender, role);
    if (new_account == NULL)
    {
        printf("提示：创建账号失败。\n");
        return 0;
    }

    insert_account_tail(g_account_list, new_account);
    printf("提示：账号注册成功。\n");
    printf("账号：%s\n", username);
    printf("真实姓名：%s\n", real_name);
    printf("角色：%s\n", 
        role == ROLE_ADMIN ? "管理员" :
        role == ROLE_NURSE ? "护士" :
        role == ROLE_DOCTOR ? "医生" :
        role == ROLE_PHARMACIST ? "药师" : "未知");
    
    // 记录日志
    snprintf(description, sizeof(description), "真实姓名：%s，角色：%s", real_name, 
        role == ROLE_ADMIN ? "管理员" :
        role == ROLE_NURSE ? "护士" :
        role == ROLE_DOCTOR ? "医生" :
        role == ROLE_PHARMACIST ? "药师" : "未知");
    add_log("新增员工", username, description);
    
    return 1;
}

// 更新员工资料
int update_account_basic_info(
    const char* username,
    const char* new_real_name,
    const char* new_password,
    RoleType new_role
)
{
    AccountNode* account = NULL;
    char old_real_name[MAX_NAME_LEN];
    char old_password[MAX_ID_LEN];
    RoleType old_role;
    int modified = 0;
    char description[200];
    int name_changed = 0;
    int password_changed = 0;
    int role_changed = 0;

    if (g_account_list == NULL)
    {
        printf("提示：账号链表尚未初始化，无法修改资料。\n");
        return 0;
    }

    if (is_blank_string(username))
    {
        printf("提示：账号不能为空。\n");
        return 0;
    }

    account = find_account_by_username(g_account_list, username);
    if (account == NULL)
    {
        printf("提示：未找到对应账号，修改失败。\n");
        return 0;
    }

    // 保存旧值
    safe_copy_string(old_real_name, MAX_NAME_LEN, account->real_name);
    safe_copy_string(old_password, MAX_ID_LEN, account->password);
    old_role = account->role;

    // 更新真实姓名
    if (new_real_name != NULL && !is_blank_string(new_real_name))
    {
        if (strcmp(account->real_name, new_real_name) != 0)
        {
            safe_copy_string(account->real_name, MAX_NAME_LEN, new_real_name);
            modified = 1;
        }
    }

    // 更新密码
    if (new_password != NULL && !is_blank_string(new_password))
    {
        if (strcmp(account->password, new_password) != 0)
        {
            safe_copy_string(account->password, MAX_ID_LEN, new_password);
            modified = 1;
        }
    }

    // 更新角色
    if (new_role != 0)
    {
        if (account->role != new_role)
        {
            account->role = new_role;
            modified = 1;
        }
    }

    if (modified)
    {
        printf("\n======================================================\n");
        printf("                   账号资料修改\n");
        printf("======================================================\n");
        printf("账号：%s\n", username);
        printf("------------------------------------------------------\n");
        printf("修改前：真实姓名 = %s\n", old_real_name);
        printf("修改后：真实姓名 = %s\n", account->real_name);
        printf("------------------------------------------------------\n");
        printf("修改前：角色 = %s\n", 
            old_role == ROLE_ADMIN ? "管理员" :
            old_role == ROLE_NURSE ? "护士" :
            old_role == ROLE_DOCTOR ? "医生" :
            old_role == ROLE_PHARMACIST ? "药师" : "未知");
        printf("修改后：角色 = %s\n", 
            account->role == ROLE_ADMIN ? "管理员" :
            account->role == ROLE_NURSE ? "护士" :
            account->role == ROLE_DOCTOR ? "医生" :
            account->role == ROLE_PHARMACIST ? "药师" : "未知");
        printf("======================================================\n");
        printf("提示：账号资料修改成功。\n");
        
        // 记录日志 - 动态生成描述
        name_changed = 0;
        password_changed = 0;
        role_changed = 0;
        
        if (strcmp(old_real_name, account->real_name) != 0)
        {
            name_changed = 1;
        }
        if (strcmp(old_password, account->password) != 0)
        {
            password_changed = 1;
        }
        if (old_role != account->role)
        {
            role_changed = 1;
        }
        
        if (name_changed && password_changed && role_changed)
        {
            snprintf(description, sizeof(description), "修改了姓名、密码和角色");
        }
        else if (name_changed && password_changed)
        {
            snprintf(description, sizeof(description), "修改了姓名和密码");
        }
        else if (name_changed && role_changed)
        {
            snprintf(description, sizeof(description), "修改了姓名和角色");
        }
        else if (password_changed && role_changed)
        {
            snprintf(description, sizeof(description), "修改了密码和角色");
        }
        else if (name_changed)
        {
            snprintf(description, sizeof(description), "修改了姓名");
        }
        else if (password_changed)
        {
            snprintf(description, sizeof(description), "修改了密码");
        }
        else if (role_changed)
        {
            snprintf(description, sizeof(description), "修改了角色");
        }
        else
        {
            snprintf(description, sizeof(description), "修改了资料");
        }
        
        add_log("修改员工资料", username, description);
    }
    else
    {
        printf("提示：未修改任何信息。\n");
    }

    return modified;
}

// 查看所有医生及其值班状态
void show_all_doctors_with_duty_status(void)
{
    DoctorNode* curr = NULL;
    int count = 0;

    if (g_doctor_list == NULL || g_doctor_list->next == NULL)
    {
        printf("当前暂无医生数据\n");
        return;
    }

    printf("\n========================================================================\n");
    printf("                    医生值班状态\n");
    printf("========================================================================\n");
    printf("医生工号            姓名            性别    科室            值班状态\n");
    printf("------------------------------------------------------------------------\n");

    curr = g_doctor_list->next;
    while (curr != NULL)
    {
        printf("%-18s %-12s %-4s    %-14s %-8s\n", 
            curr->id, 
            curr->name, 
            strlen(curr->gender) > 0 ? curr->gender : "-",
            curr->department, 
            curr->is_on_duty ? "值班中" : "未值班");
        count++;
        curr = curr->next;
    }

    printf("--------------------------------------------------------------\n");
    printf("医生总数：%d\n", count);
    printf("==============================================================\n");
}

// 更新医生值班状态
int update_doctor_duty_status(const char* doctor_id, int new_status)
{
    DoctorNode* doctor = NULL;
    AccountNode* account_curr = NULL;
    char description[200];

    if (g_doctor_list == NULL)
    {
        printf("提示：医生链表尚未初始化，无法更新值班状态。\n");
        return 0;
    }

    if (is_blank_string(doctor_id))
    {
        printf("提示：医生工号不能为空。\n");
        return 0;
    }

    if (new_status != 0 && new_status != 1)
    {
        printf("提示：值班状态必须是 0（未值班）或 1（值班中）。\n");
        return 0;
    }

    doctor = find_doctor_by_id(g_doctor_list, doctor_id);
    if (doctor == NULL)
    {
        printf("提示：未找到对应医生，更新失败。\n");
        return 0;
    }

    doctor->is_on_duty = new_status;
    printf("提示：医生值班状态更新成功。\n");
    printf("医生：%s (%s)\n", doctor->name, doctor->id);
    printf("新值班状态：%s\n", new_status ? "值班中" : "未值班");

    // 同步更新对应账号的值班状态
    if (g_account_list != NULL && g_account_list->next != NULL)
    {
        account_curr = g_account_list->next;
        while (account_curr != NULL)
        {
            if (strcmp(account_curr->username, doctor_id) == 0 && account_curr->role == ROLE_DOCTOR)
            {
                account_curr->is_on_duty = new_status;
                printf("提示：已同步更新账号 %s 的值班状态。\n", doctor_id);
                break;
            }
            account_curr = account_curr->next;
        }
    }

    // 记录日志
    snprintf(description, sizeof(description), "医生：%s，新状态：%s", doctor->name, new_status ? "值班中" : "未值班");
    add_log("医生值班状态修改", doctor->id, description);
    return 1;
}

// 显示管理员统计面板
void show_admin_dashboard(void)
{
    int employee_count = 0;
    int doctor_count = 0;
    int patient_count = 0;
    int pending_count = 0;
    int recheck_count = 0;
    int examining_count = 0;
    int unpaid_count = 0;
    int paid_count = 0;
    int no_show_count = 0;
    int completed_count = 0;
    int low_stock_count = 0;
    int waiting_dispense_count = 0;
    int occupied_bed_count = 0;
    int free_bed_count = 0;

    AccountNode* account_curr = NULL;
    DoctorNode* doctor_curr = NULL;
    PatientNode* patient_curr = NULL;
    MedicineNode* medicine_curr = NULL;
    WardNode* ward_curr = NULL;

    // 统计员工总数
    if (g_account_list != NULL && g_account_list->next != NULL)
    {
        account_curr = g_account_list->next;
        while (account_curr != NULL)
        {
            employee_count++;
            account_curr = account_curr->next;
        }
    }

    // 统计医生总数
    if (g_doctor_list != NULL && g_doctor_list->next != NULL)
    {
        doctor_curr = g_doctor_list->next;
        while (doctor_curr != NULL)
        {
            doctor_count++;
            doctor_curr = doctor_curr->next;
        }
    }

    // 统计患者相关数据
    if (g_patient_list != NULL && g_patient_list->next != NULL)
    {
        patient_curr = g_patient_list->next;
        while (patient_curr != NULL)
        {
            patient_count++;
            switch (patient_curr->status)
            {
                case STATUS_PENDING:
                    pending_count++;
                    break;
                case STATUS_RECHECK_PENDING:
                    recheck_count++;
                    break;
                case STATUS_EXAMINING:
                    examining_count++;
                    break;
                case STATUS_UNPAID:
                    unpaid_count++;
                    break;
                case STATUS_WAIT_MED:
                    paid_count++;
                    waiting_dispense_count++;
                    break;
                case STATUS_NO_SHOW:
                    no_show_count++;
                    break;
                case STATUS_COMPLETED:
                    completed_count++;
                    break;
                default:
                    break;
            }
            patient_curr = patient_curr->next;
        }
    }

    // 统计低库存药品数量
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        medicine_curr = g_medicine_list->next;
        while (medicine_curr != NULL)
        {
            if (medicine_curr->stock < 5) // 低库存阈值为 5
            {
                low_stock_count++;
            }
            medicine_curr = medicine_curr->next;
        }
    }

    // 统计床位使用情况
    if (g_ward_list != NULL && g_ward_list->next != NULL)
    {
        ward_curr = g_ward_list->next;
        while (ward_curr != NULL)
        {
            if (ward_curr->is_occupied)
            {
                occupied_bed_count++;
            }
            else
            {
                free_bed_count++;
            }
            ward_curr = ward_curr->next;
        }
    }

    printf("\n======================================================\n");
    printf("                  管理统计面板\n");
    printf("======================================================\n");
    printf("1. 员工总数：%d\n", employee_count);
    printf("2. 医生总数：%d\n", doctor_count);
    printf("3. 患者总数：%d\n", patient_count);
    printf("4. 待诊患者数：%d\n", pending_count);
    printf("5. 待复诊患者数：%d\n", recheck_count);
    printf("6. 检查中患者数：%d\n", examining_count);
    printf("7. 待缴费患者数：%d\n", unpaid_count);
    printf("8. 已缴费待取药患者数：%d\n", paid_count);
    printf("9. 过号作废患者数：%d\n", no_show_count);
    printf("10. 已完成患者数：%d\n", completed_count);
    printf("11. 当前低库存药品数量：%d\n", low_stock_count);
    printf("12. 当前待发药患者数量：%d\n", waiting_dispense_count);
    printf("13. 当前已占用床位数量：%d\n", occupied_bed_count);
    printf("14. 当前空闲床位数量：%d\n", free_bed_count);
    printf("======================================================\n");
    printf("提示：此面板数据实时更新，可作为系统运行监控参考。\n");
}

// 显示资源预警查看
void show_resource_warnings(void)
{
    int low_stock_count = 0;
    int occupied_bed_count = 0;
    int free_bed_count = 0;
    int total_bed_count = 0;
    int found = 0;
    int found_occupied = 0;

    MedicineNode* medicine_curr = NULL;
    WardNode* ward_curr = NULL;

    // 统计低库存药品数量
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        medicine_curr = g_medicine_list->next;
        while (medicine_curr != NULL)
        {
            if (medicine_curr->stock < 5) // 低库存阈值为 5
            {
                low_stock_count++;
            }
            medicine_curr = medicine_curr->next;
        }
    }

    // 统计床位使用情况
    if (g_ward_list != NULL && g_ward_list->next != NULL)
    {
        ward_curr = g_ward_list->next;
        while (ward_curr != NULL)
        {
            total_bed_count++;
            if (ward_curr->is_occupied)
            {
                occupied_bed_count++;
            }
            else
            {
                free_bed_count++;
            }
            ward_curr = ward_curr->next;
        }
    }

    // 显示预警摘要
    printf("\n======================================================\n");
    printf("                  资源预警查看\n");
    printf("======================================================\n");
    printf("【预警摘要】\n");
    printf("------------------------------------------------------\n");
    printf("低库存药品数量：%d\n", low_stock_count);
    printf("已占用床位数量：%d\n", occupied_bed_count);
    printf("空闲床位数量：%d\n", free_bed_count);
    printf("欠费患者数量：待启用\n");
    printf("======================================================\n");

    // 显示低库存药品明细
    printf("【低库存药品预警明细】\n");
    printf("------------------------------------------------------\n");
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        medicine_curr = g_medicine_list->next;
        found = 0;
        while (medicine_curr != NULL)
        {
            if (medicine_curr->stock < 5)
            {
                printf("药品编号：%s\n", medicine_curr->id);
                printf("商品名：%s\n", medicine_curr->name);
                printf("当前库存：%d\n", medicine_curr->stock);
                printf("效期：%s\n", medicine_curr->expiry_date);
                printf("------------------------------------------------------\n");
                found = 1;
            }
            medicine_curr = medicine_curr->next;
        }
        if (!found)
        {
            printf("当前无低库存药品预警\n");
            printf("------------------------------------------------------\n");
        }
    }
    else
    {
        printf("当前无药品数据\n");
        printf("------------------------------------------------------\n");
    }

    // 显示临期药品预警明细
    printf("【临期药品预警明细】\n");
    printf("------------------------------------------------------\n");
    if (g_medicine_list != NULL && g_medicine_list->next != NULL)
    {
        medicine_curr = g_medicine_list->next;
        found = 0;
        time_t now = time(NULL);
        struct tm* tm_now = localtime(&now);
        char today_str[11];
        sprintf(today_str, "%04d-%02d-%02d", tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday);
        
        while (medicine_curr != NULL)
        {
            // 检查效期是否在30天内
            int diff_days = days_between_dates(today_str, medicine_curr->expiry_date);
            if (diff_days >= 0 && diff_days <= 30)
            {
                printf("药品编号：%s\n", medicine_curr->id);
                printf("商品名：%s\n", medicine_curr->name);
                printf("效期：%s\n", medicine_curr->expiry_date);
                printf("距离过期：%d天\n", diff_days);
                printf("------------------------------------------------------\n");
                found = 1;
            }
            medicine_curr = medicine_curr->next;
        }
        if (!found)
        {
            printf("当前无临期药品预警\n");
            printf("------------------------------------------------------\n");
        }
    }
    else
    {
        printf("当前无药品数据\n");
        printf("------------------------------------------------------\n");
    }

    // 显示床位资源预警明细
    printf("【床位资源预警明细】\n");
    printf("------------------------------------------------------\n");
    if (g_ward_list != NULL && g_ward_list->next != NULL)
    {
        printf("总床位数：%d\n", total_bed_count);
        printf("已占用床位数：%d\n", occupied_bed_count);
        printf("空闲床位数：%d\n", free_bed_count);
        printf("------------------------------------------------------\n");
        
        if (free_bed_count <= 1)
        {
            printf("⚠️ 预警：当前床位资源紧张，请及时调度。\n");
            printf("------------------------------------------------------\n");
        }
        
        // 显示占用的床位
        ward_curr = g_ward_list->next;
        found_occupied = 0;
        while (ward_curr != NULL)
        {
            if (ward_curr->is_occupied)
            {
                printf("床位编号：%s\n", ward_curr->bed_id);
                printf("状态：占用\n");
                printf("当前患者编号：%s\n", ward_curr->patient_id);
                printf("------------------------------------------------------\n");
                found_occupied = 1;
            }
            ward_curr = ward_curr->next;
        }
        if (!found_occupied)
        {
            printf("当前无占用床位\n");
            printf("------------------------------------------------------\n");
        }
    }
    else
    {
        printf("当前无床位数据\n");
        printf("------------------------------------------------------\n");
    }

    // 显示欠费患者预警
    printf("【欠费患者预警】\n");
    printf("------------------------------------------------------\n");
    printf("当前版本欠费患者预警功能待收费/结算模块完全接入后启用。\n");
    printf("------------------------------------------------------\n");

    printf("======================================================\n");
    printf("提示：当前页面用于管理员快速查看资源风险\n");
    printf("后续可继续扩展排队负载、传染病异常提醒等\n");
    printf("======================================================\n");
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
                // 传染病隔离病房单独统计
                if (ward_curr->is_occupied)
                    occupied_beds++;
            }
            
            ward_curr = ward_curr->next;
        }
        
        free_beds = total_beds - occupied_beds;
        
        // 显示床位资源预警摘要
        printf("【床位资源预警摘要】\n");
        printf("------------------------------------------------------\n");
        printf("总床位数：%d\n", total_beds);
        printf("已占用床位：%d\n", occupied_beds);
        printf("空闲床位：" RED "%d" RESET "\n", free_beds);
        printf("床位使用率：%.2f%%\n", total_beds > 0 ? (float)occupied_beds / total_beds * 100 : 0);
        printf("------------------------------------------------------\n");
        
        // 显示各类型病房床位情况
        printf("【各类型病房床位情况】\n");
        printf("------------------------------------------------------\n");
        printf("普通病房：已占用 %d, 空闲 " RED "%d" RESET "\n", general_occupied, general_free);
        printf("特殊病房：已占用 %d, 空闲 " RED "%d" RESET "\n", special_occupied, special_free);
        printf("ICU：已占用 %d, 空闲 " RED "%d" RESET "\n", icu_occupied, icu_free);
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
    
    // 输入要处理的投诉编号
    char complaint_id[MAX_ID_LEN];
    get_safe_string("请输入要处理的投诉工单编号: ", complaint_id, MAX_ID_LEN);
    
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
        printf("\n⚠️ 未找到该投诉工单！\n");
        return;
    }
    
    if (target_complaint->status != 0)
    {
        printf("\n⚠️ 该投诉工单已经处理过了！\n");
        return;
    }
    
    // 输入处理意见
    char response[MAX_RECORD_LEN];
    get_safe_string("请输入处理意见: ", response, MAX_RECORD_LEN);
    
    // 更新投诉状态和回复
    target_complaint->status = 1;
    strncpy(target_complaint->response, response, MAX_RECORD_LEN - 1);
    target_complaint->response[MAX_RECORD_LEN - 1] = '\0';
    
    printf("\n✅ 投诉处理成功！\n");
    printf("工单编号：%s\n", target_complaint->complaint_id);
    printf("处理意见：%s\n", target_complaint->response);
}

// 按患者编号查询投诉历史
void query_patient_complaints_by_id()
{
    char patient_id[MAX_ID_LEN];
    get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
    
    // 调用现有的 query_patient_complaints 函数
    query_patient_complaints(patient_id);
}

// 按投诉编号查询投诉详情
void query_complaint_by_id()
{
    char complaint_id[MAX_ID_LEN];
    get_safe_string("请输入投诉编号: ", complaint_id, MAX_ID_LEN);
    
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
    int doctor_count = 0;
    int pending_count = 0;
    int recheck_count = 0;
    int completed_count = 0;
    int no_show_count = 0;
    int wait_med_count = 0;
    int occupied_beds = 0;
    int total_beds = 0;
    int free_beds = 0;

    DoctorNode* doctor_curr = NULL;
    PatientNode* patient_curr = NULL;
    
    // 用于存储科室信息的临时链表
    DeptStatNode* dept_list = NULL;
    DeptStatNode* dept_tail = NULL;

    // 统计医生总数和各医生排队人数
    if (g_doctor_list != NULL && g_doctor_list->next != NULL)
    {
        doctor_curr = g_doctor_list->next;
        while (doctor_curr != NULL)
        {
            doctor_count++;
            
            // 统计科室信息
            DeptStatNode* existing_dept = dept_list;
            int found = 0;
            while (existing_dept != NULL)
            {
                if (strcmp(existing_dept->department, doctor_curr->department) == 0)
                {
                    existing_dept->doctor_count++;
                    existing_dept->queue_length += doctor_curr->queue_length;
                    found = 1;
                    break;
                }
                existing_dept = existing_dept->next;
            }
            if (!found)
            {
                // 创建新的科室统计节点
                DeptStatNode* new_dept = (DeptStatNode*)malloc(sizeof(DeptStatNode));
                if (new_dept != NULL)
                {
                    strcpy(new_dept->department, doctor_curr->department);
                    new_dept->doctor_count = 1;
                    new_dept->queue_length = doctor_curr->queue_length;
                    new_dept->next = NULL;

                    // 添加到链表
                    if (dept_list == NULL)
                    {
                        dept_list = new_dept;
                        dept_tail = new_dept;
                    }
                    else
                    {
                        dept_tail->next = new_dept;
                        dept_tail = new_dept;
                    }
                }
            }
            
            doctor_curr = doctor_curr->next;
        }
    }

    // 统计患者相关数据
    if (g_patient_list != NULL && g_patient_list->next != NULL)
    {
        patient_curr = g_patient_list->next;
        while (patient_curr != NULL)
        {
            switch (patient_curr->status)
            {
                case STATUS_PENDING:
                    pending_count++;
                    break;
                case STATUS_RECHECK_PENDING:
                    recheck_count++;
                    break;
                case STATUS_NO_SHOW:
                    no_show_count++;
                    break;
                case STATUS_COMPLETED:
                    completed_count++;
                    break;
                case STATUS_WAIT_MED:
                    wait_med_count++;
                    break;
                default:
                    break;
            }
            patient_curr = patient_curr->next;
        }
    }

    // 统计床位使用情况
    if (g_ward_list != NULL && g_ward_list->next != NULL)
    {
        WardNode* ward_curr = g_ward_list->next;
        while (ward_curr != NULL)
        {
            total_beds++;
            if (ward_curr->is_occupied)
                occupied_beds++;
            ward_curr = ward_curr->next;
        }
        free_beds = total_beds - occupied_beds;
    }

    // 显示总体负载摘要
    printf("\n================================================================================\n");
    printf("                              负载监控查看\n");
    printf("================================================================================\n");
    printf("【总体负载摘要】\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("%-20s %-10d\n", "医生总数：", doctor_count);
    printf("%-20s %-10d\n", "总待诊患者数：", pending_count);
    printf("%-20s %-10d\n", "总待复诊患者数：", recheck_count);
    printf("%-20s %-10d\n", "总待发药患者数：", wait_med_count);
    printf("%-20s %-10d\n", "总病床数：", total_beds);
    printf("%-20s %-10d\n", "已占用病床数：", occupied_beds);
    printf("%-20s %-10d\n", "空闲病床数：", free_beds);
    printf("================================================================================\n");

    // 显示各医生负载明细
    printf("【各医生负载明细】\n");
    printf("--------------------------------------------------------------------------------\n");
    print_field("医生工号", 10); print_field("姓名", 8); print_field("性别", 4); printf(" ");
    print_field("科室", 16); printf("%-8s %-6s\n", "排队人数", "值班状态");
    printf("--------------------------------------------------------------------------------\n");
    if (g_doctor_list != NULL && g_doctor_list->next != NULL)
    {
        doctor_curr = g_doctor_list->next;
        while (doctor_curr != NULL)
        {
            print_field(doctor_curr->id, 10);
            print_field(doctor_curr->name, 8);
            print_field(strlen(doctor_curr->gender) > 0 ? doctor_curr->gender : "-", 4);
            printf(" ");
            print_field(doctor_curr->department, 16);
            printf("%-8d ", doctor_curr->queue_length);
            print_field(doctor_curr->is_on_duty ? "值班中" : "未值班", 6);
            printf("\n");
            doctor_curr = doctor_curr->next;
        }
    }
    else
    {
        printf("当前无医生数据\n");
    }
    printf("--------------------------------------------------------------------------------\n");

    // 显示各科室负载汇总
    printf("【各科室负载汇总】\n");
    printf("--------------------------------------------------------------------------------\n");
    print_field("科室名称", 16); printf("%-10s %-10s\n", "医生人数", "总排队人数");
    printf("--------------------------------------------------------------------------------\n");
    if (dept_list != NULL)
    {
        DeptStatNode* curr_dept = dept_list;
        while (curr_dept != NULL)
        {
            print_field(curr_dept->department, 16);
            printf("%-10d %-10d\n",
                curr_dept->doctor_count,
                curr_dept->queue_length);
            curr_dept = curr_dept->next;
        }
    }
    else
    {
        printf("当前无科室数据\n");
    }
    printf("================================================================================\n");

    // 显示床位占用明细
    printf("【床位占用】\n");
    printf("--------------------------------------------------------------------------------\n");
    if (total_beds > 0)
    {
        int general_occ = 0, general_total = 0;
        int single_occ = 0, single_total = 0;
        int icu_occ = 0, icu_total = 0;
        int isolation_occ = 0, isolation_total = 0;
        WardNode* wc = g_ward_list->next;
        while (wc != NULL)
        {
            switch (wc->ward_type)
            {
                case WARD_TYPE_GENERAL:  general_total++; if(wc->is_occupied) general_occ++; break;
                case WARD_TYPE_SINGLE:    single_total++;  if(wc->is_occupied) single_occ++;  break;
                case WARD_TYPE_ICU:       icu_total++;     if(wc->is_occupied) icu_occ++;     break;
                case WARD_TYPE_ISOLATION: isolation_total++; if(wc->is_occupied) isolation_occ++; break;
                default: break;
            }
            wc = wc->next;
        }
        printf("床位使用率：%d / %d (%.1f%%)\n", occupied_beds, total_beds,
            100.0f * occupied_beds / total_beds);
        printf("--------------------------------------------------------------------------------\n");
        print_field("病房类型", 12); printf("%-10s %-10s\n", "已占用", "总数");
        printf("--------------------------------------------------------------------------------\n");
        if(general_total>0)  { print_field("普通病房", 12); printf("%-10d %-10d\n", general_occ, general_total); }
        if(single_total>0)   { print_field("单人病房", 12); printf("%-10d %-10d\n", single_occ, single_total); }
        if(icu_total>0)      { print_field("ICU", 12);      printf("%-10d %-10d\n", icu_occ, icu_total); }
        if(isolation_total>0){ print_field("隔离病房", 12); printf("%-10d %-10d\n", isolation_occ, isolation_total); }
        if (free_beds <= 3)
            printf("\n  " RED "床位紧张，仅剩 %d 张空闲床位！" RESET "\n", free_beds);
    }
    else
    {
        printf("当前无床位数据\n");
    }
    printf("================================================================================\n");

    // 显示待发药患者列表
    printf("【待发药患者】\n");
    printf("--------------------------------------------------------------------------------\n");
    if (wait_med_count > 0 && g_patient_list != NULL && g_patient_list->next != NULL)
    {
        patient_curr = g_patient_list->next;
        int med_idx = 0;
        while (patient_curr != NULL)
        {
            if (patient_curr->status == STATUS_WAIT_MED)
            {
                med_idx++;
                printf("%-3d %-10s %-10s %-16s\n", med_idx, patient_curr->id,
                    patient_curr->name, patient_curr->diagnosis_text[0] ? patient_curr->diagnosis_text : "-");
            }
            patient_curr = patient_curr->next;
        }
    }
    else
    {
        printf("当前无待发药患者\n");
    }
    printf("================================================================================\n");

    printf("提示：此面板数据实时更新，可作为系统运行监控参考。\n");
    printf("================================================================================\n");
    
    // 释放科室链表
    DeptStatNode* curr_dept = dept_list;
    while (curr_dept != NULL)
    {
        DeptStatNode* temp = curr_dept;
        curr_dept = curr_dept->next;
        free(temp);
    }
}

// 显示公共状态统计摘要
void show_public_status_statistics(void)
{
    int patient_count = 0;
    int pending_count = 0;
    int recheck_count = 0;
    int examining_count = 0;
    int unpaid_count = 0;
    int paid_count = 0;
    int no_show_count = 0;
    int completed_count = 0;
    int doctor_count = 0;
    int nurse_count = 0;
    int pharmacist_count = 0;

    PatientNode* patient_curr = NULL;
    DoctorNode* doctor_curr = NULL;
    AccountNode* account_curr = NULL;

    // 统计患者相关数据
    if (g_patient_list != NULL && g_patient_list->next != NULL)
    {
        patient_curr = g_patient_list->next;
        while (patient_curr != NULL)
        {
            patient_count++;
            switch (patient_curr->status)
            {
                case STATUS_PENDING:
                    pending_count++;
                    break;
                case STATUS_RECHECK_PENDING:
                    recheck_count++;
                    break;
                case STATUS_EXAMINING:
                    examining_count++;
                    break;
                case STATUS_UNPAID:
                    unpaid_count++;
                    break;
                case STATUS_WAIT_MED:
                    paid_count++;
                    break;
                case STATUS_NO_SHOW:
                    no_show_count++;
                    break;
                case STATUS_COMPLETED:
                    completed_count++;
                    break;
                default:
                    break;
            }
            patient_curr = patient_curr->next;
        }
    }

    // 统计医生总数
    if (g_doctor_list != NULL && g_doctor_list->next != NULL)
    {
        doctor_curr = g_doctor_list->next;
        while (doctor_curr != NULL)
        {
            doctor_count++;
            doctor_curr = doctor_curr->next;
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
    printf("2. 待诊患者数：%d\n", pending_count);
    printf("3. 待复诊患者数：%d\n", recheck_count);
    printf("4. 检查中患者数：%d\n", examining_count);
    printf("5. 待缴费患者数：%d\n", unpaid_count);
    printf("6. 已缴费待取药患者数：%d\n", paid_count);
    printf("7. 过号作废患者数：%d\n", no_show_count);
    printf("8. 已完成患者数：%d\n", completed_count);
    printf("9. 医生总数：%d\n", doctor_count);
    printf("10. 护士总数：%d\n", nurse_count);
    printf("11. 药师总数：%d\n", pharmacist_count);
    printf("======================================================\n");
    printf("提示：此面板数据实时更新，仅供参考。\n");
    printf("======================================================\n");
}

// 显示所有护士值班状态
void show_all_nurses_with_duty_status(void)
{
    int nurse_count = 0;
    AccountNode* account_curr = NULL;

    // 显示护士值班状态
    printf("\n======================================================\n");
    printf("                  护士值班状态\n");
    printf("======================================================\n");
    printf("【护士列表】\n");
    printf("------------------------------------------------------\n");
    printf("工号            姓名            值班状态\n");
    printf("------------------------------------------------------\n");

    if (g_account_list != NULL && g_account_list->next != NULL)
    {
        account_curr = g_account_list->next;
        while (account_curr != NULL)
        {
            if (account_curr->role == ROLE_NURSE)
            {
                nurse_count++;
                printf("%-14s %-14s %-10s\n", 
                    account_curr->username, 
                    account_curr->real_name, 
                    account_curr->is_on_duty ? "值班中" : "未值班");
            }
            account_curr = account_curr->next;
        }
    }

    if (nurse_count == 0)
    {
        printf("当前无护士数据\n");
    }

    printf("------------------------------------------------------\n");
    printf("护士总数：%d\n", nurse_count);
    printf("======================================================\n");
}

// 更新护士值班状态
int update_nurse_duty_status(const char* username, int new_status)
{
    AccountNode* account_curr = NULL;
    int found = 0;
    char description[200];

    if (g_account_list == NULL)
    {
        printf("提示：账号链表尚未初始化，无法更新值班状态。\n");
        return 0;
    }

    if (is_blank_string(username))
    {
        printf("提示：护士用户名不能为空。\n");
        return 0;
    }

    if (new_status != 0 && new_status != 1)
    {
        printf("提示：值班状态必须是 0（未值班）或 1（值班中）。\n");
        return 0;
    }

    if (g_account_list != NULL && g_account_list->next != NULL)
    {
        account_curr = g_account_list->next;
        while (account_curr != NULL)
        {
            if (strcmp(account_curr->username, username) == 0)
            {
                if (account_curr->role != ROLE_NURSE)
                {
                    printf("提示：目标账号不是护士角色，更新失败。\n");
                    return 0;
                }
                account_curr->is_on_duty = new_status;
                found = 1;
                // 记录日志
                snprintf(description, sizeof(description), "护士：%s，新状态：%s", account_curr->real_name, new_status ? "值班中" : "未值班");
                add_log("护士值班状态修改", account_curr->username, description);
                break;
            }
            account_curr = account_curr->next;
        }
    }

    if (!found)
    {
        printf("提示：未找到对应护士账号，更新失败。\n");
        return 0;
    }

    return found;
}

// 显示传染病异常提醒
void show_infectious_disease_alerts(void)
{
    int patient_count = 0;
    int symptomatic_count = 0;
    int asymptomatic_count = 0;
    int fever_count = 0;
    int cough_count = 0;
    int diarrhea_count = 0;
    PatientNode* patient_curr = NULL;
    
    // 预警阈值常量
    const int FEVER_THRESHOLD = 5;
    const int COUGH_THRESHOLD = 8;
    const int DIARRHEA_THRESHOLD = 3;

    // 统计患者相关数据
    if (g_patient_list != NULL && g_patient_list->next != NULL)
    {
        patient_curr = g_patient_list->next;
        while (patient_curr != NULL)
        {
            patient_count++;
            
            // 检查是否有症状描述
            if (strlen(patient_curr->symptom) > 0)
            {
                symptomatic_count++;
                // 简单基于症状描述进行判断
                if (strstr(patient_curr->symptom, "发热") != NULL || strstr(patient_curr->symptom, "发烧") != NULL)
                {
                    fever_count++;
                }
                if (strstr(patient_curr->symptom, "咳嗽") != NULL)
                {
                    cough_count++;
                }
                if (strstr(patient_curr->symptom, "腹泻") != NULL)
                {
                    diarrhea_count++;
                }
            }
            else
            {
                asymptomatic_count++;
            }
            
            patient_curr = patient_curr->next;
        }
    }

    // 显示传染病异常提醒
    printf("\n======================================================\n");
    printf("                传染病异常提醒\n");
    printf("======================================================\n");
    printf("【异常提醒摘要】\n");
    printf("------------------------------------------------------\n");
    printf("患者总数：%d\n", patient_count);
    printf("有症状描述的患者数：%d\n", symptomatic_count);
    printf("无症状描述的患者数：%d\n", asymptomatic_count);
    printf("发热症状患者数：%d\n", fever_count);
    printf("咳嗽症状患者数：%d\n", cough_count);
    printf("腹泻症状患者数：%d\n", diarrhea_count);
    printf("------------------------------------------------------\n");

    // 简单的异常判断
    if (fever_count > FEVER_THRESHOLD)
    {
        printf("⚠️ 预警：发热症状患者数量较多，请关注是否有传染病流行。\n");
    }
    if (cough_count > COUGH_THRESHOLD)
    {
        printf("⚠️ 预警：咳嗽症状患者数量较多，请关注呼吸道传染病。\n");
    }
    if (diarrhea_count > DIARRHEA_THRESHOLD)
    {
        printf("⚠️ 预警：腹泻症状患者数量较多，请关注肠道传染病。\n");
    }

    printf("------------------------------------------------------\n");
    printf("提示：当前版本基于症状关键词做基础预警，后续将完善\n");
    printf("基于流行病学数据的传染病监测与预警功能。\n");
    printf("======================================================\n");
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
    printf("提示：输入 'B' 返回上一步，第一步输入 'B' 退出\n\n");

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
                    if (!get_form_string("请输入别名（可留空，输入 B 返回上一步）: ", alias, MAX_ALIAS_LEN))
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
                    if (!get_form_int("请输入医保类型编号（输入 B 返回上一步）: ", &medicare_type, 0, 2, "医保类型输入非法，请重新输入\n"))
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

    printf("\n================ 修改药品基础信息 ===============-\n");

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
            printf("未找到编号为 %s 的药品，修改流程结束\n", med_id);
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

    // 6. 效期（选填，空白字符串表示不修改）
    get_safe_string("请输入新效期（留空不修改，输入 B 返回上一级）: ", new_expiry_date, MAX_DATE_LEN);
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
    printf("提示：输入 'B' 可返回上一级菜单\n\n");

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
    printf("输入药品编号或关键词查询，输入 Q 退出\n\n");

    while (1)
    {
        printf("请输入药品编号或关键词: ");
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
    threshold = get_safe_int("请输入低库存阈值: ");
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

void handle_medicine_remove(void)
{
    char med_id[MAX_ID_LEN];
    MedicineNode* medicine = NULL;
    int confirm;

    printf("\n================ 下架药品 ===============-\n");
    get_safe_string("请输入要下架的药品编号: ", med_id, MAX_ID_LEN);

    // 查找药品是否存在
    medicine = find_medicine_by_id(g_medicine_list, med_id);
    if (medicine == NULL)
    {
        printf("提示：未找到对应药品，下架失败。\n");
        return;
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
    LogNode* new_log = NULL;
    LogNode* curr = NULL;
    time_t now;
    struct tm* local_time;

    // 初始化日志列表
    init_log_list();

    // 创建新日志节点
    new_log = (LogNode*)malloc(sizeof(LogNode));
    if (new_log == NULL)
    {
        return;
    }

    // 填充日志信息
    // 获取当前时间
    time(&now);
    local_time = localtime(&now);
    strftime(new_log->timestamp, sizeof(new_log->timestamp), "%Y-%m-%d %H:%M:%S", local_time);

    // 复制操作、目标和描述
    if (operation != NULL)
    {
        strncpy(new_log->operation, operation, sizeof(new_log->operation) - 1);
        new_log->operation[sizeof(new_log->operation) - 1] = '\0';
    }
    else
    {
        new_log->operation[0] = '\0';
    }

    if (target != NULL)
    {
        strncpy(new_log->target, target, sizeof(new_log->target) - 1);
        new_log->target[sizeof(new_log->target) - 1] = '\0';
    }
    else
    {
        new_log->target[0] = '\0';
    }

    if (description != NULL)
    {
        strncpy(new_log->description, description, sizeof(new_log->description) - 1);
        new_log->description[sizeof(new_log->description) - 1] = '\0';
    }
    else
    {
        new_log->description[0] = '\0';
    }

    // 添加到链表尾部
    curr = g_log_list;
    while (curr->next != NULL)
    {
        curr = curr->next;
    }
    curr->next = new_log;
    new_log->next = NULL;
}

// 显示系统日志
void show_system_logs(void)
{
    LogNode* curr = NULL;
    int count = 0;

    if (g_log_list == NULL || g_log_list->next == NULL)
    {
        printf("当前暂无系统日志\n");
        return;
    }

    printf("\n==============================================================\n");
    printf("                        系统日志\n");
    printf("==============================================================\n");
    print_field("时间", 20); print_field("操作", 16); print_field("目标", 8); printf("描述\n");
    printf("--------------------------------------------------------------\n");

    curr = g_log_list->next;
    while (curr != NULL)
    {
        printf("%-20s", curr->timestamp);
        print_field(curr->operation, 16);
        printf("%-8s %s\n", curr->target, curr->description);
        count++;
        curr = curr->next;
    }

    printf("--------------------------------------------------------------\n");
    printf("日志总数：%d\n", count);
    printf("==============================================================\n");
}

// 清理日志（保留最近 N 条）
void clean_logs(int keep_count)
{
    LogNode* curr = NULL;
    LogNode* prev = NULL;
    LogNode* temp = NULL;
    int count = 0;
    int to_delete = 0;

    if (g_log_list == NULL || g_log_list->next == NULL)
    {
        return;
    }

    // 统计日志总数
    curr = g_log_list->next;
    while (curr != NULL)
    {
        count++;
        curr = curr->next;
    }

    // 计算需要删除的日志数量
    to_delete = count - keep_count;
    if (to_delete <= 0)
    {
        return;
    }

    // 删除最旧的 to_delete 条日志
    curr = g_log_list->next;
    prev = g_log_list;
    while (curr != NULL && to_delete > 0)
    {
        temp = curr;
        curr = curr->next;
        prev->next = curr;
        free(temp);
        to_delete--;
    }
}

// 显示医保类型统计
void show_medicare_statistics(void)
{
    MedicineNode* curr = NULL;
    int total_count = 0;
    int none_count = 0;
    int class_a_count = 0;
    int class_b_count = 0;

    if (g_medicine_list == NULL || g_medicine_list->next == NULL)
    {
        printf("当前暂无药品数据\n");
        return;
    }

    curr = g_medicine_list->next;
    while (curr != NULL)
    {
        total_count++;
        switch (curr->m_type)
        {
            case MEDICARE_NONE:
                none_count++;
                break;
            case MEDICARE_CLASS_A:
                class_a_count++;
                break;
            case MEDICARE_CLASS_B:
                class_b_count++;
                break;
            default:
                break;
        }
        curr = curr->next;
    }

    printf("\n======================================================\n");
    printf("                  医保类型统计\n");
    printf("======================================================\n");
    printf("药品总数：%d\n", total_count);
    printf("非医保药品数：%d (%.1f%%)\n", none_count, total_count > 0 ? (float)none_count / total_count * 100 : 0);
    printf("甲类医保药品数：%d (%.1f%%)\n", class_a_count, total_count > 0 ? (float)class_a_count / total_count * 100 : 0);
    printf("乙类医保药品数：%d (%.1f%%)\n", class_b_count, total_count > 0 ? (float)class_b_count / total_count * 100 : 0);
    printf("======================================================\n");
}

// 显示药品价格分布
void show_price_distribution(void)
{
    MedicineNode* curr = NULL;
    int total_count = 0;
    int price_0_50 = 0;
    int price_50_100 = 0;
    int price_100_500 = 0;
    int price_500_1000 = 0;
    int price_1000_plus = 0;

    if (g_medicine_list == NULL || g_medicine_list->next == NULL)
    {
        printf("当前暂无药品数据\n");
        return;
    }

    curr = g_medicine_list->next;
    while (curr != NULL)
    {
        total_count++;
        if (curr->price < 50)
        {
            price_0_50++;
        }
        else if (curr->price < 100)
        {
            price_50_100++;
        }
        else if (curr->price < 500)
        {
            price_100_500++;
        }
        else if (curr->price < 1000)
        {
            price_500_1000++;
        }
        else
        {
            price_1000_plus++;
        }
        curr = curr->next;
    }

    printf("\n======================================================\n");
    printf("                  药品价格分布\n");
    printf("======================================================\n");
    printf("药品总数：%d\n", total_count);
    printf("0-50元：%d (%.1f%%)\n", price_0_50, total_count > 0 ? (float)price_0_50 / total_count * 100 : 0);
    printf("50-100元：%d (%.1f%%)\n", price_50_100, total_count > 0 ? (float)price_50_100 / total_count * 100 : 0);
    printf("100-500元：%d (%.1f%%)\n", price_100_500, total_count > 0 ? (float)price_100_500 / total_count * 100 : 0);
    printf("500-1000元：%d (%.1f%%)\n", price_500_1000, total_count > 0 ? (float)price_500_1000 / total_count * 100 : 0);
    printf("1000元以上：%d (%.1f%%)\n", price_1000_plus, total_count > 0 ? (float)price_1000_plus / total_count * 100 : 0);
    printf("======================================================\n");
}

// 显示系统日志
void show_logs(void)
{
    show_system_logs();
}

// 处理近效期药品检查
void handle_expiring_medicine_check(void)
{
    handle_medicine_expiring();
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

    printf("\n==============================================================\n");
    printf("                    检查项目字典\n");
    printf("==============================================================\n");
    printf("%-10s %-20s %-12s %10s %8s\n",
           "项目编号", "项目名称", "所属科室", "价格(元)", "医保类型");
    printf("--------------------------------------------------------------\n");

    curr = g_check_item_list->next;
    while (curr != NULL)
    {
        const char* m_type_name = NULL;
        switch (curr->m_type)
        {
            case MEDICARE_CLASS_A:
                m_type_name = "甲类";
                break;
            case MEDICARE_CLASS_B:
                m_type_name = "乙类";
                break;
            case MEDICARE_NONE:
                m_type_name = "自费";
                break;
            default:
                m_type_name = "未知";
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
    printf("检查项目总数：%d\n", count);
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
        printf("提示：查询关键字不能为空。\n");
        return;
    }

    if (g_check_item_list == NULL || g_check_item_list->next == NULL)
    {
        printf("未找到匹配检查项目\n");
        return;
    }

    printf("\n================ 检查项目查询结果 ================\n");
    printf("%-10s %-20s %-12s %10s %8s\n",
           "项目编号", "项目名称", "所属科室", "价格(元)", "医保类型");
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
                case MEDICARE_CLASS_A: m_type_name = "甲类"; break;
                case MEDICARE_CLASS_B: m_type_name = "乙类"; break;
                case MEDICARE_NONE:    m_type_name = "自费"; break;
                default:               m_type_name = "未知"; break;
            }
            printf("%-10s %-20s %-12s %10.2f %8s\n",
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
    char item_name[MAX_NAME_LEN];
    char dept[MAX_NAME_LEN];
    double price;
    int medicare_type;
    char new_id[MAX_ID_LEN];

    printf("\n================ 新增检查项目 ===============-\n");
    printf("提示：任意输入项输入 'B' 可返回上一级菜单\n\n");

    while (1)
    {
        if (!get_form_string("请输入检查项目名称（输入 B 返回上一级）: ", item_name, MAX_NAME_LEN))
            return;
        if (is_blank_string(item_name))
        {
            printf("检查项目名称不能为空，请重新输入\n");
            continue;
        }
        break;
    }

    while (1)
    {
        if (!get_form_string("请输入所属科室（输入 B 返回上一级）: ", dept, MAX_NAME_LEN))
            return;
        if (is_blank_string(dept))
        {
            printf("所属科室不能为空，请重新输入\n");
            continue;
        }
        break;
    }

    while (1)
    {
        if (!get_form_double("请输入检查费用（输入 B 返回上一级）: ", &price, 0, "检查费用必须大于 0，请重新输入\n"))
            return;
        break;
    }

    printf("医保类型：0=自费 1=甲类 2=乙类\n");
    while (1)
    {
        if (!get_form_int("请输入医保类型编号（输入 B 返回上一级）: ", &medicare_type, 0, 2, "医保类型输入非法，请重新输入\n"))
            return;
        break;
    }

    if (g_check_item_list == NULL)
    {
        printf("提示：检查项目链表尚未初始化，无法新增。\n");
        return;
    }

    generate_check_item_id(new_id);
    CheckItemNode* new_node = create_check_item_node(new_id, item_name, dept, price, (MedicareType)medicare_type);
    if (new_node == NULL)
    {
        printf("提示：检查项目节点创建失败。\n");
        return;
    }

    insert_check_item_tail(g_check_item_list, new_node);
    printf("检查项目新增成功！\n");
    printf("项目编号：%s\n", new_node->item_id);
    printf("项目名称：%s\n", new_node->item_name);
    printf("所属科室：%s\n", new_node->dept);
    printf("检查费用：%.2f\n", new_node->price);
    printf("医保类型：%s\n",
           new_node->m_type == MEDICARE_CLASS_A ? "甲类" :
           new_node->m_type == MEDICARE_CLASS_B ? "乙类" : "自费");
}

void handle_check_item_update(void)
{
    char item_id[MAX_ID_LEN];
    char new_name[MAX_NAME_LEN];
    char new_dept[MAX_NAME_LEN];
    double new_price = -1.0;
    int new_m_type = -1;

    printf("\n================ 修改检查项目信息 ===============-\n");

    CheckItemNode* target = NULL;
    while (1)
    {
        get_safe_string("请输入检查项目编号（输入 B 返回上一级）: ", item_id, MAX_ID_LEN);
        if (strcmp(item_id, "B") == 0 || strcmp(item_id, "b") == 0) return;
        if (is_blank_string(item_id))
        {
            printf("检查项目编号不能为空，请重新输入\n");
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
        case MEDICARE_CLASS_A: printf("医保类型：甲类\n"); break;
        case MEDICARE_CLASS_B: printf("医保类型：乙类\n"); break;
        case MEDICARE_NONE:    printf("医保类型：自费\n"); break;
    }
    printf("----------------------------------------\n");

    get_safe_string("请输入新项目名称（留空不修改，输入 B 返回上一级）: ", new_name, MAX_NAME_LEN);
    if (strcmp(new_name, "B") == 0 || strcmp(new_name, "b") == 0) return;

    get_safe_string("请输入新所属科室（留空不修改，输入 B 返回上一级）: ", new_dept, MAX_NAME_LEN);
    if (strcmp(new_dept, "B") == 0 || strcmp(new_dept, "b") == 0) return;

    while (1)
    {
        char price_str[32];
        get_safe_string("请输入新检查费用（留空不修改，输入 B 返回上一级）: ", price_str, sizeof(price_str));
        if (strcmp(price_str, "B") == 0 || strcmp(price_str, "b") == 0) return;
        if (is_blank_string(price_str)) break;
        char* endptr = NULL;
        new_price = strtod(price_str, &endptr);
        if (endptr == price_str || *endptr != '\0' || new_price <= 0)
        {
            printf("检查费用必须为大于 0 的数字，请重新输入\n");
            continue;
        }
        break;
    }

    printf("医保类型：0=自费 1=甲类 2=乙类（-1 不修改）\n");
    while (1)
    {
        char type_str[32];
        get_safe_string("请输入新医保类型（留空不修改，输入 B 返回上一级）: ", type_str, sizeof(type_str));
        if (strcmp(type_str, "B") == 0 || strcmp(type_str, "b") == 0) return;
        if (is_blank_string(type_str)) break;
        char* endptr = NULL;
        new_m_type = (int)strtol(type_str, &endptr, 10);
        if (endptr == type_str || *endptr != '\0' || new_m_type < 0 || new_m_type > 2)
        {
            printf("医保类型必须为 0/1/2，请重新输入\n");
            continue;
        }
        break;
    }

    if (!is_blank_string(new_name)) strncpy(target->item_name, new_name, MAX_NAME_LEN - 1);
    if (!is_blank_string(new_dept)) strncpy(target->dept, new_dept, MAX_NAME_LEN - 1);
    if (new_price > 0) target->price = new_price;
    if (new_m_type >= 0) target->m_type = (MedicareType)new_m_type;

    printf("\n提示：检查项目信息修改成功。\n");
}

void handle_check_item_remove(void)
{
    char item_id[MAX_ID_LEN];
    CheckItemNode* target = NULL;
    int confirm;

    printf("\n================ 下架检查项目 ===============-\n");
    get_safe_string("请输入要下架的检查项目编号: ", item_id, MAX_ID_LEN);

    target = find_check_item_by_id(g_check_item_list, item_id);
    if (target == NULL)
    {
        printf("提示：未找到对应检查项目，下架失败。\n");
        return;
    }

    printf("\n当前检查项目信息：\n");
    printf("项目编号：%s\n", target->item_id);
    printf("项目名称：%s\n", target->item_name);
    printf("所属科室：%s\n", target->dept);
    printf("检查费用：%.2f\n", target->price);
    switch (target->m_type)
    {
        case MEDICARE_CLASS_A: printf("医保类型：甲类\n"); break;
        case MEDICARE_CLASS_B: printf("医保类型：乙类\n"); break;
        case MEDICARE_NONE:    printf("医保类型：自费\n"); break;
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