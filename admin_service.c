#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "admin_service.h"
#include "list_ops.h"
#include "medicine_service.h"
#include "pharmacy_service.h"
#include "utils.h"

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
int register_account(const char* username, const char* password, const char* real_name, RoleType role)
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

    new_account = create_account_node(username, password, real_name, role);
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
    sprintf(description, "真实姓名：%s，角色：%s", real_name, 
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
            sprintf(description, "修改了姓名、密码和角色");
        }
        else if (name_changed && password_changed)
        {
            sprintf(description, "修改了姓名和密码");
        }
        else if (name_changed && role_changed)
        {
            sprintf(description, "修改了姓名和角色");
        }
        else if (password_changed && role_changed)
        {
            sprintf(description, "修改了密码和角色");
        }
        else if (name_changed)
        {
            sprintf(description, "修改了姓名");
        }
        else if (password_changed)
        {
            sprintf(description, "修改了密码");
        }
        else if (role_changed)
        {
            sprintf(description, "修改了角色");
        }
        else
        {
            sprintf(description, "修改了资料");
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

    printf("\n==============================================================\n");
    printf("                    医生值班状态\n");
    printf("==============================================================\n");
    printf("医生工号            姓名            科室            值班状态\n");
    printf("--------------------------------------------------------------\n");

    curr = g_doctor_list->next;
    while (curr != NULL)
    {
        printf("%-18s %-14s %-16s %-8s\n", 
            curr->id, 
            curr->name, 
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
    sprintf(description, "医生：%s，新状态：%s", doctor->name, new_status ? "值班中" : "未值班");
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

// 显示负载监控查看
void show_load_monitoring(void)
{
    int doctor_count = 0;
    int pending_count = 0;
    int recheck_count = 0;
    int completed_count = 0;
    int no_show_count = 0;
    int dept_count = 0;
    int i;
    int found;

    DoctorNode* doctor_curr = NULL;
    PatientNode* patient_curr = NULL;
    
    // 用于存储科室信息的临时数组
    char departments[50][MAX_NAME_LEN];
    int dept_doctor_counts[50];
    int dept_queue_lengths[50];

    // 初始化科室数组
    for (i = 0; i < 50; i++)
    {
        departments[i][0] = '\0';
        dept_doctor_counts[i] = 0;
        dept_queue_lengths[i] = 0;
    }

    // 统计医生总数和各医生排队人数
    if (g_doctor_list != NULL && g_doctor_list->next != NULL)
    {
        doctor_curr = g_doctor_list->next;
        while (doctor_curr != NULL)
        {
            doctor_count++;
            
            // 统计科室信息
            found = 0;
            for (i = 0; i < dept_count; i++)
            {
                if (strcmp(departments[i], doctor_curr->department) == 0)
                {
                    dept_doctor_counts[i]++;
                    dept_queue_lengths[i] += doctor_curr->queue_length;
                    found = 1;
                    break;
                }
            }
            if (!found && dept_count < 50)
            {
                strcpy(departments[dept_count], doctor_curr->department);
                dept_doctor_counts[dept_count] = 1;
                dept_queue_lengths[dept_count] = doctor_curr->queue_length;
                dept_count++;
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
                default:
                    break;
            }
            patient_curr = patient_curr->next;
        }
    }

    // 显示总体负载摘要
    printf("\n======================================================\n");
    printf("                  负载监控查看\n");
    printf("======================================================\n");
    printf("【总体负载摘要】\n");
    printf("------------------------------------------------------\n");
    printf("医生总数：%d\n", doctor_count);
    printf("总待诊患者数：%d\n", pending_count);
    printf("总待复诊患者数：%d\n", recheck_count);
    printf("总过号作废患者数：%d\n", no_show_count);
    printf("总已完成患者数：%d\n", completed_count);
    printf("======================================================\n");

    // 显示各医生负载明细
    printf("【各医生负载明细】\n");
    printf("------------------------------------------------------\n");
    printf("医生工号            姓名            科室            当前排队人数    值班状态\n");
    printf("------------------------------------------------------\n");
    if (g_doctor_list != NULL && g_doctor_list->next != NULL)
    {
        doctor_curr = g_doctor_list->next;
        while (doctor_curr != NULL)
        {
            printf("%-18s %-14s %-16s %-16d %-8s\n", 
                doctor_curr->id, 
                doctor_curr->name, 
                doctor_curr->department, 
                doctor_curr->queue_length, 
                doctor_curr->is_on_duty ? "值班中" : "未值班");
            doctor_curr = doctor_curr->next;
        }
    }
    else
    {
        printf("当前无医生数据\n");
    }
    printf("------------------------------------------------------\n");

    // 显示各科室负载汇总
    printf("【各科室负载汇总】\n");
    printf("------------------------------------------------------\n");
    printf("科室名称            医生人数    科室总排队人数\n");
    printf("------------------------------------------------------\n");
    if (dept_count > 0)
    {
        for (i = 0; i < dept_count; i++)
        {
            printf("%-18s %-10d %-16d\n", 
                departments[i], 
                dept_doctor_counts[i], 
                dept_queue_lengths[i]);
        }
    }
    else
    {
        printf("当前无科室数据\n");
    }
    printf("------------------------------------------------------\n");

    printf("======================================================\n");
    printf("提示：此面板数据实时更新，可作为系统运行监控参考。\n");
    printf("======================================================\n");
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

    PatientNode* patient_curr = NULL;
    DoctorNode* doctor_curr = NULL;

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
                sprintf(description, "护士：%s，新状态：%s", account_curr->real_name, new_status ? "值班中" : "未值班");
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
    char name[MAX_MED_NAME_LEN];
    char alias[MAX_ALIAS_LEN];
    char generic_name[MAX_GENERIC_NAME_LEN];
    char expiry_date[MAX_DATE_LEN];
    double price;
    int stock;
    int medicare_type;

    printf("\n================ 新增药品 ================\n");
    get_safe_string("请输入商品名: ", name, MAX_MED_NAME_LEN);
    get_safe_string("请输入别名（可留空）: ", alias, MAX_ALIAS_LEN);
    get_safe_string("请输入通用名: ", generic_name, MAX_GENERIC_NAME_LEN);
    price = get_safe_double("请输入药品单价: ");
    stock = get_safe_int("请输入初始库存: ");
    printf("医保类型：0=非医保 1=甲类医保 2=乙类医保\n");
    medicare_type = get_safe_int("请输入医保类型编号: ");
    get_safe_string("请输入效期（YYYY-MM-DD）: ", expiry_date, MAX_DATE_LEN);

    if (medicare_type != MEDICARE_NONE &&
        medicare_type != MEDICARE_CLASS_A &&
        medicare_type != MEDICARE_CLASS_B)
    {
        printf("提示：医保类型非法，新增药品已取消。\n");
        return;
    }

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

void handle_medicine_basic_info_update(void)
{
    char med_id[MAX_ID_LEN];
    char new_name[MAX_MED_NAME_LEN];
    char alias_input[MAX_ALIAS_LEN];
    char new_generic_name[MAX_GENERIC_NAME_LEN];
    char new_expiry_date[MAX_DATE_LEN];
    const char* alias_ptr = NULL;
    double new_price;
    int alias_action;

    printf("\n================ 修改药品基础信息 ================\n");
    get_safe_string("请输入药品编号: ", med_id, MAX_ID_LEN);
    get_safe_string("请输入新商品名（直接回车表示不修改）: ", new_name, MAX_MED_NAME_LEN);
    get_safe_string("请输入新通用名（直接回车表示不修改）: ", new_generic_name, MAX_GENERIC_NAME_LEN);
    new_price = get_safe_double("请输入新单价（<=0 表示不修改）: ");
    get_safe_string("请输入新效期（YYYY-MM-DD，直接回车表示不修改）: ", new_expiry_date, MAX_DATE_LEN);

    printf("别名修改方式：\n");
    printf("  [0] 不修改别名\n");
    printf("  [1] 清空别名\n");
    printf("  [2] 输入新别名\n");
    alias_action = get_safe_int("👉 请输入操作编号: ");
    if (alias_action == 1)
    {
        alias_input[0] = '\0';
        alias_ptr = alias_input;
    }
    else if (alias_action == 2)
    {
        get_safe_string("请输入新别名: ", alias_input, MAX_ALIAS_LEN);
        alias_ptr = alias_input;
    }

    update_medicine_basic_info(
        med_id,
        new_name,
        alias_ptr,
        new_generic_name,
        new_price,
        new_expiry_date
    );
}

void handle_medicine_stock_update(void)
{
    char med_id[MAX_ID_LEN];
    int new_stock;

    printf("\n================ 修改药品库存 ================\n");
    get_safe_string("请输入药品编号: ", med_id, MAX_ID_LEN);
    new_stock = get_safe_int("请输入新库存: ");
    update_medicine_stock(med_id, new_stock);
}

void handle_expiring_medicine_check(void)
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
    show_paid_patients_waiting_for_dispense();
    printf("------------------------------------------------------\n");
    get_safe_string("请输入要发药的患者编号: ", patient_id, MAX_ID_LEN);
    dispense_medicine_for_patient(patient_id);
}

void handle_medicine_remove(void)
{
    char med_id[MAX_ID_LEN];

    printf("\n================ 下架药品 ================\n");
    get_safe_string("请输入要下架的药品编号: ", med_id, MAX_ID_LEN);
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
    strftime(new_log->timestamp, 20, "%Y-%m-%d %H:%M:%S", local_time);
    
    strncpy(new_log->operation, operation, 49);
    new_log->operation[49] = '\0';
    strncpy(new_log->target, target, 49);
    new_log->target[49] = '\0';
    strncpy(new_log->description, description, 199);
    new_log->description[199] = '\0';

    new_log->next = NULL;

    // 将新日志添加到链表末尾
    if (g_log_list != NULL)
    {
        curr = g_log_list;
        while (curr->next != NULL)
        {
            curr = curr->next;
        }
        curr->next = new_log;
    }
}

// 显示日志
void show_logs(void)
{
    LogNode* curr = NULL;
    int log_count = 0;

    printf("\n======================================================\n");
    printf("                  操作日志\n");
    printf("======================================================\n");
    printf("【日志列表】\n");
    printf("------------------------------------------------------\n");

    if (g_log_list != NULL && g_log_list->next != NULL)
    {
        curr = g_log_list->next;
        while (curr != NULL)
        {
            log_count++;
            printf("[%s] %s - %s\n", curr->timestamp, curr->operation, curr->target);
            printf("  %s\n", curr->description);
            printf("------------------------------------------------------\n");
            curr = curr->next;
        }
    }

    if (log_count == 0)
    {
        printf("当前无日志记录\n");
        printf("------------------------------------------------------\n");
    }
    else
    {
        printf("共 %d 条日志记录\n", log_count);
        printf("------------------------------------------------------\n");
    }

    printf("======================================================\n");
}
