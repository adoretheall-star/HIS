// ==========================================
// 文件名: main.c
// 描述: 智慧医疗信息管理系统 - 主入口
// 作者:周宇轩
// ==========================================
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "global.h"
#include "list_ops.h"
#include "utils.h"
#include "appointment.h"
#include "patient_service.h"
#include "doctor_service.h"
#include "admin_service.h"
#include "medicine_service.h"
#include "pharmacy_service.h"
#include "inpatient_service.h"

// 辅助函数：不区分大小写的字符串比较
static int my_strcasecmp(const char* s1, const char* s2)
{
    while (*s1 && *s2)
    {
        char c1 = tolower((unsigned char)*s1);
        char c2 = tolower((unsigned char)*s2);
        if (c1 != c2)
            return c1 - c2;
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

// 当前登录的医生信息
static DoctorNode* g_current_doctor = NULL;

// 这里未来会引入你们自己写的头文件
// #include "global.h"     // 全局常量与结构体定义
#include "data_io.h"    // 负责读写 txt 文件的模块
// #include "auth.h"       // 负责登录与权限的模块
// #include "utils.h"      // 存放 get_safe_int 终端输入拦截器等工具函数
// 辅助函数声明
static int my_strcasecmp(const char* s1, const char* s2);

// 内部业务相关处理函数（需要内部登录后访问）
static void quick_register_menu();
static void patient_archive_menu();
static void handle_internal_patient_register();
static void handle_internal_appointment_register();
static void handle_internal_appointment_query();
static void handle_internal_appointment_cancel();
static void handle_internal_appointment_check_in();

// 患者自助端菜单和处理函数（外部直接访问，支持本人自助服务）
static void patient_self_service_menu();
static void handle_patient_self_first_visit();
static void handle_patient_self_appointment_register();
static void handle_patient_self_registration();
static void handle_patient_self_appointment_cancel();
static void handle_patient_self_basic_record_query();
static void handle_patient_self_visit_overview_query();

static void handle_patient_self_consult_history_query(const char* patient_id, const char* id_card);
static void handle_patient_self_current_visit_overview();
static void handle_patient_self_balance_query();

static void handle_patient_archive_query_by_id();
static void handle_patient_archive_query_by_id_card();
static void handle_patient_archive_query_by_name();
static void handle_patient_archive_update();
static void display_recent_alerts();

// 判断是否为检查科室
static int is_check_department(const char* dept)
{
    if (dept == NULL) return 0;
    const char* check_depts[] = {"放射科", "影像科", "检验科", "超声科", "心电图室", NULL};
    for (int i = 0; check_depts[i] != NULL; i++) {
        if (strcmp(dept, check_depts[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static void internal_login_menu();
static void admin_menu();
static void admin_medicine_menu();
static void admin_check_item_menu();
static void nurse_menu();
static void doctor_menu();
static void pharmacist_menu();
static void check_dept_doctor_menu();
static void handle_view_waiting_patients();
static void handle_doctor_consultation();
static void handle_doctor_view_patient_overview();
static void handle_query_check_records();
static int is_check_department(const char* dept);
static RoleType prompt_admin_staff_role();
static void handle_admin_register_account();
static void handle_admin_update_account();
static void handle_admin_update_doctor_duty();
static void handle_admin_update_nurse_duty();
static void handle_pharmacy_dispense();
static int is_current_emergency_request(const char* appoint_doctor, const char* appoint_dept, const char* symptom);
static void inpatient_menu();
static void handle_inpatient_register();
static void handle_bed_assign();
static void handle_inpatient_record_query();
static void handle_inpatient_query_by_id();
static void handle_inpatient_query_by_dept();
static void handle_discharged_patients();
static void handle_deposit_recharge();
static void handle_daily_settlement();
static void handle_transfer_bed();
static void handle_discharge();

static void admin_menu()
{
    int running = 1;

    while (running)
    {
        system("cls");
        display_recent_alerts();
        printf("\n======================================================\n");
        printf("               🔐 管理员菜单\n");
        printf("======================================================\n");
        printf("  [1] 查看员工账号\n");
        printf("  [2] 新增员工账号\n");
        printf("  [3] 修改员工资料\n");
        printf("  [4] 查看医生值班状态\n");
        printf("  [5] 修改医生值班状态\n");
        printf("  [6] 查看护士值班状态\n");
        printf("  [7] 修改护士值班状态\n");
        printf("  [8] 管理统计面板\n");
        printf("  [9] 资源预警查看\n");
        printf("  [10] 负载监控查看\n");
        printf("  [11] 公共状态统计\n");
        printf("  [12] 传染病异常提醒\n");
        printf("  [13] 操作日志\n");
        printf("  [14] 药品与发药管理\n");
        printf("  [15] 病房管理\n");
        printf("  [16] 投诉管理\n");
        printf("  [17] 评价管理\n");
        printf("  [18] 患者档案管理\n");
        printf("  [19] 检查项目管理\n");
        printf("  [20] 查询检查记录\n");
        printf("  [0] 退出登录\n");
        printf("------------------------------------------------------\n");

        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                show_all_accounts();
                system("pause");
                break;
            case 2:
                handle_admin_register_account();
                system("pause");
                break;
            case 3:
                handle_admin_update_account();
                system("pause");
                break;
            case 4:
                show_all_doctors_with_duty_status();
                system("pause");
                break;
            case 5:
                handle_admin_update_doctor_duty();
                system("pause");
                break;
            case 6:
                show_all_nurses_with_duty_status();
                system("pause");
                break;
            case 7:
                handle_admin_update_nurse_duty();
                system("pause");
                break;
            case 8:
                show_admin_dashboard();
                system("pause");
                break;
            case 9:
                show_resource_warnings();
                system("pause");
                break;
            case 10:
                show_load_monitoring();
                system("pause");
                break;
            case 11:
                show_public_status_statistics();
                system("pause");
                break;
            case 12:
                show_infectious_disease_alerts();
                system("pause");
                break;
            case 13:
                show_logs();
                system("pause");
                break;
            case 14:
                admin_medicine_menu();
                system("pause");
                break;
            case 15:
                inpatient_menu();
                system("pause");
                break;
            case 16:
                admin_complaint_menu();
                system("pause");
                break;
            case 17:
                admin_evaluation_menu();
                system("pause");
                break;
            case 18:
                patient_archive_menu();
                system("pause");
                break;
            case 19:
                admin_check_item_menu();
                system("pause");
                break;
            case 20:
                handle_query_check_records();
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

static RoleType prompt_admin_staff_role()
{
    int role_choice;

    printf("\n请选择员工角色：\n");
    printf("  [1] 管理员\n");
    printf("  [2] 护士\n");
    printf("  [3] 医生\n");
    printf("  [5] 药师\n");
    role_choice = get_safe_int("👉 请输入角色编号: ");

    if (role_choice != ROLE_ADMIN &&
        role_choice != ROLE_NURSE &&
        role_choice != ROLE_DOCTOR &&
        role_choice != ROLE_PHARMACIST)
    {
        return 0;
    }

    return (RoleType)role_choice;
}

static void handle_admin_register_account()
{
    char username[MAX_ID_LEN];
    char password[MAX_ID_LEN];
    char real_name[MAX_NAME_LEN];
    char gender;
    char department[MAX_NAME_LEN];
    RoleType role;

    printf("\n================ 新增员工账号 ================\n");
    printf("提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
    
    // 输入登录账号
    input_username:
    while (1) {
        get_safe_string("请输入登录账号: ", username, MAX_ID_LEN);
        
        // 检查是否退出
        if (strcmp(username, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(username, "0") == 0)
        {
            goto input_username;
        }
        
        // 检查账号是否为空
        if (strlen(username) > 0)
        {
            break;
        }
        printf("⚠️ 登录账号不能为空，请重新输入！\n");
    }
    
    // 输入登录密码
    input_password:
    while (1) {
        get_safe_string("请输入登录密码: ", password, MAX_ID_LEN);
        
        // 检查是否退出
        if (strcmp(password, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(password, "0") == 0)
        {
            goto input_username;
        }
        
        // 检查密码是否为空
        if (strlen(password) > 0)
        {
            break;
        }
        printf("⚠️ 登录密码不能为空，请重新输入！\n");
    }
    
    // 输入真实姓名
    input_real_name:
    while (1) {
        get_safe_string("请输入真实姓名: ", real_name, MAX_NAME_LEN);
        
        // 检查是否退出
        if (strcmp(real_name, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(real_name, "0") == 0)
        {
            goto input_password;
        }
        
        // 检查姓名是否为空
        if (strlen(real_name) > 0)
        {
            break;
        }
        printf("⚠️ 真实姓名不能为空，请重新输入！\n");
    }
    
    // 输入性别
    char gender_str[8];
    input_gender:
    while (1) {
        printf("请输入性别 (男/女): ");
        get_safe_string(gender_str, gender_str, 8);
        
        // 检查是否退出
        if (strcmp(gender_str, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(gender_str, "0") == 0)
        {
            goto input_real_name;
        }
        
        if (strcmp(gender_str, "男") == 0 || strcmp(gender_str, "女") == 0) {
            break;
        }
        printf("⚠️ 无效的性别输入，请输入 男 或 女。\n");
    }

    // 输入角色
    input_role:
    role = prompt_admin_staff_role();
    
    // 检查是否回退
    if (role == 0)
    {
        printf("⚠️ 无效的角色编号，请重新选择！\n");
        goto input_role;
    }

    department[0] = '\0';
    if (role == ROLE_DOCTOR)
    {
        // 输入医生所属科室
        input_department:
        while (1) {
            get_safe_string("请输入医生所属科室: ", department, MAX_NAME_LEN);
            
            // 检查是否退出
            if (strcmp(department, "00") == 0)
            {
                printf("操作取消！\n");
                system("pause");
                return;
            }
            
            // 检查是否回退
            if (strcmp(department, "0") == 0)
            {
                goto input_role;
            }
            
            if (!is_blank_string(department))
            {
                break;
            }
            printf("⚠️ 医生账号必须绑定科室，新增账号已取消。\n");
            goto input_role;
        }
        
        if (g_doctor_list == NULL)
        {
            printf("⚠️ 医生链表尚未初始化，无法新增医生账号。\n");
            system("pause");
            return;
        }
        if (find_doctor_by_id(g_doctor_list, username) != NULL)
        {
            printf("⚠️ 已存在同编号医生实体，无法重复新增。\n");
            system("pause");
            return;
        }
    }

    if (!register_account(username, password, real_name, gender_str, role))
    {
        system("pause");
        return;
    }

    if (role == ROLE_DOCTOR)
    {
        DoctorNode* new_doctor = create_doctor_node(username, real_name, gender_str, department);
        if (new_doctor == NULL)
        {
            delete_account_by_username(g_account_list, username);
            printf("⚠️ 医生实体创建失败，已回滚刚新增的账号。\n");
            system("pause");
            return;
        }

        insert_doctor_tail(g_doctor_list, new_doctor);
        printf("✅ 已同步创建医生实体：%s（%s）- %s\n", real_name, username, department);
    }
    system("pause");
}

static void handle_admin_update_account()
{
    char username[MAX_ID_LEN];
    char new_real_name[MAX_NAME_LEN];
    char new_password[MAX_ID_LEN];
    char department[MAX_NAME_LEN];
    AccountNode* account = NULL;
    DoctorNode* doctor = NULL;
    RoleType old_role;
    int role_choice;
    RoleType target_role;

    printf("\n================ 修改员工资料 ================\n");
    get_safe_string("请输入要修改的账号: ", username, MAX_ID_LEN);

    account = find_account_by_username(g_account_list, username);
    if (account == NULL)
    {
        printf("⚠️ 未找到对应账号。\n");
        return;
    }

    old_role = account->role;
    doctor = find_doctor_by_id(g_doctor_list, username);

    printf("当前真实姓名：%s\n", account->real_name);
    printf("当前角色：%d\n", account->role);
    get_safe_string("请输入新真实姓名（直接回车表示不修改）: ", new_real_name, MAX_NAME_LEN);
    get_safe_string("请输入新密码（直接回车表示不修改）: ", new_password, MAX_ID_LEN);

    printf("\n角色修改选项：\n");
    printf("  [0] 不修改角色\n");
    printf("  [1] 管理员\n");
    printf("  [2] 护士\n");
    printf("  [3] 医生\n");
    printf("  [5] 药师\n");
    role_choice = get_safe_int("👉 请输入角色编号: ");
    if (role_choice != 0 &&
        role_choice != ROLE_ADMIN &&
        role_choice != ROLE_NURSE &&
        role_choice != ROLE_DOCTOR &&
        role_choice != ROLE_PHARMACIST)
    {
        printf("⚠️ 无效的角色编号，修改已取消。\n");
        return;
    }

    target_role = (role_choice == 0) ? account->role : (RoleType)role_choice;
    department[0] = '\0';

    if (target_role == ROLE_DOCTOR)
    {
        if (g_doctor_list == NULL)
        {
            printf("⚠️ 医生链表尚未初始化，无法同步医生资料。\n");
            return;
        }

        if (doctor != NULL)
        {
            printf("当前科室：%s\n", doctor->department);
            get_safe_string("请输入新科室（直接回车表示不修改）: ", department, MAX_NAME_LEN);
        }
        else
        {
            get_safe_string("请输入医生所属科室: ", department, MAX_NAME_LEN);
            if (is_blank_string(department))
            {
                printf("⚠️ 医生账号必须绑定科室，修改已取消。\n");
                return;
            }
        }
    }

    if (!update_account_basic_info(username, new_real_name, new_password, role_choice == 0 ? 0 : (RoleType)role_choice))
    {
        return;
    }

    account = find_account_by_username(g_account_list, username);
    if (account == NULL)
    {
        printf("⚠️ 账号更新后未能重新定位账号节点，请检查数据。\n");
        return;
    }

    if (old_role != ROLE_DOCTOR && account->role == ROLE_DOCTOR)
    {
        DoctorNode* new_doctor = create_doctor_node(username, account->real_name, account->gender, department);
        if (new_doctor == NULL)
        {
            printf("⚠️ 医生实体创建失败，请尽快补录医生信息。\n");
            return;
        }

        insert_doctor_tail(g_doctor_list, new_doctor);
        printf("✅ 已同步创建医生实体：%s（科室：%s）\n", account->real_name, department);
    }
    else if (old_role == ROLE_DOCTOR && account->role != ROLE_DOCTOR)
    {
        if (!delete_doctor_by_id(g_doctor_list, username))
        {
            printf("⚠️ 账号角色已修改，但未能删除旧医生实体，请手动核对。\n");
            return;
        }
        printf("✅ 已同步移除旧医生实体。\n");
    }
    else if (account->role == ROLE_DOCTOR)
    {
        doctor = find_doctor_by_id(g_doctor_list, username);
        if (doctor == NULL)
        {
            if (is_blank_string(department))
            {
                printf("⚠️ 检测到缺失医生实体，但当前未提供科室，无法自动补建。\n");
                return;
            }

            doctor = create_doctor_node(username, account->real_name, account->gender, department);
            if (doctor == NULL)
            {
                printf("⚠️ 医生实体补建失败，请手动处理。\n");
                return;
            }
            insert_doctor_tail(g_doctor_list, doctor);
            printf("✅ 已补建缺失的医生实体。\n");
        }
        else
        {
            safe_copy_string(doctor->name, MAX_NAME_LEN, account->real_name);
            if (!is_blank_string(department))
            {
                safe_copy_string(doctor->department, MAX_NAME_LEN, department);
            }
            printf("✅ 已同步更新医生实体资料。\n");
        }
    }
}

static void handle_admin_update_doctor_duty()
{
    char doctor_id[MAX_ID_LEN];
    int new_status;

    printf("\n================ 修改医生值班状态 ================\n");
    printf("提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
    
    // 输入医生工号
    while (1)
    {
        get_safe_string("请输入医生工号: ", doctor_id, MAX_ID_LEN);
        
        // 检查是否退出
        if (strcmp(doctor_id, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(doctor_id, "0") == 0)
        {
            return;
        }
        
        // 检查医生工号是否为空
        if (strlen(doctor_id) > 0)
        {
            break;
        }
        printf("⚠️ 医生工号不能为空，请重新输入！\n");
    }
    
    // 输入新状态
    while (1)
    {
        new_status = get_safe_int("请输入新状态（1=值班中, 0=未值班）: ");
        
        // 检查状态是否有效
        if (new_status == 0 || new_status == 1)
        {
            break;
        }
        printf("⚠️ 无效的状态值，请输入 0 或 1！\n");
    }
    
    update_doctor_duty_status(doctor_id, new_status);
    system("pause");
}
static void handle_admin_update_nurse_duty()
{
    char username[MAX_ID_LEN];
    int new_status;

    printf("\n================ 修改护士值班状态 ================\n");
    printf("提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
    
    // 输入护士账号
    while (1)
    {
        get_safe_string("请输入护士账号: ", username, MAX_ID_LEN);
        
        // 检查是否退出
        if (strcmp(username, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(username, "0") == 0)
        {
            return;
        }
        
        // 检查护士账号是否为空
        if (strlen(username) > 0)
        {
            break;
        }
        printf("⚠️ 护士账号不能为空，请重新输入！\n");
    }
    
    // 输入新状态
    while (1)
    {
        new_status = get_safe_int("请输入新状态（1=值班中, 0=未值班）: ");
        
        // 检查状态是否有效
        if (new_status == 0 || new_status == 1)
        {
            break;
        }
        printf("⚠️ 无效的状态值，请输入 0 或 1！\n");
    }
    
    update_nurse_duty_status(username, new_status);
    system("pause");
}

static int is_current_emergency_request(const char* appoint_doctor, const char* appoint_dept, const char* symptom)
{
    if (!is_blank_string(symptom))
    {
        const char* recommended_dept = recommend_dept_by_symptom(symptom);
        if (recommended_dept != NULL && strcmp(recommended_dept, "急诊科") == 0)
        {
            return 1;
        }
    }

    if (!is_blank_string(appoint_dept) && strcmp(appoint_dept, "急诊科") == 0)
    {
        return 1;
    }

    if (!is_blank_string(appoint_doctor) && g_doctor_list != NULL)
    {
        DoctorNode* doctor = find_doctor_by_id(g_doctor_list, appoint_doctor);
        if (doctor != NULL && strcmp(doctor->department, "急诊科") == 0)
        {
            return 1;
        }
    }

    return 0;
}

static void admin_medicine_menu()
{
    int running = 1;
    char keyword[MAX_NAME_LEN];
    int threshold;

    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               💊 药品管理\n");
        printf("======================================================\n");
        printf("  [1] 查看全部药品\n");
        printf("  [2] 查询药品\n");
        printf("  [3] 新增药品\n");
        printf("  [4] 修改药品信息\n");
        printf("  [5] 修改药品库存\n");
        printf("  [6] 查看低库存药品\n");
        printf("  [7] 查看近效期药品\n");
        printf("  [8] 下架药品\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");

        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                show_all_medicines();
                system("pause");
                break;
            case 2:
                get_safe_string("请输入查询关键词: ", keyword, MAX_NAME_LEN);
                search_medicine_by_keyword(keyword);
                system("pause");
                break;
            case 3:
                handle_medicine_register();
                system("pause");
                break;
            case 4:
                handle_medicine_basic_info_update();
                system("pause");
                break;
            case 5:
                handle_medicine_stock_update();
                system("pause");
                break;
            case 6:
                threshold = get_safe_int("请输入低库存阈值: ");
                show_low_stock_medicines(threshold);
                system("pause");
                break;
            case 7:
                handle_expiring_medicine_check();
                system("pause");
                break;
            case 8:
                handle_medicine_remove();
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

static void admin_check_item_menu()
{
    int running = 1;
    char keyword[MAX_NAME_LEN];

    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               检查项目管理\n");
        printf("======================================================\n");
        printf("  [1] 查看全部检查项目\n");
        printf("  [2] 查询检查项目\n");
        printf("  [3] 新增检查项目\n");
        printf("  [4] 修改检查项目\n");
        printf("  [5] 下架检查项目\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");

        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                show_all_check_items();
                system("pause");
                break;
            case 2:
                get_safe_string("请输入查询关键词: ", keyword, MAX_NAME_LEN);
                search_check_item_by_keyword(keyword);
                system("pause");
                break;
            case 3:
                handle_check_item_register();
                system("pause");
                break;
            case 4:
                handle_check_item_update();
                system("pause");
                break;
            case 5:
                handle_check_item_remove();
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

static void handle_view_waiting_patients()
{
    printf("\n================ 查看待诊患者 ================\n");
    show_waiting_patients_by_doctor(g_current_doctor->id);
    system("pause");
}

static void handle_doctor_consultation()
{
    PatientNode* patient = NULL;
    char diagnosis_text[MAX_RECORD_LEN];
    char treatment_advice[MAX_RECORD_LEN];
    char med_id[MAX_ID_LEN];
    char item_id[MAX_ID_LEN];
    PrescriptionNode* temp_prescription_list = NULL;
    CheckRecordNode* temp_check_record_list = NULL;
    int decision;
    int count;
    int consult_success = 0;
    int estimated_days = 0;
    int condition_level = 0;
    double deposit = 0.0;
    static int check_record_counter = 1001;

    printf("\n================ 医生接诊 ================\n");

    show_waiting_patients_by_doctor(g_current_doctor->id);

    // 使用与 doctor_service.c 相同的逻辑获取排序后的待诊患者链表
    PatientPtrNode* waiting_list = get_waiting_patients_by_doctor(g_current_doctor->id);
    count = get_patient_ptr_list_count(waiting_list);

    if (count == 0)
    {
        free_patient_ptr_list(waiting_list);
        system("pause");
        return;
    }

    // 自动选择首位患者
    patient = get_nth_patient_from_ptr_list(waiting_list, 1);
    printf("\n📙 当前系统自动叫号排队首位患者...\n");

    printf("\n========================================\n");
    printf("确认接诊患者：\n");
    printf("患者编号：%s\n", patient->id);
    printf("姓名：%s\n", patient->name);
    printf("性别：%s\n", patient->gender);
    printf("年龄：%d\n", patient->age);
    printf("症状：%s\n", patient->symptom[0] != '\0' ? patient->symptom : "暂无");

    if (patient->status == STATUS_RECHECK_PENDING)
    {
        int has_completed_reports = 0;
        CheckRecordNode* cr_curr = NULL;

        printf("\n\033[1;33m========================================\n");
        printf("【复诊患者接诊中】\n");
        printf("========================================\033[0m\n");

        if (strlen(patient->diagnosis_text) > 0)
        {
            printf("\n📋 历史诊断结论：%s\n", patient->diagnosis_text);
        }
        if (strlen(patient->treatment_advice) > 0)
        {
            printf("📝 历史处理意见：%s\n", patient->treatment_advice);
        }

        printf("\n📊 检查报告：\n");
        if (g_check_record_list != NULL)
        {
            cr_curr = g_check_record_list->next;
        }

        while (cr_curr != NULL)
        {
            if (strcmp(cr_curr->patient_id, patient->id) == 0 && cr_curr->is_completed == 1)
            {
                printf("------------------------------------------\n");
                printf("检查项目：%s\n", cr_curr->item_name);
                printf("检查科室：%s\n", cr_curr->dept);
                printf("检查时间：%s\n", cr_curr->check_time[0] != '\0' ? cr_curr->check_time : "未知");
                printf("检查结果：%s\n", cr_curr->result);
                has_completed_reports = 1;
            }
            cr_curr = cr_curr->next;
        }

        if (!has_completed_reports)
        {
            printf("------------------------------------------\n");
            printf("ℹ️ 该复诊患者暂无已完成的检查报告。\n");
        }

        {
            int continue_consult = get_safe_int("是否继续接诊？(1=继续, 0=取消): ");
            if (continue_consult == 0)
            {
                free_patient_ptr_list(waiting_list);
                system("pause");
                return;
            }
        }
    }

    printf("========================================\n");

enter_diagnosis:
    get_safe_string("请输入诊断结论(输入 B 取消本次接诊，可直接回车留空): ", diagnosis_text, MAX_RECORD_LEN);
    if (strcmp(diagnosis_text, "B") == 0 || strcmp(diagnosis_text, "b") == 0)
    {
        free_patient_ptr_list(waiting_list);
        system("pause");
        return;
    }

enter_treatment:
    get_safe_string("请输入处理意见(输入 B 回退修改诊断，叫号未到或可直接回车留空): ", treatment_advice, MAX_RECORD_LEN);
    if (strcmp(treatment_advice, "B") == 0 || strcmp(treatment_advice, "b") == 0)
        goto enter_diagnosis;

    // 保存患者原始状态，用于回退时恢复
    MedStatus original_status = patient->status;

back_to_decision:
    // 清空临时处方链表
    while (temp_prescription_list != NULL)
    {
        PrescriptionNode* temp = temp_prescription_list;
        temp_prescription_list = temp_prescription_list->next;
        free(temp);
    }
    // 清空临时检查记录链表
    while (temp_check_record_list != NULL)
    {
        CheckRecordNode* temp = temp_check_record_list;
        temp_check_record_list = temp_check_record_list->next;
        free(temp);
    }
    estimated_days = 0;
    condition_level = 0;
    deposit = 0.0;
    printf("\n请选择诊疗决策:\n");
    printf("  [1] 结束就诊\n");
    printf("  [2] 开药\n");
    printf("  [3] 开检查\n");
    printf("  [4] 办理住院\n");
    printf("  [5] 叫号未到 (过号顺延3位)\n");
    printf("  [0] 返回修改诊断/意见\n");

    if (patient->status == STATUS_RECHECK_PENDING)
    {
        printf("------------------------------------------\n");
        printf("💡 提示：建议根据检查报告结果进行\"开药\"、\"住院\"或\"结束就诊\"\n");
    }
    else
    {
        printf("------------------------------------------\n");
    }

    decision = get_safe_int("👉 请输入操作编号: ");

    if (decision == 0)
        goto enter_diagnosis;

    if (decision == 2)
    {
        if (g_medicine_list == NULL || g_medicine_list->next == NULL)
        {
            printf("\n⚠️ 当前没有可用药品数据，无法执行“开药”决策！\n");
            free_patient_ptr_list(waiting_list);
            system("pause");
            return;
        }
    }
    else if (decision == 3)
    {
        if (g_check_item_list == NULL || g_check_item_list->next == NULL || g_check_record_list == NULL)
        {
            printf("\n⚠️ 当前没有可用检查项目数据，无法执行“开检查”决策！\n");
            free_patient_ptr_list(waiting_list);
            system("pause");
            return;
        }
    }

    else if (decision == 4)
    {
        if (patient->status == STATUS_NEED_HOSPITALIZE)
        {
            printf("\n⚠️ 该患者已标记为需要住院，请通知护士办理登记！\n");
            free_patient_ptr_list(waiting_list);
            system("pause");
            return;
        }

        if (find_active_inpatient_by_patient_id(patient->id) != NULL)
        {
            printf("\n⚠️ 该患者已拥有有效住院记录，不能重复办理住院！\n");
            free_patient_ptr_list(waiting_list);
            system("pause");
            return;
        }

        patient->status = STATUS_NEED_HOSPITALIZE;

        printf("\n✅ 已为患者开具住院通知，请通知患者前往护士站办理住院登记！\n");

        patient->status = original_status;
        consult_success = doctor_consult_patient(
            g_current_doctor->id,
            patient->id,
            decision,
            diagnosis_text,
            treatment_advice
        );

        if (!consult_success)
        {
            patient->status = original_status;
            free_patient_ptr_list(waiting_list);
            system("pause");
            return;
        }

        patient->status = STATUS_NEED_HOSPITALIZE;
        printf("\n✅ 住院通知已开具，患者可前往护士站办理住院登记！\n");
        goto finish_doctor_consultation;
    }

    if (decision == 5)
    {
        int confirm_no_show = get_safe_int("确定该患者叫号未到？(1=确定, 0=返回决策): ");
        if (confirm_no_show != 1)
            goto back_to_decision;

        patient->call_count++;

        if (patient->call_count >= 3)
        {
            AppointmentNode* curr_appointment = NULL;

            if (g_current_doctor != NULL && g_current_doctor->queue_length > 0)
            {
                g_current_doctor->queue_length--;
            }

            patient->status = STATUS_NO_SHOW;
            patient->queue_time = 0;
            patient->call_count = 0;
            patient->doctor_id[0] = '\0';

            if (g_appointment_list != NULL)
            {
                curr_appointment = g_appointment_list->next;
                while (curr_appointment != NULL)
                {
                    if (strcmp(curr_appointment->patient_id, patient->id) == 0 &&
                        curr_appointment->appointment_status == CHECKED_IN)
                    {
                        curr_appointment->appointment_status = MISSED;
                    }
                    curr_appointment = curr_appointment->next;
                }
            }

            printf("\n⛔ [熔断机制] 该患者已连续 3 次叫号未到，系统已将其从当前排队序列中移除！\n");
            printf("如需看诊，请患者重新前往服务台挂号。\n");
        }
        else
        {
            // 获取排序后的待诊患者链表
            PatientPtrNode* temp_waiting_list = get_waiting_patients_by_doctor(g_current_doctor->id);
            int count = get_patient_ptr_list_count(temp_waiting_list);
            
            if (count > 0)
            {
                // 找到当前患者在排序后链表中的位置
                int current_position = find_patient_position_in_ptr_list(temp_waiting_list, patient);
                
                if (current_position != -1)
                {
                    // 计算顺延 3 位后的目标位置
                    PatientNode* target_patient = get_patient_after_position_in_ptr_list(temp_waiting_list, current_position, 3);
                    
                    if (target_patient != NULL)
                    {
                        patient->queue_time = target_patient->queue_time + 1;
                    }
                    else
                    {
                        patient->queue_time = time(NULL);
                    }
                }
                else
                {
                    patient->queue_time = time(NULL);
                }
            }
            else
            {
                patient->queue_time = time(NULL);
            }
            
            // 释放临时链表
            free_patient_ptr_list(temp_waiting_list);
            printf("\n🔄 [过号顺延] 第 %d 次叫号未到，已顺延至 3 位之后。(满 3 次将作废)\n", patient->call_count);
        }

        free_patient_ptr_list(waiting_list);
        system("pause");
        return;
    }

    if (decision != 2 && decision != 3 && decision != 4)
    {
        if (decision == 1)
        {
            int confirm_end = get_safe_int("确定结束本次就诊？(1=确定, 0=返回决策): ");
            if (confirm_end != 1)
                goto back_to_decision;
        }
        consult_success = doctor_consult_patient(
            g_current_doctor->id,
            patient->id,
            decision,
            diagnosis_text,
            treatment_advice
        );

        if (!consult_success)
        {
            free_patient_ptr_list(waiting_list);
            system("pause");
            return;
        }
    }

    if (decision == 2)
    {
        MedicineNode* med_curr = NULL;

        if (g_medicine_list == NULL)
        {
            printf("\n⚠️ 药品链表尚未初始化，无法开药！\n");
            free_patient_ptr_list(waiting_list);
            system("pause");
            return;
        }

        // 是否继续开药的确认环节
        confirm_prescription:
        printf("\n是否继续开具处方？\n");
        printf("1. 确认开始开药\n");
        printf("0. 返回诊疗决策\n");
        int confirm_prescription_choice = get_safe_int("请选择: ");
        if (confirm_prescription_choice == 0)
        {
            // 回退到诊疗决策，恢复患者原始状态
            goto back_to_decision;
        }
        else if (confirm_prescription_choice != 1)
        {
            printf("无效选择，请重新选择！\n");
            goto confirm_prescription;
        }

        printf("\n================ 可用药品列表 ================\n");
        med_curr = g_medicine_list->next;
        while (med_curr != NULL)
        {
            printf("药品编号：%s | 商品名：%s | 单价：%.2f | 当前库存：%d\n",
                   med_curr->id,
                   med_curr->name,
                   med_curr->price,
                   med_curr->stock);
            med_curr = med_curr->next;
        }

        printf("------------------------------------------\n");
        while (1)
        {
            MedicineNode* med = NULL;
            int quantity = 0;

            // 药品编号输入环节
            input_med_id:
            get_safe_string("请输入药品编号（输入0结束开药，输入00退出，输入000回退确认）: ", med_id, MAX_ID_LEN);
            if (strcmp(med_id, "00") == 0)
            {
                goto back_to_decision;
            }
            if (strcmp(med_id, "000") == 0)
            {
                // 清空临时处方链表
                while (temp_prescription_list != NULL)
                {
                    PrescriptionNode* temp = temp_prescription_list;
                    temp_prescription_list = temp_prescription_list->next;
                    free(temp);
                }
                goto confirm_prescription;
            }
            if (strcmp(med_id, "0") == 0)
            {
                // 检查临时处方链表是否为空
                if (temp_prescription_list == NULL)
                {
                    printf("⚠️ “开药”决策至少需要开出一种药品，当前不能空单结束！\n");
                    continue;
                }
                break;
            }

            med = find_medicine_by_id(g_medicine_list, med_id);
            if (med == NULL)
            {
                printf("⚠️ 未找到对应药品，请重新输入！\n");
                continue;
            }

            // 数量输入环节
            quantity = get_safe_int("请输入开药数量（输入0回退上一步）: ");
            if (quantity == 0)
            {
                goto input_med_id;
            }
            if (quantity <= 0)
            {
                printf("⚠️ 开药数量必须大于 0！\n");
                continue;
            }

            if (quantity > med->stock)
            {
                printf("⚠️ 库存不足！当前库存为 %d，无法开具 %d。\n", med->stock, quantity);
                continue;
            }

            // 创建新的处方节点
            PrescriptionNode* new_prescription = (PrescriptionNode*)malloc(sizeof(PrescriptionNode));
            if (new_prescription == NULL)
            {
                printf("⚠️ 内存分配失败，无法添加药品！\n");
                continue;
            }
            strncpy(new_prescription->med_id, med_id, MAX_ID_LEN - 1);
            new_prescription->med_id[MAX_ID_LEN - 1] = '\0';
            new_prescription->quantity = quantity;
            new_prescription->next = NULL;

            // 将新节点添加到临时处方链表
            if (temp_prescription_list == NULL)
            {
                temp_prescription_list = new_prescription;
            }
            else
            {
                PrescriptionNode* temp = temp_prescription_list;
                while (temp->next != NULL)
                {
                    temp = temp->next;
                }
                temp->next = new_prescription;
            }
            printf("✅ 开药成功：%s x %d\n", med->name, quantity);
        }

        // 检查临时处方链表是否为空
        if (temp_prescription_list == NULL)
        {
            printf("\n⚠️ 本次接诊未追加任何处方药品，请确认是否符合医嘱预期。\n");
        }
    }
    else if (decision == 3)
    {
        CheckItemNode* item_curr = NULL;

        if (g_check_item_list == NULL || g_check_record_list == NULL)
        {
            printf("\n⚠️ 检查相关链表尚未初始化，无法开检查单！\n");
            free_patient_ptr_list(waiting_list);
            system("pause");
            return;
        }

        // 是否继续开检查的确认环节
        confirm_check:
        printf("\n是否继续开具检查单？\n");
        printf("1. 确认开始开检查\n");
        printf("0. 返回诊疗决策\n");
        int confirm_check_choice = get_safe_int("请选择: ");
        if (confirm_check_choice == 0)
        {
            // 回退到诊疗决策，恢复患者原始状态
            goto back_to_decision;
        }
        else if (confirm_check_choice != 1)
        {
            printf("无效选择，请重新选择！\n");
            goto confirm_check;
        }

        printf("\n================ 可用检查项目列表 ================\n");
        item_curr = g_check_item_list->next;
        while (item_curr != NULL)
        {
            printf("项目编号：%s | 项目名称：%s | 所属科室：%s | 价格：%.2f\n",
                   item_curr->item_id,
                   item_curr->item_name,
                   item_curr->dept,
                   item_curr->price);
            item_curr = item_curr->next;
        }

        printf("------------------------------------------\n");
        while (1)
        {
            CheckItemNode* item = NULL;
            CheckRecordNode* new_node = NULL;
            char record_id[MAX_ID_LEN];

            // 检查项目编号输入环节
            get_safe_string("请输入检查项目编号（输入0结束开检查，输入00退出，输入000回退确认）: ", item_id, MAX_ID_LEN);
            if (strcmp(item_id, "00") == 0)
            {
                goto back_to_decision;
            }
            if (strcmp(item_id, "000") == 0)
            {
                // 清空临时检查记录链表
                while (temp_check_record_list != NULL)
                {
                    CheckRecordNode* temp = temp_check_record_list;
                    temp_check_record_list = temp_check_record_list->next;
                    free(temp);
                }
                goto confirm_check;
            }
            if (strcmp(item_id, "0") == 0)
            {
                // 检查临时检查记录链表是否为空
                if (temp_check_record_list == NULL)
                {
                    printf("⚠️ “开检查”决策至少需要开出一个检查项目，当前不能空单结束！\n");
                    continue;
                }
                break;
            }

            item = find_check_item_by_id(g_check_item_list, item_id);
            if (item == NULL)
            {
                printf("⚠️ 未找到对应检查项目，请重新输入！\n");
                continue;
            }

            // 创建新的检查记录节点
            snprintf(record_id, sizeof(record_id), "CRK-%04d", check_record_counter++);
            new_node = (CheckRecordNode*)malloc(sizeof(CheckRecordNode));
            if (new_node == NULL)
            {
                printf("⚠️ 内存分配失败，无法添加检查项目！\n");
                continue;
            }
            strncpy(new_node->record_id, record_id, MAX_ID_LEN - 1);
            new_node->record_id[MAX_ID_LEN - 1] = '\0';
            strncpy(new_node->patient_id, patient->id, MAX_ID_LEN - 1);
            new_node->patient_id[MAX_ID_LEN - 1] = '\0';
            strncpy(new_node->item_id, item->item_id, MAX_ID_LEN - 1);
            new_node->item_id[MAX_ID_LEN - 1] = '\0';
            strncpy(new_node->item_name, item->item_name, MAX_NAME_LEN - 1);
            new_node->item_name[MAX_NAME_LEN - 1] = '\0';
            strncpy(new_node->dept, item->dept, MAX_NAME_LEN - 1);
            new_node->dept[MAX_NAME_LEN - 1] = '\0';
            new_node->check_time[0] = '\0';
            new_node->result[0] = '\0';
            new_node->is_completed = 0;
            new_node->is_paid = 0;
            new_node->prev = NULL;
            new_node->next = NULL;

            // 将新节点添加到临时检查记录链表
            if (temp_check_record_list == NULL)
            {
                temp_check_record_list = new_node;
            }
            else
            {
                CheckRecordNode* temp = temp_check_record_list;
                while (temp->next != NULL)
                {
                    temp = temp->next;
                }
                temp->next = new_node;
            }
            printf("✅ 已缓存检查项目：%s\n", item->item_name);
        }

        // 检查临时检查记录链表是否为空
        if (temp_check_record_list == NULL)
        {
            printf("\n⚠️ 本次接诊未追加任何检查项目，请确认是否符合医嘱预期。\n");
        }
        consult_success = doctor_consult_patient(
            g_current_doctor->id,
            patient->id,
            decision,
            diagnosis_text,
            treatment_advice
        );

        if (!consult_success)
        {
            // 释放临时检查记录链表
            while (temp_check_record_list != NULL)
            {
                CheckRecordNode* temp = temp_check_record_list;
                temp_check_record_list = temp_check_record_list->next;
                free(temp);
            }
            free_patient_ptr_list(waiting_list);
            system("pause");
            return;
        }

        // 将临时检查记录链表中的节点添加到主检查记录链表
        CheckRecordNode* temp = temp_check_record_list;
        while (temp != NULL)
        {
            CheckRecordNode* next = temp->next;
            // 断开临时链表节点的关系
            temp->next = NULL;
            temp->prev = NULL;
            // 将节点添加到主链表
            if (g_check_record_list->next == NULL)
            {
                g_check_record_list->next = temp;
                temp->prev = g_check_record_list;
            }
            else
            {
                CheckRecordNode* last = g_check_record_list->next;
                while (last->next != NULL)
                {
                    last = last->next;
                }
                last->next = temp;
                temp->prev = last;
            }
            temp = next;
        }
        temp_check_record_list = NULL;
    }

    if (decision == 2)
    {
        consult_success = doctor_consult_patient(
            g_current_doctor->id,
            patient->id,
            decision,
            diagnosis_text,
            treatment_advice
        );

        if (!consult_success)
        {
            // 释放临时处方链表
            while (temp_prescription_list != NULL)
            {
                PrescriptionNode* temp = temp_prescription_list;
                temp_prescription_list = temp_prescription_list->next;
                free(temp);
            }
            free_patient_ptr_list(waiting_list);
            system("pause");
            return;
        }

        // 遍历临时处方链表，将药品添加到患者的处方中
        PrescriptionNode* temp = temp_prescription_list;
        while (temp != NULL)
        {
            add_prescription_to_patient(patient, temp->med_id, temp->quantity);
            temp = temp->next;
        }
    }

finish_doctor_consultation:
    // 释放临时链表
    free_patient_ptr_list(waiting_list);
    system("pause");
}

static void handle_doctor_view_patient_overview()
{
    int count;
    int choice;
    
    printf("\n================ 查看患者接诊前信息 ================\n");
    
    // 显示待诊患者列表
    show_waiting_patients_by_doctor(g_current_doctor->id);
    
    // 使用与 doctor_service.c 相同的逻辑获取排序后的待诊患者链表
    PatientPtrNode* waiting_list = get_waiting_patients_by_doctor(g_current_doctor->id);
    count = get_patient_ptr_list_count(waiting_list);
    
    if (count == 0)
    {
        free_patient_ptr_list(waiting_list);
        system("pause");
        return;
    }
    
    // 选择患者
    printf("\n请选择要查看的患者序号 (1-%d): ", count);
    choice = get_safe_int("👉 ");
    
    if (choice < 1 || choice > count)
    {
        free_patient_ptr_list(waiting_list);
        printf("\n⚠️ 无效的选择！\n");
        system("pause");
        return;
    }
    
    // 从排序后的链表中获取选择的患者
    PatientNode* selected_patient = get_nth_patient_from_ptr_list(waiting_list, choice);
    
    // 查看患者概览
    doctor_view_patient_overview(g_current_doctor->id, selected_patient->id);
    
    // 释放临时链表
    free_patient_ptr_list(waiting_list);
    system("pause");
}

static void handle_doctor_view_processed_patients()
{
    printf("\n================ 查看已处理患者 ================\n");
    doctor_view_processed_patients(g_current_doctor->id);
    system("pause");
}

static void handle_doctor_view_processed_patient_detail()
{
    int count = 0;
    int choice;
    
    printf("\n================ 查看已处理患者详情 ================\n");
    
    // 显示已处理患者列表（带序号）
    doctor_view_processed_patients(g_current_doctor->id);
    
    // 使用 get_processed_patients_by_doctor 获取已处理患者链表
    PatientPtrNode* processed_list = get_processed_patients_by_doctor(g_current_doctor->id);
    count = get_patient_ptr_list_count(processed_list);
    
    if (count == 0)
    {
        free_patient_ptr_list(processed_list);
        printf("暂无已处理患者记录\n");
        system("pause");
        return;
    }
    
    // 选择患者
    printf("\n请选择要查看详情的患者序号 (1-%d): ", count);
    choice = get_safe_int("👉 ");
    
    if (choice < 1 || choice > count)
    {
        free_patient_ptr_list(processed_list);
        printf("\n⚠️ 无效的选择！\n");
        system("pause");
        return;
    }
    
    // 从链表中获取选择的患者
    PatientNode* selected_patient = get_nth_patient_from_ptr_list(processed_list, choice);
    
    // 查看患者详情
    doctor_view_processed_patient_detail(g_current_doctor->id, selected_patient->id);
    
    // 释放临时链表
    free_patient_ptr_list(processed_list);
    system("pause");
}

static void handle_doctor_view_consult_history()
{
    int count = 0;
    int choice;
    
    printf("\n================ 查看历史接诊记录 ================\n");
    
    // 显示已处理患者列表（带序号）
    doctor_view_processed_patients(g_current_doctor->id);
    
    // 使用 get_processed_patients_by_doctor 获取已处理患者链表
    PatientPtrNode* processed_list = get_processed_patients_by_doctor(g_current_doctor->id);
    count = get_patient_ptr_list_count(processed_list);
    
    if (count == 0)
    {
        free_patient_ptr_list(processed_list);
        printf("暂无已处理患者记录\n");
        system("pause");
        return;
    }
    
    // 选择患者查看历史记录
    printf("\n请选择要查看历史记录的患者序号 (1-%d): ", count);
    choice = get_safe_int("👉 ");
    
    if (choice < 1 || choice > count)
    {
        free_patient_ptr_list(processed_list);
        printf("\n⚠️ 无效的选择！\n");
        system("pause");
        return;
    }
    
    // 从链表中获取选择的患者
    PatientNode* selected_patient = get_nth_patient_from_ptr_list(processed_list, choice);
    
    // 查看该患者的历史接诊记录
    doctor_view_consult_history(g_current_doctor->id, selected_patient->id);
    
    // 释放临时链表
    free_patient_ptr_list(processed_list);
    system("pause");
}

static void handle_query_check_records()
{
    char query_str[MAX_ID_LEN];

    printf("\n================ 查询检查记录 ================\n");
    printf("支持：患者编号（P-1001）| 检查记录编号（CR-001）\n");

    while (1)
    {
        get_safe_string("请输入编号（输入 B 返回上一级）: ", query_str, MAX_ID_LEN);
        if (strcmp(query_str, "B") == 0 || strcmp(query_str, "b") == 0) return;
        if (is_blank_string(query_str))
        {
            printf("编号不能为空，请重新输入\n");
            continue;
        }
        if (strncmp(query_str, "P-", 2) == 0)
        {
            if (!validate_patient_id(query_str))
            {
                printf("⚠️ 患者编号格式不合法（应为 P-4位数字，如 P-1001），请重新输入！\n");
                continue;
            }
            show_check_records_by_patient_id(query_str);
            break;
        }
        else if (strncmp(query_str, "CRK-", 4) == 0)
        {
            int len = strlen(query_str);
            int valid = (len == 8);
            for (int i = 4; valid && i < 8; i++)
                if (!isdigit((unsigned char)query_str[i])) valid = 0;
            if (!valid)
            {
                printf("⚠️ 检查记录编号格式不合法（应为 CRK-4位数字，如 CRK-0001），请重新输入！\n");
                continue;
            }
            show_check_record_by_id(query_str);
            break;
        }
        else if (strncmp(query_str, "CR-", 3) == 0)
        {
            int len = strlen(query_str);
            int valid = (len == 6);
            for (int i = 3; valid && i < 6; i++)
                if (!isdigit((unsigned char)query_str[i])) valid = 0;
            if (!valid)
            {
                printf("⚠️ 检查记录编号格式不合法（应为 CR-3位数字，如 CR-001），请重新输入！\n");
                continue;
            }
            show_check_record_by_id(query_str);
            break;
        }
        else
        {
            printf("⚠️ 编号格式不合法（患者 P-xxxx / 记录 CR-xxx / CRK-xxxx），请重新输入！\n");
            continue;
        }
        break;
    }

    system("pause");
}

static void doctor_menu()
{
    int running = 1;

    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               👨‍⚕️ 医生接诊菜单\n");
        printf("======================================================\n");
        printf(" 当前医生：%s（编号：%s）\n", g_current_doctor->name, g_current_doctor->id);
        printf(" 所属科室：%s\n", g_current_doctor->department);
        printf("------------------------------------------------------\n");
        printf("  [1] 查看待诊患者\n");
        printf("  [2] 查看患者接诊前信息\n");
        printf("  [3] 接诊并做诊疗决策\n");
        printf("  [4] 查看已处理患者详情\n");
        printf("  [5] 查询检查记录\n");
        printf("  [0] 退出登录\n");
        printf("------------------------------------------------------\n");

        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                handle_view_waiting_patients();
                break;
            case 2:
                handle_doctor_view_patient_overview();
                break;
            case 3:
                handle_doctor_consultation();
                break;
            case 4:
                handle_doctor_view_processed_patient_detail();
                break;
            case 5:
                handle_query_check_records();
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

// 检查科室医生菜单
static void check_dept_doctor_menu()
{
    int running = 1;
    char result[MAX_RECORD_LEN];
    int choice;
    int count;

    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               🔬 检查科室医生菜单\n");
        printf("======================================================\n");
        printf(" 当前医生：%s（编号：%s）\n", g_current_doctor->name, g_current_doctor->id);
        printf(" 所属科室：%s\n", g_current_doctor->department);
        printf("------------------------------------------------------\n");
        printf("  [1] 查看待检查列表\n");
        printf("  [2] 录入检查结果\n");
        printf("  [3] 查看检查记录详情\n");
        printf("  [4] 按患者查询检查记录\n");
        printf("  [0] 退出登录\n");
        printf("------------------------------------------------------\n");

        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                printf("\n================ 查看待检查列表 ================\n");
                show_waiting_checks_by_dept(g_current_doctor->id);
                system("pause");
                break;
            case 2:
                printf("\n================ 录入检查结果 ================\n");
                
                // 统计待检查记录数量并显示
                count = 0;
                CheckRecordNode* curr = g_check_record_list->next;
                printf("待检查列表：\n");
                while (curr != NULL)
                {
                    if (curr->is_completed == 0 && strcmp(curr->dept, g_current_doctor->department) == 0)
                    {
                        count++;
                        const char* patient_name = get_patient_name_by_id(curr->patient_id);
                        printf("[%d] 检查记录: %s | 患者编号: %s | 患者姓名: %s | 检查项目: %s\n",
                            count,
                            curr->record_id,
                            curr->patient_id,
                            patient_name != NULL ? patient_name : "未知",
                            curr->item_name);
                    }
                    curr = curr->next;
                }
                
                if (count == 0)
                {
                    printf("ℹ️ 当前没有待检查的记录。\n");
                    system("pause");
                    break;
                }
                
                // 选择检查记录
                choice = get_safe_int("\n请选择要录入结果的检查记录序号: ");
                if (choice < 1 || choice > count)
                {
                    printf("⚠️ 无效的序号！\n");
                    system("pause");
                    break;
                }
                
                // 找到选择的检查记录
                int index = 0;
                CheckRecordNode* selected_check = NULL;
                curr = g_check_record_list->next;
                while (curr != NULL)
                {
                    if (curr->is_completed == 0 && strcmp(curr->dept, g_current_doctor->department) == 0)
                    {
                        index++;
                        if (index == choice)
                        {
                            selected_check = curr;
                            break;
                        }
                    }
                    curr = curr->next;
                }
                
                // 显示选中的检查记录详情
                show_check_record_detail(selected_check);
                
                // 录入检查结果
                get_safe_string("请输入检查结果: ", result, MAX_RECORD_LEN);
                doctor_update_check_result(g_current_doctor->id, selected_check->record_id, result);
                system("pause");
                break;
            case 3:
                printf("\n================ 查看检查记录详情 ================\n");
                
                // 显示当前科室的所有检查记录
                count = 0;
                curr = g_check_record_list->next;
                printf("检查记录列表：\n");
                while (curr != NULL)
                {
                    if (strcmp(curr->dept, g_current_doctor->department) == 0)
                    {
                        count++;
                        const char* patient_name = get_patient_name_by_id(curr->patient_id);
                        printf("[%d] 检查记录: %s | 患者编号: %s | 患者姓名: %s | 检查项目: %s | 状态: %s\n",
                            count,
                            curr->record_id,
                            curr->patient_id,
                            patient_name != NULL ? patient_name : "未知",
                            curr->item_name,
                            curr->is_completed == 1 ? "已完成" : "待检查");
                    }
                    curr = curr->next;
                }
                
                if (count == 0)
                {
                    printf("ℹ️ 当前科室没有检查记录。\n");
                    system("pause");
                    break;
                }
                
                // 选择检查记录
                choice = get_safe_int("\n请选择要查看详情的检查记录序号: ");
                if (choice < 1 || choice > count)
                {
                    printf("⚠️ 无效的序号！\n");
                    system("pause");
                    break;
                }
                
                // 找到选择的检查记录
                index = 0;
                selected_check = NULL;
                curr = g_check_record_list->next;
                while (curr != NULL)
                {
                    if (strcmp(curr->dept, g_current_doctor->department) == 0)
                    {
                        index++;
                        if (index == choice)
                        {
                            selected_check = curr;
                            break;
                        }
                    }
                    curr = curr->next;
                }
                
                // 显示选中的检查记录详情
                show_check_record_detail(selected_check);
                system("pause");
                break;
            case 4:
                handle_query_check_records();
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

static void nurse_menu()
{
    int running = 1;

    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               👩‍⚕️ 护士/前台菜单\n");
        printf("======================================================\n");
        
        // 显示最近的安全预警
        display_recent_alerts();
        
        printf("  [1] 进入快捷挂号业务菜单\n");
        printf("  [2] 患者档案管理\n");
        printf("  [0] 退出登录\n");
        printf("------------------------------------------------------\n");

        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                quick_register_menu();
                break;
            case 2:
                patient_archive_menu();
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

// 显示最近的安全预警
static void display_recent_alerts()
{
    int has_content = 0;

    printf("\n🚨 【安全预警中心】最近预警：\n");
    printf("------------------------------------------------------\n");

    // === 实时扫描：住院欠费预警 ===
    if (g_inpatient_list && g_inpatient_list->next)
    {
        InpatientRecord* ir = g_inpatient_list->next;
        while (ir)
        {
            if (ir->is_active && ir->deposit_balance < 0)
            {
                PatientNode* p = find_patient_by_id(g_patient_list, ir->patient_id);
                printf("🚨 住院欠费：%s（%s）押金 %.2f 元，已欠费！\n",
                    p ? p->name : "未知", ir->patient_id, ir->deposit_balance);
                has_content = 1;
            }
            ir = ir->next;
        }
    }

    // === 实时扫描：押金不足 ===
    if (g_inpatient_list && g_inpatient_list->next)
    {
        InpatientRecord* ir = g_inpatient_list->next;
        while (ir)
        {
            if (ir->is_active && ir->deposit_balance >= 0 && strlen(ir->bed_id) > 0)
            {
                double rate = 120.0;
                switch (ir->ward_type) {
                    case WARD_TYPE_ICU: rate = 500.0; break;
                    case WARD_TYPE_ISOLATION: rate = 300.0; break;
                    case WARD_TYPE_SINGLE: rate = 400.0; break;
                    default: rate = 120.0; break;
                }
                if (ir->deposit_balance < rate * 3)
                {
                    PatientNode* p = find_patient_by_id(g_patient_list, ir->patient_id);
                    printf("⚠️ 押金不足：%s（%s）余额 %.2f，不足3天费用\n",
                        p ? p->name : "未知", ir->patient_id, ir->deposit_balance);
                    has_content = 1;
                }
            }
            ir = ir->next;
        }
    }

    // === 实时扫描：低库存 ===
    if (g_medicine_list && g_medicine_list->next)
    {
        MedicineNode* m = g_medicine_list->next;
        while (m)
        {
            if (m->stock < 5)
            {
                printf("⚠️ 低库存：药品 [%s] %s 库存 %d，低于阈值5\n",
                    m->id, m->name, m->stock);
                has_content = 1;
            }
            m = m->next;
        }
    }

    // === 实时扫描：临期药品 ===
    if (g_medicine_list && g_medicine_list->next)
    {
        MedicineNode* m = g_medicine_list->next;
        while (m)
        {
            if (strlen(m->expiry_date) > 0)
            {
                struct tm tm_exp = {0};
                if (sscanf(m->expiry_date, "%d-%d-%d", &tm_exp.tm_year, &tm_exp.tm_mon, &tm_exp.tm_mday) == 3)
                {
                    tm_exp.tm_year -= 1900;
                    tm_exp.tm_mon -= 1;
                    time_t exp_time = mktime(&tm_exp);
                    double days_left = difftime(exp_time, time(NULL)) / 86400.0;
                    if (days_left <= 30)
                    {
                        printf("⚠️ 临期药品：%s [%s] %d天后过期\n",
                            m->name, m->id, (int)days_left);
                        has_content = 1;
                    }
                }
            }
            m = m->next;
        }
    }

    // === 事件预警（来自预警队列） ===
    if (g_alert_list && g_alert_list->next)
    {
        AlertNode* curr = g_alert_list->next;
        while (curr->next) curr = curr->next;
        int count = 0;
        while (curr != g_alert_list && count < 5)
        {
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&curr->time));
            printf("[%s] %s\n", time_str, curr->message);
            curr = curr->prev;
            count++;
            has_content = 1;
        }
    }

    if (!has_content)
        printf("✅ 当前无预警，系统运行正常\n");

    printf("------------------------------------------------------\n");
}

static void handle_pharmacy_dispense()
{
    char patient_id[MAX_ID_LEN];

    printf("\n================ 执行发药 ================\n");
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
            system("pause");
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
        }
    }
    
    dispense_medicine_for_patient(patient_id);
}

static void pharmacist_menu()
{
    int running = 1;

    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               💊 药房菜单\n");
        printf("======================================================\n");
        printf("  [1] 查看待发药患者\n");
        printf("  [2] 执行发药\n");
        printf("  [0] 退出登录\n");
        printf("------------------------------------------------------\n");

        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                show_paid_patients_waiting_for_dispense();
                system("pause");
                break;
            case 2:
                handle_pharmacy_dispense();
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

static void internal_login_menu()
{
    char username[MAX_ID_LEN];
    char password[MAX_ID_LEN];
    AccountNode* account = NULL;

    system("cls");
    printf("\n======================================================\n");
    printf("               🔑 内部人员登录\n");
    printf("======================================================\n");

    // 1. 先输入账号并验证存在性
    get_safe_string("请输入账号: ", username, MAX_ID_LEN);
    account = find_account_by_username(g_account_list, username);
    
    if (account == NULL)
    {
        printf("\n⚠️ 登录失败，账号不存在！\n");
        system("pause");
        return;
    }

    // 2. 检查账号是否被锁定
    if (account->error_count >= 3)
    {
        time_t current_time = time(NULL);
        int lock_duration = current_time - account->lock_time;
        
        if (lock_duration < 60)
        {
            printf("\n⛔ 账号已被锁定！请 %d 秒后再试。\n", 60 - lock_duration);
            system("pause");
            return;
        }
        else
        {
            // 锁定时间已过，解除锁定
            account->error_count = 0;
            account->lock_time = 0;
            printf("\n🔓 账号锁定已解除，欢迎再次尝试登录！\n");
        }
    }

    // 3. 账号状态正常，输入密码
    get_safe_string("请输入密码: ", password, MAX_ID_LEN);

    // 4. 比对密码
    if (strcmp(account->password, password) != 0)
    {
        account->error_count++;
        
        if (account->error_count >= 3)
        {
            account->lock_time = time(NULL);
            printf("\n⛔ 密码错误！账号已被锁定 60 秒。\n");
        }
        else
        {
            printf("\n⚠️ 密码错误！您还有 %d 次机会。\n", 3 - account->error_count);
        }
        
        system("pause");
        return;
    }

    // 5. 密码正确，重置错误计数和锁定时间
    account->error_count = 0;
    account->lock_time = 0;

    printf("\n✅ 登录成功，欢迎你：%s\n", account->real_name);
    system("pause");

    switch (account->role)
    {
        case ROLE_ADMIN:
            admin_menu();
            break;
        case ROLE_NURSE:
            nurse_menu();
            break;
        case ROLE_DOCTOR:
            // 查找并保存当前医生信息
            g_current_doctor = find_doctor_by_id(g_doctor_list, account->username);
            if (g_current_doctor == NULL)
            {
                printf("\n⚠️ 未找到对应医生信息！\n");
                system("pause");
                return;
            }
            // 根据科室类型进入不同菜单
            if (is_check_department(g_current_doctor->department))
            {
                check_dept_doctor_menu();
            }
            else
            {
                doctor_menu();
            }
            // 退出医生菜单后，清除当前医生信息
            g_current_doctor = NULL;
            break;
        case ROLE_PHARMACIST:
            pharmacist_menu();
            break;
        default:
            printf("\n⚠️ 当前角色暂未开放内部菜单！\n");
            system("pause");
            break;
    }
}

static void handle_internal_patient_register()
{
    char name[MAX_NAME_LEN];
    char id_card[MAX_ID_LEN];
    char symptom[MAX_SYMPTOM_LEN];
    char target_dept[MAX_NAME_LEN];
    int age;
    char gender;
    printf("\n================ 患者建档 ================\n");
    printf("提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
    
    // 输入姓名并验证
    input_name:
    while (1)
    {
        get_safe_string("请输入患者姓名: ", name, MAX_NAME_LEN);
        
        // 检查是否退出
        if (strcmp(name, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(name, "0") == 0)
        {
            // 重新开始
            goto input_name;
        }
        
        if (strlen(name) > 0)
            break;
        printf("⚠️ 患者姓名不能为空，请重新输入！\n");
    }
    
    // 输入年龄并验证
    input_age:
    while (1)
    {
        char age_str[20];
        get_safe_string("请输入患者年龄: ", age_str, 20);
        
        // 检查是否退出
        if (strcmp(age_str, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(age_str, "0") == 0)
        {
            goto input_name;
        }
        
        if (sscanf(age_str, "%d", &age) == 1 && age > 0)
            break;
        printf("⚠️ 患者年龄必须大于0，请重新输入！\n");
    }
    
    // 输入性别并验证
    char gender_str[8];
    input_gender:
    while (1)
    {
        printf("请输入患者性别 (男/女): ");
        get_safe_string(gender_str, gender_str, 8);
        
        // 检查是否退出
        if (strcmp(gender_str, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(gender_str, "0") == 0)
        {
            goto input_age;
        }
        
        if (strcmp(gender_str, "男") == 0 || strcmp(gender_str, "女") == 0) {
            break;
        }
        printf("⚠️ 性别输入无效，请输入 男 或 女！\n");
    }
    
    // 输入身份证号并验证
    input_id_card:
    while (1)
    {
        get_safe_string("请输入身份证号: ", id_card, MAX_ID_LEN);
        
        // 检查是否退出
        if (strcmp(id_card, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(id_card, "0") == 0)
        {
            goto input_gender;
        }
        
        if (strlen(id_card) == 0)
        {
            printf("⚠️ 身份证号不能为空，请重新输入！\n");
            continue;
        }
        if (!validate_id_card(id_card))
        {
            printf("⚠️ 身份证号格式不合法，请重新输入！\n");
            continue;
        }
        if (find_patient_by_id_card(id_card) != NULL)
        {
            printf("⚠️ 该身份证号已存在，不能重复建档！\n");
            continue;
        }
        break;
    }
    
    // 输入症状描述（可选）
    input_symptom:
    get_safe_string("请输入症状描述(可选): ", symptom, MAX_SYMPTOM_LEN);
    
    // 检查是否退出
    if (strcmp(symptom, "00") == 0)
    {
        printf("操作取消！\n");
        system("pause");
        return;
    }
    
    // 检查是否回退
    if (strcmp(symptom, "0") == 0)
    {
        goto input_id_card;
    }
    
    // 输入目标科室（可选）
    input_dept:
    get_safe_string("请输入目标科室(可选): ", target_dept, MAX_NAME_LEN);
    
    // 检查是否退出
    if (strcmp(target_dept, "00") == 0)
    {
        printf("操作取消！\n");
        system("pause");
        return;
    }
    
    // 检查是否回退
    if (strcmp(target_dept, "0") == 0)
    {
        goto input_symptom;
    }

    register_patient(name, age, gender_str, id_card, symptom, target_dept);
    system("pause");
}
static void handle_internal_appointment_register()
{
    char patient_id[MAX_ID_LEN];
    char appointment_date[MAX_NAME_LEN];
    char appointment_slot[MAX_NAME_LEN];
    char appoint_doctor[MAX_NAME_LEN];
    char appoint_dept[MAX_NAME_LEN];
    printf("\n================ 预约登记 ================\n");
    printf("提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
    
    // 输入患者编号并验证
    input_patient_id:
    while (1)
    {
        get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
        
        // 检查是否退出
        if (strcmp(patient_id, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(patient_id, "0") == 0)
        {
            goto input_patient_id;
        }
        
        // 检查患者编号格式是否合法
        if (validate_patient_id(patient_id))
        {
            break;
        }
        else
        {
            printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
        }
    }
    
    // 输入预约日期并验证
    input_date:
    get_safe_string("请输入预约日期: ", appointment_date, MAX_NAME_LEN);
    
    // 检查是否退出
    if (strcmp(appointment_date, "00") == 0)
    {
        printf("操作取消！\n");
        system("pause");
        return;
    }
    
    // 检查是否回退
    if (strcmp(appointment_date, "0") == 0)
    {
        goto input_patient_id;
    }
    
    // 检查日期是否为空
    if (strlen(appointment_date) == 0)
    {
        printf("⚠️ 预约日期不能为空，请重新输入！\n");
        goto input_date;
    }
    
    // 夜间模式下自动设置为晚上
    input_slot:
    if (is_night_shift()) {
        strcpy(appointment_slot, "晚上");
        printf("当前为夜间模式，自动设置预约时段为: %s\n", appointment_slot);
    } else {
        get_safe_string("请输入预约时段: ", appointment_slot, MAX_NAME_LEN);
        
        // 检查是否退出
        if (strcmp(appointment_slot, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(appointment_slot, "0") == 0)
        {
            goto input_date;
        }
        
        // 检查时段是否为空
        if (strlen(appointment_slot) == 0)
        {
            printf("⚠️ 预约时段不能为空，请重新输入！\n");
            goto input_slot;
        }
    }
    
    // 输入预约科室（可选）
    input_dept:
    get_safe_string("请输入预约科室(可留空): ", appoint_dept, MAX_NAME_LEN);
    
    // 检查是否退出
    if (strcmp(appoint_dept, "00") == 0)
    {
        printf("操作取消！\n");
        system("pause");
        return;
    }
    
    // 检查是否回退
    if (strcmp(appoint_dept, "0") == 0)
    {
        goto input_slot;
    }
    
    // 如果输入了科室，显示该科室的医生列表
    if (strlen(appoint_dept) > 0)
    {
        display_doctors_by_dept(appoint_dept);
    }
    
    // 输入预约医生编号（可选）
    input_doctor:
    get_safe_string("请输入预约医生编号(可留空): ", appoint_doctor, MAX_NAME_LEN);
    
    // 检查是否退出
    if (strcmp(appoint_doctor, "00") == 0)
    {
        printf("操作取消！\n");
        system("pause");
        return;
    }
    
    // 检查是否回退
    if (strcmp(appoint_doctor, "0") == 0)
    {
        goto input_dept;
    }
    
    AppointmentNode* new_appt = register_appointment(
        patient_id,
        appointment_date,
        appointment_slot,
        appoint_doctor,
        appoint_dept
    );
    if (new_appt != NULL)
    {
        printf("\n✅ 预约成功！预约编号：%s\n", new_appt->appointment_id);
    }
    system("pause");
}
static void handle_internal_appointment_query()
{
    int choice;
    char patient_id[MAX_ID_LEN];
    char id_card[MAX_ID_LEN];
    while (1)
    {
        system("cls");
        printf("\n================ 预约查询 ================\n");
        printf("  [1] 按患者编号查询\n");
        printf("  [2] 按身份证号查询\n");
        printf("  [3] 按预约编号查询（含患者信息）\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------\n");
        choice = get_safe_int("👉 请输入操作编号: ");
        if (choice == 0) return;
        switch (choice)
        {
            case 1:
                printf("提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
                while (1)
                {
                    get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
                    
                    // 检查是否退出
                    if (strcmp(patient_id, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    // 检查是否回退
                    if (strcmp(patient_id, "0") == 0)
                    {
                        break;
                    }
                    
                    // 检查患者编号格式是否合法
                    if (validate_patient_id(patient_id))
                    {
                        query_appointments_by_patient_id(patient_id);
                        system("pause");
                        break;
                    }
                    else
                    {
                        printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                    }
                }
                break;
            case 2:
                printf("提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
                while (1)
                {
                    get_safe_string("请输入身份证号: ", id_card, MAX_ID_LEN);
                    
                    // 检查是否退出
                    if (strcmp(id_card, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    // 检查是否回退
                    if (strcmp(id_card, "0") == 0)
                    {
                        break;
                    }
                    
                    // 检查身份证号格式是否合法
                    if (validate_id_card(id_card))
                    {
                        query_appointments_by_id_card(id_card);
                        system("pause");
                        break;
                    }
                    else
                    {
                        printf("⚠️ 身份证号格式不合法，请重新输入！\n");
                    }
                }
                break;
            case 3:
                printf("提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
                while (1)
                {
                    get_safe_string("请输入预约编号: ", patient_id, MAX_ID_LEN);
                    
                    // 检查是否退出
                    if (strcmp(patient_id, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    // 检查是否回退
                    if (strcmp(patient_id, "0") == 0)
                    {
                        break;
                    }
                    
                    // 检查预约编号是否为空
                    if (strlen(patient_id) > 0)
                    {
                        query_appointment_and_patient(patient_id);
                        system("pause");
                        break;
                    }
                    else
                    {
                        printf("⚠️ 预约编号不能为空，请重新输入！\n");
                    }
                }
                break;
            default:
                printf("\n⚠️ 无效的选项，请重新输入！\n");
                system("pause");
                break;
        }
    }
}
static void handle_internal_appointment_cancel()
{
    char appointment_id[MAX_ID_LEN];
    printf("\n================ 预约取消 ================\n");
    printf("提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
    
    while (1)
    {
        get_safe_string("请输入预约编号: ", appointment_id, MAX_ID_LEN);
        
        // 检查是否退出
        if (strcmp(appointment_id, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(appointment_id, "0") == 0)
        {
            return;
        }
        
        // 检查预约编号是否为空
        if (strlen(appointment_id) > 0)
        {
            break;
        }
        printf("⚠️ 预约编号不能为空，请重新输入！\n");
    }
    
    cancel_appointment(appointment_id);
    system("pause");
}
static void handle_internal_appointment_check_in()
{
    char appointment_id[MAX_ID_LEN];
    printf("\n================ 预约签到入队 ================\n");
    printf("提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
    
    while (1)
    {
        get_safe_string("请输入预约编号: ", appointment_id, MAX_ID_LEN);
        
        // 检查是否退出
        if (strcmp(appointment_id, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(appointment_id, "0") == 0)
        {
            return;
        }
        
        // 检查预约编号是否为空
        if (strlen(appointment_id) > 0)
        {
            break;
        }
        printf("⚠️ 预约编号不能为空，请重新输入！\n");
    }
    
    check_in_appointment(appointment_id);
    system("pause");
}
static void handle_patient_self_basic_record_query()
{
    char patient_id[MAX_ID_LEN];
    char id_card[MAX_ID_LEN];
    
    printf("\n================ 基础病历查询 ================\n");
    
    // 患者编号输入环节
    input_patient_id:
    get_safe_string("请输入患者编号(输入 0 回退上一步，输入 00 退出): ", patient_id, MAX_ID_LEN);
    if (strcmp(patient_id, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    // 患者编号校验（死循环）
    while (1)
    {
        if (strcmp(patient_id, "0") == 0)
            goto input_patient_id;
        if (strlen(patient_id) == 0)
        {
            printf("患者编号不能为空，请重新输入：");
            get_safe_string("", patient_id, MAX_ID_LEN);
            continue;
        }
        if (find_patient_by_id(g_patient_list, patient_id) == NULL)
        {
            printf("未找到该患者，请重新输入：");
            get_safe_string("", patient_id, MAX_ID_LEN);
            continue;
        }
        break;
    }
    
    // 身份证号输入环节
    get_safe_string("请输入身份证号(输入 0 回退上一步，输入 00 退出): ", id_card, MAX_ID_LEN);
    if (strcmp(id_card, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    if (strcmp(id_card, "0") == 0)
        goto input_patient_id;
    // 身份证号校验（死循环）
    while (1)
    {
        if (strlen(id_card) == 0)
        {
            printf("身份证号不能为空，请重新输入：");
            get_safe_string("", id_card, MAX_ID_LEN);
            continue;
        }
        if (!validate_id_card(id_card))
        {
            printf("身份证号格式不合法，请重新输入：");
            get_safe_string("", id_card, MAX_ID_LEN);
            continue;
        }
        break;
    }
    
    query_basic_patient_record(patient_id, id_card);
    system("pause");
}

static void handle_patient_self_visit_overview_query()
{
    char patient_id[MAX_ID_LEN];
    char id_card[MAX_ID_LEN];
    
    printf("\n================ 查询自己的就诊概览 ================\n");
    printf("提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
    
    // 患者编号输入和验证
    while (1)
    {
        get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
        
        // 检查是否退出
        if (strcmp(patient_id, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查患者编号格式是否合法
        if (validate_patient_id(patient_id))
        {
            break; // 格式正确，退出循环
        }
        else
        {
            printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
        }
    }
    
    // 身份证号输入和验证
    while (1)
    {
        get_safe_string("请输入身份证号: ", id_card, MAX_ID_LEN);
        
        // 检查是否退出
        if (strcmp(id_card, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(id_card, "0") == 0)
        {
            // 重新输入患者编号
            while (1)
            {
                get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
                
                // 检查是否退出
                if (strcmp(patient_id, "00") == 0)
                {
                    printf("操作取消！\n");
                    system("pause");
                    return;
                }
                
                // 检查患者编号格式是否合法
                if (validate_patient_id(patient_id))
                {
                    break; // 格式正确，退出循环
                }
                else
                {
                    printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                }
            }
            continue; // 回到身份证号输入循环
        }
        
        // 检查身份证号格式是否合法
        if (validate_id_card(id_card))
        {
            break; // 格式正确，退出循环
        }
        else
        {
            printf("⚠️ 身份证号格式不合法，请重新输入！\n");
        }
    }
    
    // 身份核验：检查患者编号和身份证号是否匹配
    PatientNode* patient = find_patient_by_id_card(id_card);
    if (patient == NULL || strcmp(patient->id, patient_id) != 0)
    {
        printf("\n❌ 身份核验失败！患者编号与身份证号不匹配\n");
        system("pause");
        return;
    }
    
    printf("\n身份核验成功！欢迎，%s\n", patient->name);
    query_patient_visit_overview_by_id(patient_id);
    system("pause");
}

static void handle_patient_self_current_visit_overview()
{
    char patient_id[MAX_ID_LEN];
    char id_card[MAX_ID_LEN];
    
    printf("\n================ 当前就诊总览 ================\n");
    
    // 患者编号输入环节
    input_patient_id:
    get_safe_string("请输入患者编号(输入 0 回退上一步，输入 00 退出): ", patient_id, MAX_ID_LEN);
    if (strcmp(patient_id, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    // 患者编号校验（死循环）
    while (1)
    {
        if (strcmp(patient_id, "0") == 0)
            goto input_patient_id;
        if (strlen(patient_id) == 0)
        {
            printf("患者编号不能为空，请重新输入：");
            get_safe_string("", patient_id, MAX_ID_LEN);
            continue;
        }
        if (find_patient_by_id(g_patient_list, patient_id) == NULL)
        {
            printf("未找到该患者，请重新输入：");
            get_safe_string("", patient_id, MAX_ID_LEN);
            continue;
        }
        break;
    }
    
    // 身份证号输入环节
    get_safe_string("请输入身份证号(输入 0 回退上一步，输入 00 退出): ", id_card, MAX_ID_LEN);
    if (strcmp(id_card, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    if (strcmp(id_card, "0") == 0)
        goto input_patient_id;
    // 身份证号校验（死循环）
    while (1)
    {
        if (strlen(id_card) == 0)
        {
            printf("身份证号不能为空，请重新输入：");
            get_safe_string("", id_card, MAX_ID_LEN);
            continue;
        }
        if (!validate_id_card(id_card))
        {
            printf("身份证号格式不合法，请重新输入：");
            get_safe_string("", id_card, MAX_ID_LEN);
            continue;
        }
        break;
    }
    
    // 身份核验：检查患者编号和身份证号是否匹配
    PatientNode* patient = find_patient_by_id_card(id_card);
    if (patient == NULL || strcmp(patient->id, patient_id) != 0)
    {
        printf("\n❌ 身份核验失败！患者编号与身份证号不匹配\n");
        system("pause");
        return;
    }
    
    printf("\n身份核验成功！欢迎，%s\n", patient->name);
    
    // 显示患者基本信息
    printf("\n==============================================\n");
    printf("                📋 基本信息\n");
    printf("==============================================\n");
    printf("患者编号：%s\n", patient->id);
    printf("姓名：%s\n", patient->name);
    printf("性别：%s\n", patient->gender);
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
    
    // 当前状态
    printf("当前状态: %s\n", get_patient_status_text(patient->status));
    
    // 最近一次诊断结论
    if (patient->diagnosis_text[0] != '\0')
        printf("最近诊断结论: %s\n", patient->diagnosis_text);
    else
        printf("最近诊断结论: 暂无诊断记录\n");
    
    // 最近一次处理意见
    if (patient->treatment_advice[0] != '\0')
        printf("最近一次处理意见: %s\n", patient->treatment_advice);
    else
        printf("最近一次处理意见: 暂无处理意见\n");
    
    // 显示就诊概览独有的信息
    printf("\n==============================================\n");
    printf("                💰 账户与挂号\n");
    printf("==============================================\n");
    printf("账户余额：%.2f\n", patient->balance);
    
    // 最近一次挂号信息
    AppointmentNode* latest_appointment = find_latest_appointment_by_patient_id(patient->id);
    if (latest_appointment != NULL)
    {
        printf("\n最近挂号信息:\n");
        printf("  挂号编号：%s\n", latest_appointment->appointment_id);
        printf("  挂号类型：%s\n", latest_appointment->is_walk_in ? "现场号" : "预约号");
        printf("  挂号日期：%s\n", latest_appointment->appointment_date);
        printf("  挂号时段：%s\n", latest_appointment->appointment_slot);
        
        // 显示挂号医生或科室
        if (latest_appointment->appoint_doctor[0] != '\0')
            printf("  挂号医生：%s\n", latest_appointment->appoint_doctor);
        else if (latest_appointment->appoint_dept[0] != '\0')
            printf("  挂号科室：%s\n", latest_appointment->appoint_dept);
        else
            printf("  挂号科室：暂无\n");
        printf("  挂号状态：%s\n", get_appointment_display_status(latest_appointment));
    }
    else
    {
        printf("\n最近挂号信息: 暂无挂号记录\n");
    }
    
    system("pause");
}

static void handle_patient_self_balance_query()
{
    char patient_id[MAX_ID_LEN];
    char id_card[MAX_ID_LEN];

    printf("\n================ 查询账户余额 ================\n");

    while (1)
    {
        get_safe_string("请输入患者编号（输入 00 退出）: ", patient_id, MAX_ID_LEN);
        if (strcmp(patient_id, "00") == 0) { printf("操作取消！\n"); return; }
        if (is_blank_string(patient_id))
        {
            printf("⚠️ 患者编号不能为空，请重新输入\n");
            continue;
        }
        if (find_patient_by_id(g_patient_list, patient_id) == NULL)
        {
            printf("⚠️ 未找到该患者，请重新输入\n");
            continue;
        }
        break;
    }

    while (1)
    {
        get_safe_string("请输入身份证号进行身份核验（输入 00 退出）: ", id_card, MAX_ID_LEN);
        if (strcmp(id_card, "00") == 0) { printf("操作取消！\n"); return; }
        if (is_blank_string(id_card))
        {
            printf("⚠️ 身份证号不能为空，请重新输入\n");
            continue;
        }
        if (!validate_id_card(id_card))
        {
            printf("⚠️ 身份证号格式不合法，请重新输入\n");
            continue;
        }
        break;
    }

    PatientNode* patient = find_patient_by_id_card(id_card);
    if (patient == NULL || strcmp(patient->id, patient_id) != 0)
    {
        printf("\n❌ 身份核验失败！患者编号与身份证号不匹配\n");
        return;
    }

    printf("\n=============================================\n");
    printf("  💰 账户余额\n");
    printf("=============================================\n");
    printf("  患者编号：%s\n", patient->id);
    printf("  姓名：%s\n", patient->name);
    printf("  账户余额：%.2f 元\n", patient->balance);
    if (patient->balance < 0)
        printf("  ⚠️ 余额已欠费，请及时充值！\n");
    printf("=============================================\n");

    InpatientRecord* ir = find_active_inpatient_by_patient_id(patient->id);
    if (ir)
    {
        printf("\n  📋 住院押金信息\n");
        printf("  住院号：%s\n", ir->inpatient_id);
        printf("  押金余额：%.2f 元\n", ir->deposit_balance);
        if (ir->deposit_balance < 0)
            printf("  ⚠️ 押金已欠费，请及时催缴！\n");
        printf("---------------------------------------------\n");
    }
}

static void handle_patient_self_consult_history_query(const char* patient_id, const char* id_card)
{
    printf("\n================ 查询自己的历史就诊记录 ================\n");
    printf("温馨提示：请确保您输入的是本人信息\n");
    
    query_patient_consult_history_verified(patient_id, id_card);
    system("pause");
}

static void handle_patient_self_appointment_register()
{
    char patient_id[MAX_ID_LEN];
    char id_card[MAX_ID_LEN];
    char appointment_date[MAX_NAME_LEN];
    char appointment_slot[MAX_NAME_LEN];
    char appoint_doctor[MAX_NAME_LEN];
    char appoint_dept[MAX_NAME_LEN];
    char current_symptom[MAX_SYMPTOM_LEN];
    char old_symptom[MAX_SYMPTOM_LEN];
    int is_current_emergency = 0;
    int old_is_emergency = 0;
    PatientNode* patient = NULL;
    
    printf("\n================ 自助预约登记 ================\n");
    printf("温馨提示：请确保您输入的是本人信息\n");
    
    // 患者编号输入环节
    input_patient_id:
    get_safe_string("请输入您的患者编号(输入 0 回退上一步，输入 00 退出): ", patient_id, MAX_ID_LEN);
    if (strcmp(patient_id, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    // 患者编号校验（死循环）
    while (1)
    {
        if (strcmp(patient_id, "0") == 0)
            goto input_patient_id;
        if (strlen(patient_id) == 0)
        {
            printf("患者编号不能为空，请重新输入：");
            get_safe_string("", patient_id, MAX_ID_LEN);
            continue;
        }
        patient = find_patient_by_id(g_patient_list, patient_id);
        if (patient == NULL)
        {
            printf("未找到该患者，请重新输入：");
            get_safe_string("", patient_id, MAX_ID_LEN);
            continue;
        }
        break;
    }
    
    // 身份证号输入环节
    input_id_card:
    get_safe_string("请输入您的身份证号（身份核验）(输入 0 回退上一步，输入 00 退出): ", id_card, MAX_ID_LEN);
    if (strcmp(id_card, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    if (strcmp(id_card, "0") == 0)
        goto input_patient_id;
    // 身份证号校验（死循环）
    while (1)
    {
        if (strlen(id_card) == 0)
        {
            printf("身份证号不能为空，请重新输入：");
            get_safe_string("", id_card, MAX_ID_LEN);
            continue;
        }
        if (!validate_id_card(id_card))
        {
            printf("身份证号格式不合法，请重新输入：");
            get_safe_string("", id_card, MAX_ID_LEN);
            continue;
        }
        // 身份核验：检查患者编号和身份证号是否匹配
        PatientNode* id_patient = find_patient_by_id_card(id_card);
        if (id_patient == NULL || strcmp(id_patient->id, patient_id) != 0)
        {
            printf("身份核验失败！患者编号与身份证号不匹配，请重新输入：");
            get_safe_string("", id_card, MAX_ID_LEN);
            continue;
        }
        break;
    }
    
    printf("\n身份核验成功！欢迎，%s\n", patient->name);
    
    // 预约日期输入环节
    input_date:
    get_safe_string("请输入预约日期(输入 0 回退上一步，输入 00 退出): ", appointment_date, MAX_NAME_LEN);
    if (strcmp(appointment_date, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    if (strcmp(appointment_date, "0") == 0)
        goto input_id_card;
    // 日期校验
    if (strlen(appointment_date) == 0)
    {
        printf("预约日期不能为空，请重新输入：");
        get_safe_string("", appointment_date, MAX_NAME_LEN);
        goto input_date;
    }
    
    // 预约时段输入环节
    input_slot:
    // 夜间模式下自动设置为晚上
    if (is_night_shift()) {
        strcpy(appointment_slot, "晚上");
        printf("当前为夜间模式，自动设置预约时段为: %s\n", appointment_slot);
    } else {
        get_safe_string("请输入预约时段(输入 0 回退上一步，输入 00 退出): ", appointment_slot, MAX_NAME_LEN);
        if (strcmp(appointment_slot, "00") == 0)
        {
            printf("操作取消！\n");
            return;
        }
        if (strcmp(appointment_slot, "0") == 0)
            goto input_date;
        // 时段校验
        if (strlen(appointment_slot) == 0)
        {
            printf("预约时段不能为空，请重新输入：");
            get_safe_string("", appointment_slot, MAX_NAME_LEN);
            goto input_slot;
        }
    }
    
    // 症状输入环节
    input_symptom:
    get_safe_string("请输入本次症状描述(可留空)(输入 0 回退上一步，输入 00 退出): ", current_symptom, MAX_SYMPTOM_LEN);
    if (strcmp(current_symptom, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    if (strcmp(current_symptom, "0") == 0)
        goto input_slot;
    
    // 科室输入环节
    input_dept:
    get_safe_string("请输入预约科室(可留空)(输入 0 回退上一步，输入 00 退出): ", appoint_dept, MAX_NAME_LEN);
    if (strcmp(appoint_dept, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    if (strcmp(appoint_dept, "0") == 0)
        goto input_symptom;
    
    // 如果输入了科室，显示该科室的医生列表
    if (strlen(appoint_dept) > 0)
    {
        display_doctors_by_dept(appoint_dept);
    }
    
    // 医生输入环节
    get_safe_string("请输入预约医生编号(可留空)(输入 0 回退上一步，输入 00 退出): ", appoint_doctor, MAX_NAME_LEN);
    if (strcmp(appoint_doctor, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    if (strcmp(appoint_doctor, "0") == 0)
        goto input_dept;
    
    safe_copy_string(old_symptom, MAX_SYMPTOM_LEN, patient->symptom);
    old_is_emergency = patient->is_emergency;

    if (!is_blank_string(current_symptom))
    {
        safe_copy_string(patient->symptom, MAX_SYMPTOM_LEN, current_symptom);
    }
    is_current_emergency = is_current_emergency_request(appoint_doctor, appoint_dept, current_symptom);
    patient->is_emergency = is_current_emergency;
    
    // ==========================================
    // 动态挂号计费引擎
    // ==========================================
    double reg_fee = 15.0; // 默认普通号

    // 1. 判断是否为急诊
    if (is_current_emergency == 1)
    {
        reg_fee = 50.0;
    }
    // 2. 判断是否为专家号 (指定了医生编号)
    else if (strlen(appoint_doctor) > 0)
    {
        reg_fee = 30.0;
    }

    if (is_current_emergency != 1)
    {
        // 🏥 普通门诊/专家号：严格执行预付费拦截机制
        if (patient->balance < reg_fee)
        {
            printf("\n❌ 余额不足！需预先扣除挂号费 %.2f 元。\n", reg_fee);
            printf("您当前余额为 %.2f 元，请先到窗口充值后再试！\n", patient->balance);
            system("pause");
            safe_copy_string(patient->symptom, MAX_SYMPTOM_LEN, old_symptom);
            patient->is_emergency = old_is_emergency;
            return; // 资金不足，无情拦截！
        }
    }
    // ==========================================
    
    AppointmentNode* new_appt = register_appointment(
        patient_id,
        appointment_date,
        appointment_slot,
        appoint_doctor,
        appoint_dept
    );
    
    if (new_appt != NULL)
    {
        if (is_current_emergency == 1)
        {
            printf("\n🚨 【绿色通道启动】生命至上！已为您开启\"先诊疗后付费\"特权！\n");
            patient->balance -= reg_fee;
            if (patient->balance < 0)
            {
                printf("⚠️ 提示：挂号费 %.2f 元已挂账，当前账户欠费 %.2f 元，请家属后续补缴。\n", reg_fee, -patient->balance);
            }
            else
            {
                printf("💰 已扣除急诊挂号费：%.2f 元，当前余额：%.2f 元。\n", reg_fee, patient->balance);
            }
        }
        else
        {
            patient->balance -= reg_fee;
            printf("\n✅ 扣费成功！本次挂号费：%.2f 元，账户余额：%.2f 元。\n", reg_fee, patient->balance);
        }

        new_appt->reg_fee = reg_fee;
        new_appt->fee_paid = 1;
        printf("\n✅ 自助预约登记成功！预约编号：%s\n", new_appt->appointment_id);
    }
    else
    {
        safe_copy_string(patient->symptom, MAX_SYMPTOM_LEN, old_symptom);
        patient->is_emergency = old_is_emergency;
    }
    system("pause");
}

static void handle_patient_self_first_visit()
{
    char name[MAX_NAME_LEN];
    int age;
    char id_card[MAX_ID_LEN];
    char symptom[MAX_SYMPTOM_LEN];
    char target_dept[MAX_NAME_LEN];
    
    printf("\n================ 首次来院登记 ================\n");
    printf("温馨提示：本功能用于首次来院患者建档或已建档患者复用档案\n");
    printf("系统将通过身份证号判断您是否为首次来院\n\n");
    printf("提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
    
    // 身份证号输入和验证
    while (1)
    {
        get_safe_string("请输入您的身份证号: ", id_card, MAX_ID_LEN);
        
        // 检查是否退出
        if (strcmp(id_card, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查身份证号格式是否合法
        if (validate_id_card(id_card))
        {
            break; // 格式正确，退出循环
        }
        else
        {
            printf("⚠️ 身份证号格式不合法，请重新输入！\n");
        }
    }
    
    // 检查身份证号是否已存在
    PatientNode* existing_patient = find_patient_by_id_card(id_card);
    if (existing_patient != NULL)
    {
        // 情况B：身份证号已存在，不重复建档
        printf("\n🎉 系统已存在您的患者档案！\n");
        printf("您的患者编号是：%s\n", existing_patient->id);
        printf("您的姓名是：%s\n", existing_patient->name);
        printf("\n请使用上述患者编号进行后续的预约/挂号操作\n");
        system("pause");
        return;
    }
    
    // 情况A：身份证号不存在，首次来院，进行建档
    printf("\n您是首次来院患者，请完成建档信息录入\n");
    
    // 姓名输入和验证
    while (1)
    {
        get_safe_string("请输入您的姓名: ", name, MAX_NAME_LEN);
        
        // 检查是否退出
        if (strcmp(name, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(name, "0") == 0)
        {
            // 重新输入身份证号
            while (1)
            {
                get_safe_string("请输入您的身份证号: ", id_card, MAX_ID_LEN);
                
                // 检查是否退出
                if (strcmp(id_card, "00") == 0)
                {
                    printf("操作取消！\n");
                    system("pause");
                    return;
                }
                
                // 检查身份证号格式是否合法
                if (validate_id_card(id_card))
                {
                    break; // 格式正确，退出循环
                }
                else
                {
                    printf("⚠️ 身份证号格式不合法，请重新输入！\n");
                }
            }
            
            // 重新检查身份证号是否已存在
            existing_patient = find_patient_by_id_card(id_card);
            if (existing_patient != NULL)
            {
                // 情况B：身份证号已存在，不重复建档
                printf("\n🎉 系统已存在您的患者档案！\n");
                printf("您的患者编号是：%s\n", existing_patient->id);
                printf("您的姓名是：%s\n", existing_patient->name);
                printf("\n请使用上述患者编号进行后续的预约/挂号操作\n");
                system("pause");
                return;
            }
            continue; // 回到姓名输入循环
        }
        
        // 检查姓名是否为空
        if (strlen(name) > 0)
        {
            break; // 姓名不为空，退出循环
        }
        else
        {
            printf("⚠️ 姓名不能为空，请重新输入！\n");
        }
    }
    
    // 年龄输入和验证
    while (1)
    {
        char age_str[20];
        get_safe_string("请输入您的年龄: ", age_str, 20);
        
        // 检查是否退出
        if (strcmp(age_str, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(age_str, "0") == 0)
        {
            // 重新输入姓名
            while (1)
            {
                get_safe_string("请输入您的姓名: ", name, MAX_NAME_LEN);
                
                // 检查是否退出
                if (strcmp(name, "00") == 0)
                {
                    printf("操作取消！\n");
                    system("pause");
                    return;
                }
                
                // 检查是否回退
                if (strcmp(name, "0") == 0)
                {
                    // 重新输入身份证号
                    while (1)
                    {
                        get_safe_string("请输入您的身份证号: ", id_card, MAX_ID_LEN);
                        
                        // 检查是否退出
                        if (strcmp(id_card, "00") == 0)
                        {
                            printf("操作取消！\n");
                            system("pause");
                            return;
                        }
                        
                        // 检查身份证号格式是否合法
                        if (validate_id_card(id_card))
                        {
                            break; // 格式正确，退出循环
                        }
                        else
                        {
                            printf("⚠️ 身份证号格式不合法，请重新输入！\n");
                        }
                    }
                    
                    // 重新检查身份证号是否已存在
                    existing_patient = find_patient_by_id_card(id_card);
                    if (existing_patient != NULL)
                    {
                        // 情况B：身份证号已存在，不重复建档
                        printf("\n🎉 系统已存在您的患者档案！\n");
                        printf("您的患者编号是：%s\n", existing_patient->id);
                        printf("您的姓名是：%s\n", existing_patient->name);
                        printf("\n请使用上述患者编号进行后续的预约/挂号操作\n");
                        system("pause");
                        return;
                    }
                    continue; // 回到姓名输入循环
                }
                
                // 检查姓名是否为空
                if (strlen(name) > 0)
                {
                    break; // 姓名不为空，退出循环
                }
                else
                {
                    printf("⚠️ 姓名不能为空，请重新输入！\n");
                }
            }
            continue; // 回到年龄输入循环
        }
        
        // 转换为整数
        if (sscanf(age_str, "%d", &age) == 1 && age >= 0 && age <= 130)
        {
            break; // 年龄合法，退出循环
        }
        else
        {
            printf("⚠️ 年龄输入无效，请输入 0-130 之间的数字！\n");
        }
    }
    
    // 性别输入和验证
    char gender_str[8];
    while (1)
    {
        printf("请输入您的性别 (男/女): ");
        get_safe_string(gender_str, gender_str, 8);
        
        // 检查是否退出
        if (strcmp(gender_str, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(gender_str, "0") == 0)
        {
            // 重新输入年龄
            while (1)
            {
                char age_str[20];
                get_safe_string("请输入您的年龄: ", age_str, 20);
                
                // 检查是否退出
                if (strcmp(age_str, "00") == 0)
                {
                    printf("操作取消！\n");
                    system("pause");
                    return;
                }
                
                // 检查是否回退
                if (strcmp(age_str, "0") == 0)
                {
                    // 重新输入姓名
                    while (1)
                    {
                        get_safe_string("请输入您的姓名: ", name, MAX_NAME_LEN);
                        
                        // 检查是否退出
                        if (strcmp(name, "00") == 0)
                        {
                            printf("操作取消！\n");
                            system("pause");
                            return;
                        }
                        
                        // 检查是否回退
                        if (strcmp(name, "0") == 0)
                        {
                            // 重新输入身份证号
                            while (1)
                            {
                                get_safe_string("请输入您的身份证号: ", id_card, MAX_ID_LEN);
                                
                                // 检查是否退出
                                if (strcmp(id_card, "00") == 0)
                                {
                                    printf("操作取消！\n");
                                    system("pause");
                                    return;
                                }
                                
                                // 检查身份证号格式是否合法
                                if (validate_id_card(id_card))
                                {
                                    break; // 格式正确，退出循环
                                }
                                else
                                {
                                    printf("⚠️ 身份证号格式不合法，请重新输入！\n");
                                }
                            }
                            
                            // 重新检查身份证号是否已存在
                            existing_patient = find_patient_by_id_card(id_card);
                            if (existing_patient != NULL)
                            {
                                // 情况B：身份证号已存在，不重复建档
                                printf("\n🎉 系统已存在您的患者档案！\n");
                                printf("您的患者编号是：%s\n", existing_patient->id);
                                printf("您的姓名是：%s\n", existing_patient->name);
                                printf("\n请使用上述患者编号进行后续的预约/挂号操作\n");
                                system("pause");
                                return;
                            }
                            continue; // 回到姓名输入循环
                        }
                        
                        // 检查姓名是否为空
                        if (strlen(name) > 0)
                        {
                            break; // 姓名不为空，退出循环
                        }
                        else
                        {
                            printf("⚠️ 姓名不能为空，请重新输入！\n");
                        }
                    }
                    continue; // 回到年龄输入循环
                }
                
                // 转换为整数
                if (sscanf(age_str, "%d", &age) == 1 && age >= 0 && age <= 130)
                {
                    break; // 年龄合法，退出循环
                }
                else
                {
                    printf("⚠️ 年龄输入无效，请输入 0-130 之间的数字！\n");
                }
            }
            continue; // 回到性别输入循环
        }
        
        if (strcmp(gender_str, "男") == 0 || strcmp(gender_str, "女") == 0) {
            break;
        }
        printf("⚠️ 性别输入无效，请输入 男 或 女！\n");
    }
    
    // 症状输入和验证
    while (1)
    {
        get_safe_string("请输入您的症状描述: ", symptom, MAX_SYMPTOM_LEN);
        
        // 检查是否退出
        if (strcmp(symptom, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(symptom, "0") == 0)
        {
            // 重新输入性别
            while (1)
            {
                printf("请输入您的性别 (男/女): ");
                get_safe_string(gender_str, gender_str, 8);
                
                // 检查是否退出
                if (strcmp(gender_str, "00") == 0)
                {
                    printf("操作取消！\n");
                    system("pause");
                    return;
                }
                
                // 检查是否回退
                if (strcmp(gender_str, "0") == 0)
                {
                    // 重新输入年龄
                    while (1)
                    {
                        char age_str[20];
                        get_safe_string("请输入您的年龄: ", age_str, 20);
                        
                        // 检查是否退出
                        if (strcmp(age_str, "00") == 0)
                        {
                            printf("操作取消！\n");
                            system("pause");
                            return;
                        }
                        
                        // 检查是否回退
                        if (strcmp(age_str, "0") == 0)
                        {
                            // 重新输入姓名
                            while (1)
                            {
                                get_safe_string("请输入您的姓名: ", name, MAX_NAME_LEN);
                                
                                // 检查是否退出
                                if (strcmp(name, "00") == 0)
                                {
                                    printf("操作取消！\n");
                                    system("pause");
                                    return;
                                }
                                
                                // 检查是否回退
                                if (strcmp(name, "0") == 0)
                                {
                                    // 重新输入身份证号
                                    while (1)
                                    {
                                        get_safe_string("请输入您的身份证号: ", id_card, MAX_ID_LEN);
                                        
                                        // 检查是否退出
                                        if (strcmp(id_card, "00") == 0)
                                        {
                                            printf("操作取消！\n");
                                            system("pause");
                                            return;
                                        }
                                        
                                        // 检查身份证号格式是否合法
                                        if (validate_id_card(id_card))
                                        {
                                            break; // 格式正确，退出循环
                                        }
                                        else
                                        {
                                            printf("⚠️ 身份证号格式不合法，请重新输入！\n");
                                        }
                                    }
                                    
                                    // 重新检查身份证号是否已存在
                                    existing_patient = find_patient_by_id_card(id_card);
                                    if (existing_patient != NULL)
                                    {
                                        // 情况B：身份证号已存在，不重复建档
                                        printf("\n🎉 系统已存在您的患者档案！\n");
                                        printf("您的患者编号是：%s\n", existing_patient->id);
                                        printf("您的姓名是：%s\n", existing_patient->name);
                                        printf("\n请使用上述患者编号进行后续的预约/挂号操作\n");
                                        system("pause");
                                        return;
                                    }
                                    continue; // 回到姓名输入循环
                                }
                                
                                // 检查姓名是否为空
                                if (strlen(name) > 0)
                                {
                                    break; // 姓名不为空，退出循环
                                }
                                else
                                {
                                    printf("⚠️ 姓名不能为空，请重新输入！\n");
                                }
                            }
                            continue; // 回到年龄输入循环
                        }
                        
                        // 转换为整数
                        if (sscanf(age_str, "%d", &age) == 1 && age >= 0 && age <= 130)
                        {
                            break; // 年龄合法，退出循环
                        }
                        else
                        {
                            printf("⚠️ 年龄输入无效，请输入 0-130 之间的数字！\n");
                        }
                    }
                    continue; // 回到性别输入循环
                }
                
                if (strcmp(gender_str, "男") == 0 || strcmp(gender_str, "女") == 0) {
                    break;
                }
                printf("⚠️ 性别输入无效，请输入 男 或 女！\n");
            }
            continue; // 回到症状输入循环
        }
        
        // 症状可以为空，直接退出循环
        break;
    }
    
    // 科室输入和验证
    while (1)
    {
        get_safe_string("请输入您要就诊的科室: ", target_dept, MAX_NAME_LEN);
        
        // 检查是否退出
        if (strcmp(target_dept, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(target_dept, "0") == 0)
        {
            // 重新输入症状
            while (1)
            {
                get_safe_string("请输入您的症状描述: ", symptom, MAX_SYMPTOM_LEN);
                
                // 检查是否退出
                if (strcmp(symptom, "00") == 0)
                {
                    printf("操作取消！\n");
                    system("pause");
                    return;
                }
                
                // 检查是否回退
                if (strcmp(symptom, "0") == 0)
                {
                    // 重新输入性别
                    while (1)
                    {
                        printf("请输入您的性别 (男/女): ");
                        get_safe_string(gender_str, gender_str, 8);
                        
                        // 检查是否退出
                        if (strcmp(gender_str, "00") == 0)
                        {
                            printf("操作取消！\n");
                            system("pause");
                            return;
                        }
                        
                        // 检查是否回退
                        if (strcmp(gender_str, "0") == 0)
                        {
                            // 重新输入年龄
                            while (1)
                            {
                                char age_str[20];
                                get_safe_string("请输入您的年龄: ", age_str, 20);
                                
                                // 检查是否退出
                                if (strcmp(age_str, "00") == 0)
                                {
                                    printf("操作取消！\n");
                                    system("pause");
                                    return;
                                }
                                
                                // 检查是否回退
                                if (strcmp(age_str, "0") == 0)
                                {
                                    // 重新输入姓名
                                    while (1)
                                    {
                                        get_safe_string("请输入您的姓名: ", name, MAX_NAME_LEN);
                                        
                                        // 检查是否退出
                                        if (strcmp(name, "00") == 0)
                                        {
                                            printf("操作取消！\n");
                                            system("pause");
                                            return;
                                        }
                                        
                                        // 检查是否回退
                                        if (strcmp(name, "0") == 0)
                                        {
                                            // 重新输入身份证号
                                            while (1)
                                            {
                                                get_safe_string("请输入您的身份证号: ", id_card, MAX_ID_LEN);
                                                
                                                // 检查是否退出
                                                if (strcmp(id_card, "00") == 0)
                                                {
                                                    printf("操作取消！\n");
                                                    system("pause");
                                                    return;
                                                }
                                                
                                                // 检查身份证号格式是否合法
                                                if (validate_id_card(id_card))
                                                {
                                                    break; // 格式正确，退出循环
                                                }
                                                else
                                                {
                                                    printf("⚠️ 身份证号格式不合法，请重新输入！\n");
                                                }
                                            }
                                            
                                            // 重新检查身份证号是否已存在
                                            existing_patient = find_patient_by_id_card(id_card);
                                            if (existing_patient != NULL)
                                            {
                                                // 情况B：身份证号已存在，不重复建档
                                                printf("\n🎉 系统已存在您的患者档案！\n");
                                                printf("您的患者编号是：%s\n", existing_patient->id);
                                                printf("您的姓名是：%s\n", existing_patient->name);
                                                printf("\n请使用上述患者编号进行后续的预约/挂号操作\n");
                                                system("pause");
                                                return;
                                            }
                                            continue; // 回到姓名输入循环
                                        }
                                        
                                        // 检查姓名是否为空
                                        if (strlen(name) > 0)
                                        {
                                            break; // 姓名不为空，退出循环
                                        }
                                        else
                                        {
                                            printf("⚠️ 姓名不能为空，请重新输入！\n");
                                        }
                                    }
                                    continue; // 回到年龄输入循环
                                }
                                
                                // 转换为整数
                                if (sscanf(age_str, "%d", &age) == 1 && age >= 0 && age <= 130)
                                {
                                    break; // 年龄合法，退出循环
                                }
                                else
                                {
                                    printf("⚠️ 年龄输入无效，请输入 0-130 之间的数字！\n");
                                }
                            }
                            continue; // 回到性别输入循环
                        }
                        
                        if (strcmp(gender_str, "男") == 0 || strcmp(gender_str, "女") == 0) {
                            break;
                        }
                        printf("⚠️ 性别输入无效，请输入 男 或 女！\n");
                    }
                    continue; // 回到症状输入循环
                }
                
                // 症状可以为空，直接退出循环
                break;
            }
            continue; // 回到科室输入循环
        }
        
        // 科室可以为空，直接退出循环
        break;
    }
    
    // 调用建档函数
    PatientNode* new_patient = register_patient(name, age, gender_str, id_card, symptom, target_dept);
    
    if (new_patient != NULL)
    {
        printf("\n✅ 建档成功！\n");
        printf("您的新患者编号是：%s\n", new_patient->id);
        printf("请妥善保管您的患者编号，后续就诊时使用\n");
        printf("\n建档完成后，您可以继续进行预约/挂号操作\n");
    }
    else
    {
        printf("\n❌ 建档失败！请稍后重试\n");
    }
    
    system("pause");
}

static void handle_patient_self_registration()
{
    char patient_id[MAX_ID_LEN];
    char id_card[MAX_ID_LEN];
    char appointment_date[MAX_NAME_LEN];
    char appointment_slot[MAX_NAME_LEN];
    char appoint_doctor[MAX_NAME_LEN];
    char appoint_dept[MAX_NAME_LEN];
    char old_symptom[MAX_SYMPTOM_LEN];
    char current_symptom[MAX_SYMPTOM_LEN];
    int old_is_emergency = 0;
    PatientNode* patient = NULL;
    int is_emergency_case = 0;
    int is_night = is_night_shift();
    
    printf("\n================ 自助挂号（现场挂号） ================\n");
    printf("温馨提示：本功能仅支持已建档患者使用\n");
    printf("如果您是首次就诊，请先到服务台建档\n");
    
    // 患者编号输入环节
    input_patient_id:
    get_safe_string("请输入您的患者编号(输入 0 回退上一步，输入 00 退出): ", patient_id, MAX_ID_LEN);
    if (strcmp(patient_id, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    // 患者编号校验（死循环）
    while (1)
    {
        if (strcmp(patient_id, "0") == 0)
            goto input_patient_id;
        if (strlen(patient_id) == 0)
        {
            printf("患者编号不能为空，请重新输入：");
            get_safe_string("", patient_id, MAX_ID_LEN);
            continue;
        }
        patient = find_patient_by_id(g_patient_list, patient_id);
        if (patient == NULL)
        {
            printf("未找到该患者，请重新输入：");
            get_safe_string("", patient_id, MAX_ID_LEN);
            continue;
        }
        break;
    }
    
    // 身份证号输入环节
    input_id_card:
    get_safe_string("请输入您的身份证号（身份核验）(输入 0 回退上一步，输入 00 退出): ", id_card, MAX_ID_LEN);
    if (strcmp(id_card, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    if (strcmp(id_card, "0") == 0)
        goto input_patient_id;
    // 身份证号校验（死循环）
    while (1)
    {
        if (strlen(id_card) == 0)
        {
            printf("身份证号不能为空，请重新输入：");
            get_safe_string("", id_card, MAX_ID_LEN);
            continue;
        }
        if (!validate_id_card(id_card))
        {
            printf("身份证号格式不合法，请重新输入：");
            get_safe_string("", id_card, MAX_ID_LEN);
            continue;
        }
        // 身份核验：检查患者编号和身份证号是否匹配
        PatientNode* id_patient = find_patient_by_id_card(id_card);
        if (id_patient == NULL || strcmp(id_patient->id, patient_id) != 0)
        {
            printf("身份核验失败！患者编号与身份证号不匹配，请重新输入：");
            get_safe_string("", id_card, MAX_ID_LEN);
            continue;
        }
        break;
    }
    
    printf("\n身份核验成功！欢迎，%s\n", patient->name);
    
    // 症状输入环节
    input_symptom:
    get_safe_string("请输入您本次就诊的症状描述(输入 0 回退上一步，输入 00 退出): ", current_symptom, MAX_SYMPTOM_LEN);
    if (strcmp(current_symptom, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    if (strcmp(current_symptom, "0") == 0)
        goto input_id_card;
    // 症状校验
    if (strlen(current_symptom) == 0)
    {
        printf("症状描述不能为空，请重新输入：");
        get_safe_string("", current_symptom, MAX_SYMPTOM_LEN);
        goto input_symptom;
    }
    
    // 拷贝覆盖到患者症状
    strncpy(old_symptom, patient->symptom, MAX_SYMPTOM_LEN - 1);
    old_symptom[MAX_SYMPTOM_LEN - 1] = '\0';
    old_is_emergency = patient->is_emergency;
    strncpy(patient->symptom, current_symptom, MAX_SYMPTOM_LEN - 1);
    patient->symptom[MAX_SYMPTOM_LEN - 1] = '\0'; // 确保字符串结束
    // 重新调用智能导诊
    const char* recommended_dept = recommend_dept_by_symptom(patient->symptom);
    // 根据返回值重新判定急诊状态
    if (strcmp(recommended_dept, "急诊科") == 0 || is_night)
    {
        patient->is_emergency = 1;
        is_emergency_case = 1;
        if (is_night)
        {
            printf("🌙 夜间模式已启动，已为您开启绿色生命通道！\n");
        }
        else
        {
            printf("🚨 症状符合急诊特征，已为您开启绿色生命通道！\n");
        }
        // 锁定科室为急诊科
        strcpy(appoint_dept, "急诊科");
        printf("⚠️ 已自动锁定科室为：急诊科\n");
    }
    else
    {
        patient->is_emergency = 0;
        is_emergency_case = 0;
        
        // 科室输入环节
        input_dept:
        get_safe_string("请输入挂号科室(可留空)(输入 0 回退上一步，输入 00 退出): ", appoint_dept, MAX_NAME_LEN);
        if (strcmp(appoint_dept, "00") == 0)
        {
            printf("操作取消！\n");
            // 恢复原症状
            strncpy(patient->symptom, old_symptom, MAX_SYMPTOM_LEN - 1);
            patient->symptom[MAX_SYMPTOM_LEN - 1] = '\0';
            patient->is_emergency = old_is_emergency;
            return;
        }
        if (strcmp(appoint_dept, "0") == 0)
            goto input_symptom;
        
        // 如果输入了科室，显示该科室的医生列表
        if (strlen(appoint_dept) > 0)
        {
            display_doctors_by_dept(appoint_dept);
        }
    }
    
    // 现场挂号使用当天日期
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    snprintf(appointment_date, sizeof(appointment_date), "%04d-%02d-%02d", 
            local_time->tm_year + 1900, 
            local_time->tm_mon + 1, 
            local_time->tm_mday);
    
    printf("\n当前日期：%s\n", appointment_date);
    
    // 时段输入环节
    input_slot:
    // 夜间模式下自动设置为晚上
    if (is_night_shift()) {
        strcpy(appointment_slot, "晚上");
        printf("当前为夜间模式，自动设置挂号时段为: %s\n", appointment_slot);
    } else {
        get_safe_string("请输入挂号时段(输入 0 回退上一步，输入 00 退出): ", appointment_slot, MAX_NAME_LEN);
        if (strcmp(appointment_slot, "00") == 0)
        {
            printf("操作取消！\n");
            // 恢复原症状
            strncpy(patient->symptom, old_symptom, MAX_SYMPTOM_LEN - 1);
            patient->symptom[MAX_SYMPTOM_LEN - 1] = '\0';
            patient->is_emergency = old_is_emergency;
            return;
        }
        if (strcmp(appointment_slot, "0") == 0)
        {
            if (is_emergency_case)
                goto input_symptom;
            else
                goto input_dept;
        }
        // 时段校验
        if (strlen(appointment_slot) == 0)
        {
            printf("挂号时段不能为空，请重新输入：");
            get_safe_string("", appointment_slot, MAX_NAME_LEN);
            goto input_slot;
        }
    }
    
    // 医生输入环节（急诊也保留选择权）
    get_safe_string("请输入意向医生编号(直接回车留空表示由值班医生接诊)(输入 0 回退上一步，输入 00 退出): ", appoint_doctor, MAX_NAME_LEN);
    if (strcmp(appoint_doctor, "00") == 0)
    {
        printf("操作取消！\n");
        // 恢复原症状
        strncpy(patient->symptom, old_symptom, MAX_SYMPTOM_LEN - 1);
        patient->symptom[MAX_SYMPTOM_LEN - 1] = '\0';
        patient->is_emergency = old_is_emergency;
        return;
    }
    if (strcmp(appoint_doctor, "0") == 0)
        goto input_slot;
    
    // ==========================================
    // 动态挂号计费引擎
    // ==========================================
    double reg_fee = 15.0; // 默认普通号

    // 1. 判断是否为急诊
    if (patient->is_emergency == 1)
    {
        reg_fee = 50.0;
    }
    // 2. 判断是否为专家号 (指定了医生编号)
    else if (strlen(appoint_doctor) > 0)
    {
        reg_fee = 30.0;
    }

    if (patient->is_emergency != 1)
    {
        // 🏥 普通门诊/专家号：严格执行预付费拦截机制
        if (patient->balance < reg_fee)
        {
            printf("\n❌ 余额不足！需预先扣除挂号费 %.2f 元。\n", reg_fee);
            printf("您当前余额为 %.2f 元，请先到窗口充值后再试！\n", patient->balance);
            system("pause");
            strncpy(patient->symptom, old_symptom, MAX_SYMPTOM_LEN - 1);
            patient->symptom[MAX_SYMPTOM_LEN - 1] = '\0';
            patient->is_emergency = old_is_emergency;
            return; // 资金不足，无情拦截！
        }
    }
    // ==========================================
    
    // 接收生成的预约节点
    AppointmentNode* new_appt = register_appointment(
        patient_id,
        appointment_date,
        appointment_slot,
        appoint_doctor,
        appoint_dept
    );
    
    // 如果挂号成功，直接完成现场分诊入队
    if (new_appt != NULL )
    {
        if (patient->is_emergency == 1)
        {
            printf("\n🚨 【绿色通道启动】生命至上！已为您开启\"先诊疗后付费\"特权！\n");
            patient->balance -= reg_fee;
            if (patient->balance < 0)
            {
                printf("⚠️ 提示：挂号费 %.2f 元已挂账，当前账户欠费 %.2f 元，请家属后续补缴。\n", reg_fee, -patient->balance);
            }
            else
            {
                printf("💰 已扣除急诊挂号费：%.2f 元，当前余额：%.2f 元。\n", reg_fee, patient->balance);
            }
        }
        else
        {
            patient->balance -= reg_fee;
            printf("\n✅ 扣费成功！本次挂号费：%.2f 元，账户余额：%.2f 元。\n", reg_fee, patient->balance);
        }

        new_appt->reg_fee = reg_fee;
        new_appt->fee_paid = 1;
        new_appt->is_walk_in = 1;
        printf("\n✅ 挂号成功！");
        printf("\n🔄 [系统联动] 现场挂号正在分诊入队...\n" );
        if (!queue_walk_in_registration(new_appt->appointment_id))
        {
            patient->balance += reg_fee;
            new_appt->appointment_status = CANCELLED;
            new_appt->fee_paid = 0;
            strncpy(patient->symptom, old_symptom, MAX_SYMPTOM_LEN - 1);
            patient->symptom[MAX_SYMPTOM_LEN - 1] = '\0';
            patient->is_emergency = old_is_emergency;
            printf("\n❌ 挂号后分诊失败，系统已自动撤销本次挂号并退回 %.2f 元。\n", reg_fee);
            system("pause");
            return;
        }
        
        // 现场挂号强制使用当前物理时间戳，不享受预约的1800秒虚拟提前优惠
        PatientNode* p = find_patient_by_id(g_patient_list, patient_id);
        if (p != NULL) { 
            p->queue_time = time(NULL); 
        }
        
        printf("\n✅ 现场挂号与分诊全部完成！请关注叫号屏幕，前往对应科室候诊。\n" );
    }
    else
    {
        strncpy(patient->symptom, old_symptom, MAX_SYMPTOM_LEN - 1);
        patient->symptom[MAX_SYMPTOM_LEN - 1] = '\0';
        patient->is_emergency = old_is_emergency;
        printf("\n❌ 挂号失败，无法进行后续分诊。\n" );
    }
    
    system("pause" );
}

static void handle_patient_self_appointment_cancel()
{
    char patient_id[MAX_ID_LEN];
    char id_card[MAX_ID_LEN];
    char appointment_id[MAX_ID_LEN];
    
    printf("\n================ 自助预约取消 ================\n");
    printf("温馨提示：请确保您输入的是本人信息\n");
    printf("提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
    
    // 患者编号输入和验证
    while (1)
    {
        get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
        
        // 检查是否退出
        if (strcmp(patient_id, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查患者编号格式是否合法
        if (validate_patient_id(patient_id))
        {
            break; // 格式正确，退出循环
        }
        else
        {
            printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
        }
    }
    
    // 身份证号输入和验证
    while (1)
    {
        get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
        
        // 检查是否退出
        if (strcmp(id_card, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(id_card, "0") == 0)
        {
            // 重新输入患者编号
            while (1)
            {
                get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                
                // 检查是否退出
                if (strcmp(patient_id, "00") == 0)
                {
                    printf("操作取消！\n");
                    system("pause");
                    return;
                }
                
                // 检查患者编号格式是否合法
                if (validate_patient_id(patient_id))
                {
                    break; // 格式正确，退出循环
                }
                else
                {
                    printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                }
            }
            continue; // 回到身份证号输入循环
        }
        
        // 检查身份证号格式是否合法
        if (validate_id_card(id_card))
        {
            break; // 格式正确，退出循环
        }
        else
        {
            printf("⚠️ 身份证号格式不合法，请重新输入！\n");
        }
    }
    
    // 身份核验：检查患者编号和身份证号是否匹配
    PatientNode* patient = find_patient_by_id_card(id_card);
    if (patient == NULL || strcmp(patient->id, patient_id) != 0)
    {
        printf("\n❌ 身份核验失败！患者编号与身份证号不匹配\n");
        system("pause");
        return;
    }
    
    // 预约编号输入和验证
    while (1)
    {
        get_safe_string("请输入要取消的预约编号: ", appointment_id, MAX_ID_LEN);
        
        // 检查是否退出
        if (strcmp(appointment_id, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查是否回退
        if (strcmp(appointment_id, "0") == 0)
        {
            // 重新输入身份证号
            while (1)
            {
                get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
                
                // 检查是否退出
                if (strcmp(id_card, "00") == 0)
                {
                    printf("操作取消！\n");
                    system("pause");
                    return;
                }
                
                // 检查是否回退
                if (strcmp(id_card, "0") == 0)
                {
                    // 重新输入患者编号
                    while (1)
                    {
                        get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                        
                        // 检查是否退出
                        if (strcmp(patient_id, "00") == 0)
                        {
                            printf("操作取消！\n");
                            system("pause");
                            return;
                        }
                        
                        // 检查患者编号格式是否合法
                        if (validate_patient_id(patient_id))
                        {
                            break; // 格式正确，退出循环
                        }
                        else
                        {
                            printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                        }
                    }
                    continue; // 回到身份证号输入循环
                }
                
                // 检查身份证号格式是否合法
                if (validate_id_card(id_card))
                {
                    break; // 格式正确，退出循环
                }
                else
                {
                    printf("⚠️ 身份证号格式不合法，请重新输入！\n");
                }
            }
            
            // 重新进行身份核验
            patient = find_patient_by_id_card(id_card);
            if (patient == NULL || strcmp(patient->id, patient_id) != 0)
            {
                printf("\n❌ 身份核验失败！患者编号与身份证号不匹配\n");
                system("pause");
                return;
            }
            continue; // 回到预约编号输入循环
        }
        
        // 检查预约编号是否为空
        if (strlen(appointment_id) > 0)
        {
            break; // 预约编号不为空，退出循环
        }
        else
        {
            printf("⚠️ 预约编号不能为空，请重新输入！\n");
        }
    }
    
    // 校验预约是否属于当前患者
    AppointmentNode* appointment = find_appointment_by_id(g_appointment_list, appointment_id);
    if (appointment == NULL)
    {
        printf("\n❌ 未找到对应预约记录！\n");
        system("pause");
        return;
    }
    
    if (strcmp(appointment->patient_id, patient_id) != 0)
    {
        printf("\n❌ 无权取消他人预约！\n");
        system("pause");
        return;
    }
    
    // 调用取消函数并检查返回值
    int result = cancel_appointment(appointment_id);
    if (result == 1)
    {
        printf("\n✅ 自助预约取消成功！\n");
    }
    // 失败信息已在cancel_appointment函数内部打印
    system("pause");
}

static void handle_patient_archive_query_by_id()
{
    char patient_id[MAX_ID_LEN];
    
    printf("\n================ 按编号查询档案 ================\n");
    printf("提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
    
    while (1)
    {
        get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
        
        // 检查是否退出
        if (strcmp(patient_id, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查患者编号格式是否合法
        if (validate_patient_id(patient_id))
        {
            break; // 格式正确，退出循环
        }
        else
        {
            printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
        }
    }
    
    query_patient_archive_by_id(patient_id);
    system("pause");
}

static void handle_patient_archive_query_by_id_card()
{
    char id_card[MAX_ID_LEN];
    
    printf("\n================ 按身份证号查询档案 ================\n");
    printf("提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
    
    while (1)
    {
        get_safe_string("请输入身份证号: ", id_card, MAX_ID_LEN);
        
        // 检查是否退出
        if (strcmp(id_card, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查身份证号格式是否合法
        if (validate_id_card(id_card))
        {
            break; // 格式正确，退出循环
        }
        else
        {
            printf("⚠️ 身份证号格式不合法，请重新输入！\n");
        }
    }
    
    query_patient_archive_by_id_card(id_card);
    system("pause");
}

static void handle_patient_archive_query_by_name()
{
    char name[MAX_NAME_LEN];
    
    printf("\n================ 按姓名查询档案 ================\n");
    printf("提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
    
    while (1)
    {
        get_safe_string("请输入患者姓名: ", name, MAX_NAME_LEN);
        
        // 检查是否退出
        if (strcmp(name, "00") == 0)
        {
            printf("操作取消！\n");
            system("pause");
            return;
        }
        
        // 检查姓名是否为空
        if (strlen(name) > 0)
        {
            break; // 姓名不为空，退出循环
        }
        else
        {
            printf("⚠️ 姓名不能为空，请重新输入！\n");
        }
    }
    
    query_patient_archive_by_name(name);
    system("pause");
}

static void handle_patient_archive_update()
{
    char patient_id[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    char target_dept[MAX_NAME_LEN];
    char symptom[MAX_SYMPTOM_LEN];
    char id_card[MAX_ID_LEN];
    char temp_input[MAX_SYMPTOM_LEN];
    int age = 0;
    PatientNode* patient = NULL;

    printf("\n================ 修改患者信息 ================\n");
    
    // 患者编号输入环节
    input_patient_id:
    get_safe_string("请输入患者编号(输入 0 回退上一步，输入 00 退出): ", patient_id, MAX_ID_LEN);
    if (strcmp(patient_id, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    // 患者编号校验（死循环）
    while (1)
    {
        if (strcmp(patient_id, "0") == 0)
            goto input_patient_id;
        if (strlen(patient_id) == 0)
        {
            printf("患者编号不能为空，请重新输入：");
            get_safe_string("", patient_id, MAX_ID_LEN);
            continue;
        }
        patient = find_patient_by_id(g_patient_list, patient_id);
        if (patient == NULL)
        {
            printf("未找到该患者，请重新输入：");
            get_safe_string("", patient_id, MAX_ID_LEN);
            continue;
        }
        break;
    }
    
    printf("\n提示：患者编号不可修改。\n");
    printf("提示：留空表示不修改该字段。\n\n");
    
    // 输入新姓名环节
    input_name:
    get_safe_string("请输入新姓名(输入 0 回退上一步，输入 00 退出): ", temp_input, MAX_NAME_LEN);
    if (strcmp(temp_input, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    if (strcmp(temp_input, "0") == 0)
        goto input_patient_id;
    if (strlen(temp_input) > 0)
        strcpy(name, temp_input);
    else
        strcpy(name, patient->name);
    
    // 输入新年龄环节
    input_age:
    printf("当前年龄：%d\n", patient->age);
    get_safe_string("请输入新年龄(输入 0 回退上一步，输入 00 退出): ", temp_input, MAX_SYMPTOM_LEN);
    if (strcmp(temp_input, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    if (strcmp(temp_input, "0") == 0)
        goto input_name;
    if (strlen(temp_input) > 0)
        age = atoi(temp_input);
    else
        age = patient->age;
    
    // 输入新症状环节
    input_symptom:
    printf("当前症状：%s\n", patient->symptom);
    get_safe_string("请输入新症状描述(输入 0 回退上一步，输入 00 退出): ", temp_input, MAX_SYMPTOM_LEN);
    if (strcmp(temp_input, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    if (strcmp(temp_input, "0") == 0)
        goto input_age;
    if (strlen(temp_input) > 0)
        strcpy(symptom, temp_input);
    else
        strcpy(symptom, patient->symptom);
    
    // 输入新目标科室环节
    input_dept:
    printf("当前目标科室：%s\n", patient->target_dept);
    get_safe_string("请输入新目标科室(输入 0 回退上一步，输入 00 退出): ", temp_input, MAX_NAME_LEN);
    if (strcmp(temp_input, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    if (strcmp(temp_input, "0") == 0)
        goto input_symptom;
    if (strlen(temp_input) > 0)
        strcpy(target_dept, temp_input);
    else
        strcpy(target_dept, patient->target_dept);
    
    // 输入新身份证号环节
    printf("当前身份证号：%s\n", patient->id_card);
    get_safe_string("请输入新身份证号(输入 0 回退上一步，输入 00 退出): ", temp_input, MAX_ID_LEN);
    if (strcmp(temp_input, "00") == 0)
    {
        printf("操作取消！\n");
        return;
    }
    if (strcmp(temp_input, "0") == 0)
        goto input_dept;
    // 身份证号校验（如果不为空则校验格式）
    while (1)
    {
        if (strlen(temp_input) == 0)
        {
            strcpy(id_card, patient->id_card);
            break;
        }
        if (!validate_id_card(temp_input))
        {
            printf("身份证号格式不合法，请重新输入：");
            get_safe_string("", temp_input, MAX_ID_LEN);
            if (strcmp(temp_input, "00") == 0)
            {
                printf("操作取消！\n");
                return;
            }
            if (strcmp(temp_input, "0") == 0)
                goto input_dept;
            continue;
        }
        strcpy(id_card, temp_input);
        break;
    }
    
    // 【安全管控】余额修改权限已彻底剥夺，不允许终端用户修改账户余额
    double balance = patient->balance;

    update_patient_archive(patient_id, name, age, symptom, target_dept, id_card, balance);
    system("pause");
}

static void patient_archive_menu()
{
    int running = 1;
    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               📋 患者档案管理菜单\n");
        printf("======================================================\n");
        printf("  [1] 按编号查询档案\n");
        printf("  [2] 按身份证号查询档案\n");
        printf("  [3] 按姓名查询档案\n");
        printf("  [4] 修改患者信息\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");
        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                handle_patient_archive_query_by_id();
                break;
            case 2:
                handle_patient_archive_query_by_id_card();
                break;
            case 3:
                handle_patient_archive_query_by_name();
                break;
            case 4:
                handle_patient_archive_update();
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
static void quick_register_menu()
{
    int running = 1;
    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               👨‍⚕️ 快捷挂号业务菜单\n");
        printf("======================================================\n");
        printf("  [1] 患者建档\n");
        printf("  [2] 预约登记\n");
        printf("  [3] 预约查询\n");
        printf("  [4] 预约取消\n");
        printf("  [5] 预约签到入队\n");
        printf("  [6] 患者档案管理\n");
        printf("  [7] 登记患者爽约(过号)\n");
        printf("  [8] 登记急诊逃单黑名单\n");
        printf("  [9] 查看全院黑名单\n");
        printf("  [10] 补缴欠费并核销黑名单\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");
        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                handle_internal_patient_register();
                break;
            case 2:
                handle_internal_appointment_register();
                break;
            case 3:
                handle_internal_appointment_query();
                break;
            case 4:
                handle_internal_appointment_cancel();
                break;
            case 5:
                handle_internal_appointment_check_in();
                break;
            case 6:
                patient_archive_menu();
                break;
            case 7:
            {
                char appointment_id[MAX_ID_LEN];
                get_safe_string("请输入预约编号: ", appointment_id, MAX_ID_LEN);
                mark_appointment_missed(appointment_id);
                system("pause");
                break;
            }
            case 8:
            {
                handle_emergency_escape(NULL);
                system("pause");
                break;
            }
            case 9:
            {
                // 查看全院黑名单
                system("cls");
                printf("\n======================================================\n");
                printf("                ⛔ 全院黑名单患者\n");
                printf("======================================================\n");
                
                if (g_patient_list == NULL)
                {
                    printf("⚠️ 患者链表尚未初始化！\n");
                }
                else
                {
                    PatientNode* curr = g_patient_list->next;
                    int count = 0;
                    
                    while (curr != NULL)
                    {
                        if (curr->is_blacklisted == 1 || curr->is_blacklisted == 2)
                        {
                            count++;
                            printf("【黑名单患者 %d】\n", count);
                            printf("--------------------------------------------------\n");
                            printf("患者编号: %s\n", curr->id);
                            printf("患者姓名: %s\n", curr->name);
                            printf("身份证号: %s\n", curr->id_card);
                            
                            if (curr->is_blacklisted == 1)
                            {
                                printf("黑名单类型: 普通爽约（90天）\n");
                                printf("到期时间: %s\n", ctime(&curr->blacklist_expire));
                            }
                            else if (curr->is_blacklisted == 2)
                            {
                                printf("黑名单类型: 急诊逃单（永久）\n");
                                printf("欠费金额: %.2f 元\n", curr->emergency_debt);
                            }
                            
                            printf("--------------------------------------------------\n\n");
                        }
                        curr = curr->next;
                    }
                    
                    if (count == 0)
                    {
                        printf("✅ 当前没有黑名单患者\n");
                    }
                    else
                    {
                        printf("共找到 %d 名黑名单患者\n", count);
                    }
                }
                
                printf("======================================================\n");
                system("pause");
                break;
            }
            case 10:
            {
                // 补缴欠费并核销黑名单
                char patient_id[MAX_ID_LEN];
                get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
                settle_blacklisted_debt(patient_id);
                system("pause");
                break;
            }
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
static void patient_self_service_menu()
{
    int running = 1;
    char patient_id[MAX_ID_LEN];
    char id_card[MAX_ID_LEN];
    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               🤗 患者自助服务菜单\n");
        printf("======================================================\n");
        printf("  [1] 首次来院登记\n");
        printf("  [2] 自助预约登记\n");
        printf("  [3] 自助挂号（现场号）\n");
        printf("  [4] 查询自己的预约\n");
        printf("  [5] 取消自己的预约\n");
        printf("  [6] 当前就诊总览\n");
        printf("  [7] 查询自己的历史就诊记录\n");
        printf("  [8] 查询检查报告\n");
        printf("  [9] 自助账单缴费\n");
        printf("  [10] 发起服务投诉\n");
        printf("  [11] 查看投诉进度\n");
        printf("  [12] 就诊满意度评价\n");
        printf("  [13] 补缴黑名单欠费\n");
        printf("  [14] 查询账户余额\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");
        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                handle_patient_self_first_visit();
                break;
            case 2:
                handle_patient_self_appointment_register();
                break;
            case 3:
                handle_patient_self_registration();
                break;
            case 4:
                printf("\n提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
                
                while (1)
                {
                    get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                    
                    if (strcmp(patient_id, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    if (validate_patient_id(patient_id))
                    {
                        break;
                    }
                    else
                    {
                        printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                    }
                }
                
                if (strcmp(patient_id, "00") == 0) break;
                
                while (1)
                {
                    get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
                    
                    if (strcmp(id_card, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    if (strcmp(id_card, "0") == 0)
                    {
                        while (1)
                        {
                            get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                            
                            if (strcmp(patient_id, "00") == 0)
                            {
                                printf("操作取消！\n");
                                system("pause");
                                break;
                            }
                            
                            if (validate_patient_id(patient_id))
                            {
                                break;
                            }
                            else
                            {
                                printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                            }
                        }
                        
                        if (strcmp(patient_id, "00") == 0) break;
                        continue;
                    }
                    
                    if (validate_id_card(id_card))
                    {
                        break;
                    }
                    else
                    {
                        printf("⚠️ 身份证号格式不合法，请重新输入！\n");
                    }
                }
                
                if (strcmp(id_card, "00") == 0) break;
                
                PatientNode* patient = find_patient_by_id_card(id_card);
                if (patient == NULL || strcmp(patient->id, patient_id) != 0)
                {
                    printf("\n❌ 身份核验失败！患者编号与身份证号不匹配\n");
                    system("pause");
                    break;
                }
                
                printf("\n身份核验成功！欢迎，%s\n", patient->name);
                query_appointments_by_patient_id(patient_id);
                system("pause");
                break;
            case 5:
                handle_patient_self_appointment_cancel();
                system("pause");
                break;
            case 6:
                handle_patient_self_current_visit_overview();
                system("pause");
                break;
            case 7:
                printf("\n提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
                
                while (1)
                {
                    get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                    
                    if (strcmp(patient_id, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    if (validate_patient_id(patient_id))
                    {
                        break;
                    }
                    else
                    {
                        printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                    }
                }
                
                if (strcmp(patient_id, "00") == 0) break;
                
                while (1)
                {
                    get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
                    
                    if (strcmp(id_card, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    if (strcmp(id_card, "0") == 0)
                    {
                        while (1)
                        {
                            get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                            
                            if (strcmp(patient_id, "00") == 0)
                            {
                                printf("操作取消！\n");
                                system("pause");
                                break;
                            }
                            
                            if (validate_patient_id(patient_id))
                            {
                                break;
                            }
                            else
                            {
                                printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                            }
                        }
                        
                        if (strcmp(patient_id, "00") == 0) break;
                        continue;
                    }
                    
                    if (validate_id_card(id_card))
                    {
                        break;
                    }
                    else
                    {
                        printf("⚠️ 身份证号格式不合法，请重新输入！\n");
                    }
                }
                
                if (strcmp(id_card, "00") == 0) break;
                
                PatientNode* history_patient = find_patient_by_id_card(id_card);
                if (history_patient == NULL || strcmp(history_patient->id, patient_id) != 0)
                {
                    printf("\n❌ 身份核验失败！患者编号与身份证号不匹配\n");
                    system("pause");
                    break;
                }
                
                printf("\n身份核验成功！欢迎，%s\n", history_patient->name);
                handle_patient_self_consult_history_query(patient_id, id_card);
                system("pause");
                break;
            case 8:
                printf("\n提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
                
                while (1)
                {
                    get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                    
                    if (strcmp(patient_id, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    if (validate_patient_id(patient_id))
                    {
                        break;
                    }
                    else
                    {
                        printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                    }
                }
                
                if (strcmp(patient_id, "00") == 0) break;
                
                while (1)
                {
                    get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
                    
                    if (strcmp(id_card, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    if (strcmp(id_card, "0") == 0)
                    {
                        while (1)
                        {
                            get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                            
                            if (strcmp(patient_id, "00") == 0)
                            {
                                printf("操作取消！\n");
                                system("pause");
                                break;
                            }
                            
                            if (validate_patient_id(patient_id))
                            {
                                break;
                            }
                            else
                            {
                                printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                            }
                        }
                        
                        if (strcmp(patient_id, "00") == 0) break;
                        continue;
                    }
                    
                    if (validate_id_card(id_card))
                    {
                        break;
                    }
                    else
                    {
                        printf("⚠️ 身份证号格式不合法，请重新输入！\n");
                    }
                }
                
                if (strcmp(id_card, "00") == 0) break;
                
                PatientNode* check_patient = find_patient_by_id_card(id_card);
                if (check_patient == NULL || strcmp(check_patient->id, patient_id) != 0)
                {
                    printf("\n❌ 身份核验失败！患者编号与身份证号不匹配\n");
                    system("pause");
                    break;
                }
                
                printf("\n身份核验成功！欢迎，%s\n", check_patient->name);
                printf("\n================ 检查报告列表 ================\n");
                
                int check_count = 0;
                CheckRecordNode* curr = g_check_record_list->next;
                while (curr != NULL)
                {
                    if (strcmp(curr->patient_id, patient_id) == 0)
                    {
                        check_count++;
                        printf("\n【检查报告 %d】\n", check_count);
                        printf("检查项目：%s\n", curr->item_name);
                        printf("检查科室：%s\n", curr->dept);
                        printf("检查时间：%s\n", curr->check_time[0] != '\0' ? curr->check_time : "待安排");
                        printf("检查状态：%s\n", curr->is_completed ? "已完成" : "待检查");
                        printf("检查结果：%s\n", curr->result);
                        printf("----------------------------------------\n");
                    }
                    curr = curr->next;
                }
                
                if (check_count == 0)
                {
                    printf("暂无检查记录\n");
                }
                system("pause");
                break;
            case 9:
                printf("\n提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
                
                while (1)
                {
                    get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                    
                    if (strcmp(patient_id, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    if (validate_patient_id(patient_id))
                    {
                        break;
                    }
                    else
                    {
                        printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                    }
                }
                
                if (strcmp(patient_id, "00") == 0) break;
                
                while (1)
                {
                    get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
                    
                    if (strcmp(id_card, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    if (strcmp(id_card, "0") == 0)
                    {
                        while (1)
                        {
                            get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                            
                            if (strcmp(patient_id, "00") == 0)
                            {
                                printf("操作取消！\n");
                                system("pause");
                                break;
                            }
                            
                            if (validate_patient_id(patient_id))
                            {
                                break;
                            }
                            else
                            {
                                printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                            }
                        }
                        
                        if (strcmp(patient_id, "00") == 0) break;
                        continue;
                    }
                    
                    if (validate_id_card(id_card))
                    {
                        break;
                    }
                    else
                    {
                        printf("⚠️ 身份证号格式不合法，请重新输入！\n");
                    }
                }
                
                if (strcmp(id_card, "00") == 0) break;
                
                PatientNode* payment_patient = find_patient_by_id_card(id_card);
                if (payment_patient == NULL || strcmp(payment_patient->id, patient_id) != 0)
                {
                    printf("\n❌ 身份核验失败！患者编号与身份证号不匹配\n");
                    system("pause");
                    break;
                }
                
                printf("\n身份核验成功！欢迎，%s\n", payment_patient->name);
                process_patient_payment(patient_id);
                system("pause");
                break;
            case 10:
                printf("\n提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
                
                // 患者编号输入和验证
                while (1)
                {
                    get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                    
                    // 检查是否退出
                    if (strcmp(patient_id, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    // 检查患者编号格式是否合法
                    if (validate_patient_id(patient_id))
                    {
                        break; // 格式正确，退出循环
                    }
                    else
                    {
                        printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                    }
                }
                
                if (strcmp(patient_id, "00") == 0) break;
                
                // 身份证号输入和验证
                while (1)
                {
                    get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
                    
                    // 检查是否退出
                    if (strcmp(id_card, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    // 检查是否回退
                    if (strcmp(id_card, "0") == 0)
                    {
                        // 重新输入患者编号
                        while (1)
                        {
                            get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                            
                            // 检查是否退出
                            if (strcmp(patient_id, "00") == 0)
                            {
                                printf("操作取消！\n");
                                system("pause");
                                break;
                            }
                            
                            // 检查患者编号格式是否合法
                            if (validate_patient_id(patient_id))
                            {
                                break; // 格式正确，退出循环
                            }
                            else
                            {
                                printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                            }
                        }
                        
                        if (strcmp(patient_id, "00") == 0) break;
                        continue; // 回到身份证号输入循环
                    }
                    
                    // 检查身份证号格式是否合法
                    if (validate_id_card(id_card))
                    {
                        break; // 格式正确，退出循环
                    }
                    else
                    {
                        printf("⚠️ 身份证号格式不合法，请重新输入！\n");
                    }
                }
                
                if (strcmp(id_card, "00") == 0) break;
                
                // 身份核验
                PatientNode* complaint_patient = find_patient_by_id_card(id_card);
                if (complaint_patient == NULL || strcmp(complaint_patient->id, patient_id) != 0)
                {
                    printf("\n❌ 身份核验失败！患者编号与身份证号不匹配\n");
                    system("pause");
                    break;
                }
                
                printf("\n身份核验成功！欢迎，%s\n", complaint_patient->name);
                submit_new_complaint(patient_id);
                system("pause");
                break;
            case 11:
                printf("\n提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
                
                while (1)
                {
                    get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                    
                    if (strcmp(patient_id, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    if (validate_patient_id(patient_id))
                    {
                        break;
                    }
                    else
                    {
                        printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                    }
                }
                
                if (strcmp(patient_id, "00") == 0) break;
                
                while (1)
                {
                    get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
                    
                    if (strcmp(id_card, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    if (strcmp(id_card, "0") == 0)
                    {
                        while (1)
                        {
                            get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                            
                            if (strcmp(patient_id, "00") == 0)
                            {
                                printf("操作取消！\n");
                                system("pause");
                                break;
                            }
                            
                            if (validate_patient_id(patient_id))
                            {
                                break;
                            }
                            else
                            {
                                printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                            }
                        }
                        
                        if (strcmp(patient_id, "00") == 0) break;
                        continue;
                    }
                    
                    if (validate_id_card(id_card))
                    {
                        break;
                    }
                    else
                    {
                        printf("⚠️ 身份证号格式不合法，请重新输入！\n");
                    }
                }
                
                if (strcmp(id_card, "00") == 0) break;
                
                PatientNode* progress_patient = find_patient_by_id_card(id_card);
                if (progress_patient == NULL || strcmp(progress_patient->id, patient_id) != 0)
                {
                    printf("\n❌ 身份核验失败！患者编号与身份证号不匹配\n");
                    system("pause");
                    break;
                }
                
                printf("\n身份核验成功！欢迎，%s\n", progress_patient->name);
                query_patient_complaints(patient_id);
                system("pause");
                break;
            case 12:
                printf("\n提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
                
                // 患者编号输入和验证
                while (1)
                {
                    get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                    
                    // 检查是否退出
                    if (strcmp(patient_id, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    // 检查患者编号格式是否合法
                    if (validate_patient_id(patient_id))
                    {
                        break; // 格式正确，退出循环
                    }
                    else
                    {
                        printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                    }
                }
                
                if (strcmp(patient_id, "00") == 0) break;
                
                // 身份证号输入和验证
                while (1)
                {
                    get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
                    
                    // 检查是否退出
                    if (strcmp(id_card, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    // 检查是否回退
                    if (strcmp(id_card, "0") == 0)
                    {
                        // 重新输入患者编号
                        while (1)
                        {
                            get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                            
                            // 检查是否退出
                            if (strcmp(patient_id, "00") == 0)
                            {
                                printf("操作取消！\n");
                                system("pause");
                                break;
                            }
                            
                            // 检查患者编号格式是否合法
                            if (validate_patient_id(patient_id))
                            {
                                break; // 格式正确，退出循环
                            }
                            else
                            {
                                printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                            }
                        }
                        
                        if (strcmp(patient_id, "00") == 0) break;
                        continue; // 回到身份证号输入循环
                    }
                    
                    // 检查身份证号格式是否合法
                    if (validate_id_card(id_card))
                    {
                        break; // 格式正确，退出循环
                    }
                    else
                    {
                        printf("⚠️ 身份证号格式不合法，请重新输入！\n");
                    }
                }
                
                if (strcmp(id_card, "00") == 0) break;
                
                // 身份核验
                PatientNode* eval_patient = find_patient_by_id_card(id_card);
                if (eval_patient == NULL || strcmp(eval_patient->id, patient_id) != 0)
                {
                    printf("\n❌ 身份核验失败！患者编号与身份证号不匹配\n");
                    system("pause");
                    break;
                }
                
                printf("\n身份核验成功！欢迎，%s\n", eval_patient->name);
                submit_patient_evaluation(patient_id);
                system("pause");
                break;
            case 13:
                // 补缴黑名单欠费
                printf("\n提示：输入 '0' 可以回退上一步，输入 '00' 可以退出操作\n");
                
                while (1)
                {
                    get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                    
                    if (strcmp(patient_id, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    if (validate_patient_id(patient_id))
                    {
                        break;
                    }
                    else
                    {
                        printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                    }
                }
                
                if (strcmp(patient_id, "00") == 0) break;
                
                while (1)
                {
                    get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
                    
                    if (strcmp(id_card, "00") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    if (strcmp(id_card, "0") == 0)
                    {
                        while (1)
                        {
                            get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                            
                            if (strcmp(patient_id, "00") == 0)
                            {
                                printf("操作取消！\n");
                                system("pause");
                                break;
                            }
                            
                            if (validate_patient_id(patient_id))
                            {
                                break;
                            }
                            else
                            {
                                printf("⚠️ 患者编号格式不合法，正确格式为 P-1001，请重新输入！\n");
                            }
                        }
                        
                        if (strcmp(patient_id, "00") == 0) break;
                        continue;
                    }
                    
                    if (validate_id_card(id_card))
                    {
                        break;
                    }
                    else
                    {
                        printf("⚠️ 身份证号格式不合法，请重新输入！\n");
                    }
                }
                
                if (strcmp(id_card, "00") == 0) break;
                
                PatientNode* debt_patient = find_patient_by_id_card(id_card);
                if (debt_patient == NULL || strcmp(debt_patient->id, patient_id) != 0)
                {
                    printf("\n❌ 身份核验失败！患者编号与身份证号不匹配\n");
                    system("pause");
                    break;
                }
                
                printf("\n身份核验成功！欢迎，%s\n", debt_patient->name);
                
                // 检查患者是否在黑名单中
                if (debt_patient->is_blacklisted != 2)
                {
                    printf("\n⚠️ 您不在黑名单中，无需补缴欠费！\n");
                    system("pause");
                    break;
                }
                
                // 显示患者欠费信息
                printf("\n================ 欠费信息 ================\n");
                printf("患者编号: %s\n", debt_patient->id);
                printf("患者姓名: %s\n", debt_patient->name);
                printf("欠费金额: %.2f 元\n", debt_patient->emergency_debt);
                printf("==========================================\n");
                
                // 输入缴费金额
                double payment_amount;
                char input[50];
                while (1)
                {
                    get_safe_string("请输入您要缴纳的金额（元）: ", input, 50);
                    
                    // 检查是否取消
                    if (strcmp(input, "0") == 0)
                    {
                        printf("操作取消！\n");
                        system("pause");
                        break;
                    }
                    
                    // 转换为数字
                    if (sscanf(input, "%lf", &payment_amount) == 1 && payment_amount >= 0)
                    {
                        break;
                    }
                    else
                    {
                        printf("⚠️ 请输入有效的金额（数字）！\n");
                    }
                }
                
                if (strcmp(input, "0") == 0) break;
                
                // 比对缴费金额与欠费金额
                if (payment_amount < debt_patient->emergency_debt)
                {
                    printf("\n⚠️ 实际缴费金额（%.2f 元）小于欠费金额（%.2f 元），缴费失败！\n", payment_amount, debt_patient->emergency_debt);
                    printf("请缴纳足额欠款后再来办理！\n");
                    system("pause");
                    break;
                }
                
                // 执行缴费操作
                debt_patient->emergency_debt = 0.0; // 清零欠费
                debt_patient->is_blacklisted = 0; // 移出黑名单
                debt_patient->status = STATUS_COMPLETED; // 重置状态为就诊结束
                
                // 打印成功提示
                printf("\n✅ 缴费成功！您已移出黑名单，恢复正常就医权限。\n");
                printf("实际缴费金额: %.2f 元\n", payment_amount);
                printf("==========================================\n");
                
                system("pause");
                break;
            case 14:
                handle_patient_self_balance_query();
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
int main() 
{
    // ---------------------------------------------------
    // 第一阶段：系统初始化 (加载数据)
    // ---------------------------------------------------------
    printf("======================================================\n");
    printf("     🚀 正在启动智慧医疗信息管理系统 (HIS) ...\n");
    printf("======================================================\n");
    
    // 这里未来调用 data_io.c 里的函数，把 txt 数据全部读进双向链表
    // printf("-> 正在加载患者数据...\n");
    // init_patient_list(); 
    // printf("-> 正在加载医生与科室数据...\n");
    // init_doctor_list();
    // printf("-> 正在加载药品库存数据...\n");
    // init_medicine_list();
    
    printf("\n✅ 系统初始化完成，数据加载成功！\n");
    // 模拟一个加载停顿，让控制台看起来很高级 (Windows下用Sleep, Linux下用sleep)
    // Sleep(1000); 

    // ---------------------------------------------------------
    // 第二阶段：主事件循环 (防崩溃核心)
    // ---------------------------------------------------------
    printf("⚙️ 正在点火启动底层双向链表引擎...\n"
);
    
    // 1. 初始化所有链表头节点
    g_patient_list = init_patient_list();
    g_appointment_list = init_appointment_list();
    g_doctor_list = init_doctor_list();
    g_medicine_list = init_medicine_list();
    g_ward_list = init_ward_list();
    g_account_list = init_account_list();
    g_consult_record_list = init_consult_record_list();
    g_check_item_list = init_check_item_list();
    g_check_record_list = init_check_record_list();
    g_alert_list = init_alert_list();
    g_complaint_list = init_complaint_list();
    g_inpatient_list = create_inpatient_record_head();

    // 加载存档数据
    load_all_data();

    // ==============================================
    // 仅当没有读取到管理员账号时，才注入系统初始数据
    // ==============================================
    if (g_account_list->next == NULL)
    {
        printf("⚠️ 首次运行系统，正在注入基础测试数据...\n");
        
        // ==============================================
        // 检查项目字典初始化
        // ==============================================
        // 放射科检查项目
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-001", "X光检查", "放射科", 150.0, MEDICARE_CLASS_A));
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-002", "CT扫描", "放射科", 800.0, MEDICARE_CLASS_B));
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-003", "增强CT", "放射科", 1200.0, MEDICARE_CLASS_B));
        
        // 影像科检查项目
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-004", "MRI磁共振", "影像科", 1200.0, MEDICARE_CLASS_B));
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-005", "PET-CT", "影像科", 8000.0, MEDICARE_NONE));
        
        // 超声科检查项目
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-006", "腹部B超", "超声科", 200.0, MEDICARE_CLASS_B));
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-007", "心脏彩超", "超声科", 350.0, MEDICARE_CLASS_B));
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-008", "妇科B超", "超声科", 180.0, MEDICARE_CLASS_B));
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-009", "甲状腺B超", "超声科", 150.0, MEDICARE_CLASS_B));
        
        // 检验科检查项目
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-010", "血常规", "检验科", 50.0, MEDICARE_CLASS_A));
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-011", "生化全项", "检验科", 300.0, MEDICARE_CLASS_A));
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-012", "肝功能", "检验科", 120.0, MEDICARE_CLASS_A));
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-013", "肾功能", "检验科", 100.0, MEDICARE_CLASS_A));
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-014", "血糖检测", "检验科", 30.0, MEDICARE_CLASS_A));
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-015", "血脂四项", "检验科", 80.0, MEDICARE_CLASS_A));
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-016", "凝血功能", "检验科", 150.0, MEDICARE_CLASS_A));
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-017", "尿常规", "检验科", 30.0, MEDICARE_CLASS_A));
        
        // 心电图室检查项目
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-018", "心电图", "心电图室", 80.0, MEDICARE_CLASS_A));
        insert_check_item_tail(g_check_item_list, create_check_item_node("C-019", "动态心电图", "心电图室", 300.0, MEDICARE_CLASS_A));

        // 2. 注入一组极端测试数据
        insert_patient_tail(g_patient_list, create_patient_node(
"P-001", "张三", 19, "男", "110101199001011234"
));

        
        insert_medicine_tail(g_medicine_list, create_medicine_node(
"M-001", "阿莫西林", "阿莫", "阿莫西林胶囊", 15.5, 100, MEDICARE_CLASS_A, "2027-04-01"));
        insert_appointment_tail(g_appointment_list, create_appointment_node(
"A-001", "P-001", "2026-04-01", "上午", "D-201", "外科", RESERVED
));
        insert_ward_tail(g_ward_list, create_ward_node(
"W-101", "B-101", WARD_TYPE_GENERAL, "内科"
));
        insert_account_tail(g_account_list, create_account_node(
"admin", "123456", "超级管理员", "男", ROLE_ADMIN));

        insert_account_tail(g_account_list, create_account_node(
"nurse", "123456", "护士", "女", ROLE_NURSE));

        insert_account_tail(g_account_list, create_account_node(
"pharm", "123456", "药剂师", "女", ROLE_PHARMACIST));







        // 3. 打印核验
        printf("✅ 引擎全部就绪！\n"
);
        printf(" -> 测试患者: %s\n"
, g_patient_list->next->name);
        printf(" -> 测试药品: %s (库存:%d)\n"
, g_medicine_list->next->name, g_medicine_list->next->stock);
        printf(" -> 测试预约: %s (%s %s)\n"
, g_appointment_list->next->appointment_id, g_appointment_list->next->appointment_date, g_appointment_list->next->appointment_slot);
        printf(" -> 测试床位: %s\n"
, g_ward_list->next->bed_id);
        printf(" -> 测试超管: %s (权限级别:%d)\n\n"
, g_account_list->next->real_name, g_account_list->next->role);
// 测试查找引擎
        PatientNode* search_result = find_patient_by_id(g_patient_list, "P-001");
        if (search_result != NULL) {
            printf("🔍 雷达搜索成功！找到目标：患者姓名 [%s]，当前余额 [%.2f]\n\n", search_result->name, search_result->balance);
        } else {
            printf("❌ 雷达搜索失败，查无此人！\n\n");
        }

        // ==========================================
        // 1. 定义真实数据字典 (百家姓与科室关联)
        // ==========================================
        const char* surnames[] = {"王", "李", "张", "刘", "陈", "杨", "黄", "赵", "吴", "周", "徐", "孙", "马", "朱", "胡", "郭", "何", "高", "林", "罗" };
        const char* given_names[] = {
            "伟", "芳", "娜", "敏", "静", "强", "磊", "军", "洋", "勇",
            "建国", "欣怡", "浩然", "雨桐", "子轩", "梓涵", "诗涵", "雨泽", "宇轩", "雨晨",
            "佳琪", "子豪", "雨欣", "一诺", "艺馨", "思远", "雨桐", "语桐", "雨菲", "雨萌"
        };
        
        // 提取所有科室名称（来自 D-001 到 D-027 医生）
        const char* all_depts[] = {
            "内科", "外科", "妇产科", "儿科", "骨科", "心血管内科", "呼吸内科", "消化内科",
            "神经内科", "内分泌科", "肾内科", "泌尿外科", "肿瘤科", "急诊科", "眼科", "耳鼻喉科",
            "口腔科", "皮肤科", "风湿免疫科", "感染科", "精神科", "康复医学科", "普通外科",
            "肝胆外科", "心胸外科", "神经外科", "全科", "放射科", "影像科", "检验科",
            "超声科", "心电图室"
        };
        int num_depts = sizeof(all_depts) / sizeof(all_depts[0]);
        
        char temp_id[32], temp_name[128], temp_room[64];
        int  i;
        
        // ==========================================
        // 2. 生成 120 名医生 (按科室分组，制造重名，覆盖所有科室)
        // ==========================================
        int doctor_count = 0;
        int doc_id = 1;
        
        // 先生成前两名重名医生（李建国），分配到前两个科室
        for (int j = 0; j < 2 && doctor_count < 120; j++) {
            sprintf(temp_id, "D-2%02d", doc_id++);
            sprintf(temp_name, "李建国");
            char* dept = (char*)all_depts[j % num_depts];
            char* temp_gender = (j % 2 == 0) ? "男" : "女"; // 交替性别
            insert_doctor_tail(g_doctor_list, create_doctor_node(temp_id, temp_name, temp_gender, dept));
            // 为医生账号添加科室信息
            char doctor_full_name[128];
            sprintf(doctor_full_name, "%s-%s", temp_name, dept);
            insert_account_tail(g_account_list, create_account_node(temp_id, "123456", doctor_full_name, temp_gender, ROLE_DOCTOR));
            doctor_count++;
        }
        
        // 按科室分组生成剩余医生，确保每个科室至少有3名医生
        for (int dept_idx = 0; dept_idx < num_depts; dept_idx++) {
            char* dept = (char*)all_depts[dept_idx];
            // 计算每个科室需要生成的医生数量
            // 前两个科室已经有1名医生（李建国），所以需要生成2名
            // 其他科室需要生成3名
            int doctors_needed = 3;
            if (dept_idx < 2) {
                doctors_needed = 2;
            }
            // 如果还有剩余名额，每个科室再增加1名医生
            if (doctor_count + num_depts * 1 <= 120) {
                doctors_needed++;
            }
            // 生成医生
            for (int j = 0; j < doctors_needed && doctor_count < 120; j++) {
                sprintf(temp_id, "D-2%02d", doc_id++);
                // 生成更多样化的名字
                int surname_idx = (doctor_count * 7) % (sizeof(surnames)/sizeof(surnames[0]));
                int given_name_idx = (doctor_count * 11) % (sizeof(given_names)/sizeof(given_names[0]));
                sprintf(temp_name, "%s%s", surnames[surname_idx], given_names[given_name_idx]);
                char* temp_gender = (doctor_count % 2 == 0) ? "男" : "女";
                insert_doctor_tail(g_doctor_list, create_doctor_node(temp_id, temp_name, temp_gender, dept));
                // 为医生账号添加科室信息
                char doctor_full_name[128];
                sprintf(doctor_full_name, "%s-%s", temp_name, dept);
                insert_account_tail(g_account_list, create_account_node(temp_id, "123456", doctor_full_name, temp_gender, ROLE_DOCTOR));
                doctor_count++;
            }
        }
        
        // ==========================================
        // 3. 生成 30 种药品 (使用真实药品字典)
        // ==========================================
        // ==========================================
        // 高质量真实药品字典 (30种)
        // ==========================================
        const char* real_medicines[30][8] = {
            // 编号, 商品名, 别名, 通用名/说明(含科室相关性), 单价, 库存, 医保类型, 有效期
            {
"M-002", "小儿氨酚黄那敏颗粒", "儿童退烧药", "[儿科专属]缓解儿童普通感冒及流行性感冒引起的重度发热", "22.50", "4", "1", "2026-10-01"
},
            {
"M-003", "硝苯地平缓释片(II)", "降压缓释片", "[心血管内科专属]高血压及心绞痛长效靶向控制缓释制剂", "35.80", "800", "1", "2026-05-01"
},
            {
"M-004", "奥美拉唑肠溶胶囊", "护胃特效药", "[消化内科专属]胃溃疡及十二指肠溃疡重度粘膜修复特效药", "45.00", "600", "1", "2026-12-31"
},
            {
"M-005", "布地奈德福莫特罗粉", "哮喘吸入剂", "[呼吸内科专属]支气管哮喘及重度慢性阻塞性肺疾病控制剂", "128.50", "200", "2", "2027-08-20"
},
            {
"M-006", "塞来昔布胶囊", "骨科止痛药", "[骨科专属]骨关节炎及类风湿关节炎急性期抗炎镇痛靶向药", "55.00", "400", "2", "2026-09-09"
},
            {
"M-007", "盐酸二甲双胍片", "降糖基础药", "[内分泌科专属]2型糖尿病一线首选控制血糖药物", "18.50", "1000", "1", "2028-01-01"
},
            {
"M-008", "甲钴胺片", "营养神经药", "[神经内科专属]周围神经病变及维生素B12缺乏症修复药", "32.00", "500", "1", "2027-03-12"
},
            {
"M-009", "炉甘石洗剂", "止痒水", "[皮肤科专属]急性瘙痒性皮肤病及荨麻疹外用收敛止痒药", "9.90", "300", "1", "2026-06-30"
},
            {
"M-010", "玻璃酸钠滴眼液", "人工泪液", "[眼科专属]干眼症及角膜上皮机械性损伤修复润滑剂", "28.50", "450", "2", "2026-11-11"
},
            {
"M-011", "黄体酮胶囊", "保胎药", "[妇产科专属]先兆流产及黄体功能不足补充治疗药", "42.00", "250", "2", "2027-02-28"
},
            {
"M-012", "阿卡波糖片", "拜唐苹", "配合饮食控制用于2型糖尿病降低餐后血糖", "65.00", "600", "1", "2027-07-01"
},
            {
"M-013", "瑞舒伐他汀钙片", "可定", "原发性高胆固醇血症及混合型脂质异常血症靶向药", "48.50", "400", "1", "2028-05-20"
},
            {
"M-014", "多潘立酮片", "吗丁啉", "缓解由胃排空延缓引起的消化不良和恶心呕吐症状", "25.00", "800", "1", "2027-09-15"
},
            {
"M-015", "盐酸氨溴索口服溶液", "沐舒坦", "适用于伴有痰液分泌不正常及排痰功能不良的急性呼吸道疾病", "38.00", "500", "1", "2026-12-01"
},
            {
"M-016", "双氯芬酸钠缓释胶囊", "扶他林", "缓解各种急慢性关节炎症及软组织风湿病引起的疼痛", "29.80", "350", "1", "2027-04-30"
},
            {
"M-017", "普瑞巴林胶囊", "乐瑞卡", "用于治疗带状疱疹后神经痛及纤维肌痛综合征", "88.00", "150", "2", "2027-10-10"
},
            {
"M-018", "莫匹罗星软膏", "百多邦", "革兰阳性球菌引起的皮肤感染如脓疱病及毛囊炎", "22.00", "400", "1", "2026-08-15"
},
            {
"M-019", "妥布霉素滴眼液", "托百士", "敏感细菌引起的眼部及眼附属器局部感染", "35.50", "200", "1", "2027-01-20"
},
            {
"M-020", "头孢克肟分散片", "世福素", "对链球菌等敏感菌引起的急性呼吸道和泌尿道感染有效", "45.00", "600", "1", "2028-02-28"
},
            {
"M-021", "阿奇霉素肠溶片", "希舒美", "化脓性链球菌引起的急性咽炎及敏感细菌引起的下呼吸道感染", "52.00", "500", "1", "2027-06-18"
},
            {
"M-022", "左氧氟沙星片", "可乐必妥", "成年人由敏感细菌引起的轻至中度感染如急性细菌性鼻窦炎", "40.00", "450", "1", "2028-03-15"
},
            {
"M-023", "布洛芬缓释胶囊", "芬必得", "缓解轻至中度疼痛如头痛关节痛偏头痛牙痛肌肉痛神经痛", "18.00", "1200", "1", "2027-11-11"
},
            {
"M-024", "对乙酰氨基酚片", "泰诺林", "普通感冒或流行性感冒引起的发热及轻至中度疼痛", "15.00", "1500", "1", "2028-08-08"
},
            {
"M-025", "氯雷他定片", "开瑞坦", "缓解过敏性鼻炎有关的症状如打喷嚏流鼻涕鼻痒鼻塞", "28.00", "800", "1", "2027-05-05"
},
            {
"M-026", "碳酸钙D3片", "钙尔奇", "用于妊娠和哺乳期妇女更年期妇女老年人等的钙补充剂", "36.00", "600", "1", "2028-10-01"
},
            {
"M-027", "铝碳酸镁咀嚼片", "达喜", "急慢性胃炎反流性食管炎及与胃酸有关的胃部不适", "32.50", "550", "1", "2027-09-09"
},
            {
"M-028", "孟鲁司特钠咀嚼片", "顺尔宁", "适用于成人和儿童哮喘的预防和长期治疗", "68.00", "300", "2", "2027-12-20"
},
            {
"M-029", "美托洛尔缓释片", "倍他乐克", "高血压及心绞痛伴有左心室收缩功能异常的症状稳定的慢性心力衰竭", "45.80", "400", "1", "2028-04-10"
},
            {
"M-030", "维生素C泡腾片", "力度伸", "增强机体抵抗力用于预防和治疗各种急慢性传染性疾病", "25.00", "1000", "1", "2027-08-08"
},
            {
"M-031", "复方甘草口服溶液", "甘草合剂", "用于一般性及化痰性咳嗽的对症治疗", "12.50", "800", "1", "2026-10-30"
}
        };

        for (i = 0; i < 30; i++) {
            insert_medicine_tail(g_medicine_list, create_medicine_node(
                (char*)real_medicines[i][0], (char*)real_medicines[i][1], (char*)real_medicines[i][2],
                (char*)real_medicines[i][3], atof(real_medicines[i][4]), atoi(real_medicines[i][5]),
                atoi(real_medicines[i][6]), (char*)real_medicines[i][7]
            ));
        }
        
        // ==========================================
        // 4. 生成 50 张病床 (按科室分组，体现病房与科室关联，3种病房)
        // ==========================================
        i = 1;
        for (int dept_idx = 0; dept_idx < num_depts && i <= 50; dept_idx++) {
            char* dept = (char*)all_depts[dept_idx];
            // 每个科室分配2-3张病床
            for (int j = 0; j < 3 && i <= 50; j++) {
                sprintf(temp_id, "B-1%02d", i);
                sprintf(temp_room, "%s病区-%02d房", dept, (j/4)+1); // 科室关联病房
                int ward_type = (i % 4) + 1; // 1=普通, 2=ICU, 3=隔离病房, 4=单人病房
                insert_ward_tail(g_ward_list, create_ward_node(temp_room, temp_id, ward_type, dept));
                i++;
            }
        }
        
        // ==========================================
        // 5. 生成 110 名患者及 35 名住院关联 (按ID排序，制造重名)
        // ==========================================
        // 先生成所有患者
        for (i = 1; i <= 150; i++) {
            sprintf(temp_id, "P-1%03d", i);
            // 只保留两个重名测试
            if (i == 1 || i == 2) sprintf(temp_name, "张伟");
            else {
                // 生成更多样化的名字
                int surname_idx = (i * 13) % (sizeof(surnames)/sizeof(surnames[0]));
                int given_name_idx = (i * 17) % (sizeof(given_names)/sizeof(given_names[0]));
                sprintf(temp_name, "%s%s", surnames[surname_idx], given_names[given_name_idx]);
            }
            
            char temp_id_card[32];
            sprintf(temp_id_card, "1101051990%02d%02d123%d", (i%12)+1, (i%28)+1, i%10);
            char* temp_gender = (i % 2 == 0) ? "男" : "女";
            
            PatientNode* p = create_patient_node(temp_id, temp_name, 20 + (i%50), temp_gender, temp_id_card);
            if (p != NULL) {
                p->balance = 10000.0; // 统一充值1万元方便测试
                
                // 为前3个患者设置黑名单状态，用于测试安全预警
                if (i == 1) {
                    // 患者1：永久黑名单（急诊逃单）
                    p->is_blacklisted = 2;
                    strcpy(p->name, "李逃单");
                } else if (i == 2) {
                    // 患者2：临时黑名单（爽约3次）
                    p->is_blacklisted = 1;
                    p->missed_count = 3;
                    p->missed_time_1 = time(NULL) - 86400 * 10; // 10天前
                    p->missed_time_2 = time(NULL) - 86400 * 20; // 20天前
                    p->missed_time_3 = time(NULL) - 86400 * 5;  // 5天前
                    p->blacklist_expire = time(NULL) + 86400 * 80; // 还有80天过期
                    strcpy(p->name, "王爽约");
                } else if (i == 3) {
                    // 患者3：即将解禁的黑名单患者（用于测试急诊放行预警）
                    p->is_blacklisted = 1;
                    p->missed_count = 3;
                    p->missed_time_1 = time(NULL) - 86400 * 85; // 85天前
                    p->missed_time_2 = time(NULL) - 86400 * 86; // 86天前
                    p->missed_time_3 = time(NULL) - 86400 * 87; // 87天前
                    p->blacklist_expire = time(NULL) + 86400 * 5; // 还有5天过期
                    strcpy(p->name, "张即将解禁");
                }
                
                insert_patient_tail(g_patient_list, p);
            }
        }
        
        // 再为前 35 名患者分配病床 (满足 >=30 住院患者要求)
        for (i = 1; i <= 40; i++) {
            char temp_patient_id[32];
            sprintf(temp_patient_id, "P-1%03d", i);
            char bed_id[32];
            sprintf(bed_id, "B-1%02d", i);
            char inpatient_id[32];
            sprintf(inpatient_id, "IP-%04d", i);
            // 创建住院记录 (关联患者与床位)
            double deposit = 5000.0;
            if (i == 1) {
                deposit = 15.50;  // 押金不足，触发预警
            } else if (i == 2) {
                deposit = -120.00;  // 已经欠费，触发预警
            }
            InpatientRecord* in_record = create_inpatient_record_node(
                inpatient_id, temp_patient_id, bed_id, (i%4)+1, (i%4)+1, 10, 0, deposit, 1);
            insert_inpatient_record_tail(g_inpatient_list, in_record);
            
            // 更新床位状态为已占用
            WardNode* bed = find_bed_by_id(bed_id);
            if (bed != NULL) {
                bed->is_occupied = 1;
                strcpy(bed->patient_id, temp_patient_id);
            }
        }
        // ==========================================
        // 6. 生成历史业务数据 (预约、病历、检查、投诉)
        // ==========================================
        // 评价字典 (星级、评价内容)
        const char* feedback_dict[][2] = {
            {"5", "医生非常耐心，医术高超！"},
            {"5", "医生讲解详细，态度很好。"},
            {"4", "医生还行，诊断比较准确。"},
            {"4", "总体满意，但等候时间有点长。"},
            {"3", "医生态度一般，没有特别差但也没有很好。"},
            {"3", "看诊时间有点短，感觉有点敷衍。"},
            {"2", "医生不太耐烦，说话比较冲。"},
            {"1", "非常不满意，误诊了病情，耽误了治疗。"},
            {"4", "医生不错，但候诊大厅太吵了，影响休息。"},
            {"3", "医生诊断准确，但医院厕所卫生状况不佳。"},
            {"2", "医生态度可以，但停车场车位太少，停车困难。"},
            {"3", "诊疗过程顺利，但医院网络信号太差，无法使用手机。"}
        };
        int num_feedback = sizeof(feedback_dict) / sizeof(feedback_dict[0]);
        const char* diag_dict[] = {"急性上呼吸道感染", "原发性高血压", "急性胃肠炎", "过敏性鼻炎", "轻度支气管炎"};
        const char* adv_dict[] = {"注意休息，按时服药，多饮水", "低盐低脂饮食，定期监测血压", "清淡饮食，避免辛辣", "远离过敏原，外出佩戴口罩", "注意保暖，避免受凉"};

        // 检查项目字典 (编号、名称、科室、结果)
        const char* check_items[][4] = {
            {"C-010", "血常规", "检验科", "白细胞略高，其余各项指标均在正常参考值范围内"},
            {"C-011", "生化全项", "检验科", "肝肾功能正常，血糖偏高，需控制饮食"},
            {"C-012", "肝功能", "检验科", "谷丙转氨酶略高，建议清淡饮食一周后复查"},
            {"C-014", "血糖检测", "检验科", "空腹血糖6.8mmol/L偏高，需进一步检查"},
            {"C-006", "腹部B超", "超声科", "肝胆脾胰未见明显异常"},
            {"C-007", "心脏彩超", "超声科", "心脏结构及功能未见明显异常"},
            {"C-018", "心电图", "心电图室", "窦性心律，心电图大致正常"},
            {"C-002", "CT扫描", "放射科", "胸部CT未见明显异常"}
        };
        int num_check_items = sizeof(check_items) / sizeof(check_items[0]);

        for (i = 1; i <= 40; i++) {
            char p_id[32], d_id[32], appt_id[32], consult_id[32], check_id[32], date_str[32];
            sprintf(p_id, "P-1%03d", i);
            sprintf(d_id, "D-2%02d", (i % 60) + 1);
            sprintf(appt_id, "A-202603%02d-%04d", (i % 28) + 1, i);
            sprintf(consult_id, "CR-%04d", i);
            sprintf(check_id, "CHK-%04d", i);
            sprintf(date_str, "2026-03-%02d", (i % 28) + 1);

            AppointmentNode* appt = create_appointment_node(appt_id, p_id, date_str, (i % 2 == 0) ? "上午" : "下午", d_id, (char*)all_depts[i % 8], 3);
            if (appt) {
                appt->reg_fee = 15.0;
                appt->fee_paid = 1;
                insert_appointment_tail(g_appointment_list, appt);
            }

            ConsultRecordNode* consult = create_consult_record_node(consult_id, p_id, d_id, appt_id, date_str,
                (char*)diag_dict[i % 5], (char*)adv_dict[i % 5], 2, 0, 0);
            if (consult) {
                int fb_idx = i % num_feedback;
                consult->star_rating = atoi(feedback_dict[fb_idx][0]);
                strcpy(consult->feedback, feedback_dict[fb_idx][1]);
                insert_consult_record_tail(g_consult_record_list, consult);
            }

            int check_idx = i % num_check_items;
            CheckRecordNode* check = create_check_record_node(check_id, p_id,
                (char*)check_items[check_idx][0], (char*)check_items[check_idx][1], (char*)check_items[check_idx][2], date_str,
                (char*)check_items[check_idx][3], 1, 1);
            if (check) insert_check_record_tail(g_check_record_list, check);
        }

        // 投诉字典 (目标类型、目标ID、目标名称、投诉内容、回复)
        // target_type: 1=医生, 2=护士/前台, 3=药师
        const char* complaints_dict[][5] = {
            {"1", "D-201", "李建国", "医生看诊时间太短，态度不够耐心。", "已与该医生沟通，要求加强与患者的沟通交流。"},
            {"2", "nurse", "护士", "护士扎针技术不熟练，扎了两次才成功。", "已对当班护士进行技术培训，提升操作水平。"},
            {"3", "pharm", "药剂师", "药师没有解释用药注意事项就直接发药。", "已加强药房人员的用药指导培训。"},
            {"1", "D-205", "吴子轩", "医生开的药方剂量过大，担心有副作用。", "已联系该医生核实用药剂量，确认符合规范。"},
            {"2", "nurse", "护士", "护士值班时不在岗位，等了很久才找到人。", "已要求护士站加强值班管理，确保在岗在位。"},
            {"1", "D-210", "郑宇轩", "医生诊断不够仔细，没有找出真正的病因。", "已安排该患者进行复诊，并优化诊断流程。"},
            {"3", "pharm", "药剂师", "等了半小时才拿到药，窗口排队时间过长。", "已增加药房工作人员，缩短患者取药等待时间。"},
            {"1", "D-215", "刘建国", "医生服务态度很差，说话语气很冲。", "已对该医生进行医德医风教育，要求改进服务态度。"}
        };
        int num_complaints = sizeof(complaints_dict) / sizeof(complaints_dict[0]);

        // 7. 生成多条医患投诉记录
        for (i = 1; i <= 8; i++) {
            char p_id[32], comp_id[32];
            sprintf(p_id, "P-1%03d", i + 40);
            sprintf(comp_id, "CP-%04d", i);
            int comp_idx = (i - 1) % num_complaints;
            ComplaintNode* comp = create_complaint_node(comp_id, p_id,
                atoi(complaints_dict[comp_idx][0]),  // target_type
                (char*)complaints_dict[comp_idx][1],  // target_id
                (char*)complaints_dict[comp_idx][2],  // target_name
                (char*)complaints_dict[comp_idx][3],  // content
                1,
                (char*)complaints_dict[comp_idx][4],  // response
                "2026-04-01");
            if (comp) insert_complaint_tail(g_complaint_list, comp);
        }

        printf("✅ 批量测试数据注入完毕！(包含150患者, 40住院, 120医生, 30药品, 50病床)\n");
        
        // 8. 模拟黑名单患者触发安全预警（让alerts.txt有数据）
        printf("\n📝 正在模拟黑名单患者行为，生成安全预警数据...\n");
        
        // 模拟永久黑名单患者（李逃单）试图挂号 - 触发拦截预警
        PatientNode* blacklisted_patient1 = find_patient_by_id(g_patient_list, "P-1001");
        if (blacklisted_patient1 != NULL) {
            char alert_msg[256];
            snprintf(alert_msg, sizeof(alert_msg), 
                "⛔ 恶意挂号被拦截：患者 %s（永久黑名单）试图挂号，已拦截！", 
                blacklisted_patient1->name);
            push_system_alert(alert_msg);
            printf("   -> 永久黑名单患者 %s 挂号拦截预警已生成\n", blacklisted_patient1->name);
        }
        
        // 模拟临时黑名单患者（王爽约）试图挂号 - 触发拦截预警
        PatientNode* blacklisted_patient2 = find_patient_by_id(g_patient_list, "P-1002");
        if (blacklisted_patient2 != NULL) {
            char alert_msg[256];
            snprintf(alert_msg, sizeof(alert_msg), 
                "⛔ 爽约黑名单拦截：患者 %s（爽约3次）在惩罚期内试图挂号，已拦截！", 
                blacklisted_patient2->name);
            push_system_alert(alert_msg);
            printf("   -> 爽约黑名单患者 %s 挂号拦截预警已生成\n", blacklisted_patient2->name);
        }
        
        // 模拟黑名单患者突发急诊 - 触发强制放行预警
        PatientNode* blacklisted_patient3 = find_patient_by_id(g_patient_list, "P-1003");
        if (blacklisted_patient3 != NULL) {
            char alert_msg[256];
            snprintf(alert_msg, sizeof(alert_msg), 
                "🚨 急诊强制放行：患者 %s（黑名单人员）突发危重急症，已强制放行，请相关部门跟进！", 
                blacklisted_patient3->name);
            push_system_alert(alert_msg);
            printf("   -> 黑名单患者 %s 急诊强制放行预警已生成\n", blacklisted_patient3->name);
        }
        
        printf("✅ 安全预警数据生成完毕！\n");
    }
    else
    {
        printf("📂 成功从本地加载了存档数据！\n");
    }
    
    // 🚀 时停魔法：按任意键后才清屏进入菜单！
    system("pause");
    
    // 进入主菜单循环
    int running = 1; // 控制系统运行状态的标志位

    while (running) 
    {
        // 每次循环清空屏幕，保持 UI 整洁 (Windows用cls, Mac/Linux用clear)
        system("cls"); 
        
       printf("\n======================================================\n");
        printf("               🏥 社区智慧医疗管理系统\n");
        printf("======================================================\n");
        printf("  [1] 🔑 内部登录 (医生/护士/药房专员/管理员)\n");
        printf("  [2] 🤗 患者自助服务 (挂号/预约/查询)\n");
        printf("  [3] 📊 医疗大屏 (内部数据可视化展示)\n");
        printf("  [0] 🚪 安全退出系统\n");
        printf("------------------------------------------------------\n");

        int choice = get_safe_int("👉 请输入操作编号: ");

        switch (choice) {
            case 1:
                printf("\n[跳转] -> 进入内部人员登录模块...\n");
                internal_login_menu(); 
                break;
            case 2:
                printf("\n[跳转] -> 进入患者自助服务...\n");
                patient_self_service_menu();
                break;
            case 3:
                printf("\n[提示] 医疗大屏功能后续开发...\n");
                system("pause");
                break;
            case 0:
                // 准备退出主循环
                printf("\n正在准备退出系统...\n");
                running = 0; 
                break;
            default:
                // 处理 0-3 之外的非法数字
                printf("\n⚠️ 无效的选项，请重新输入！\n");
                system("pause");
                break;
        }
    }

    // ---------------------------------------------------------
    // 第三阶段：系统清理与安全退出 (数据持久化)
    // ---------------------------------------------------------
    printf("\n======================================================\n");
    printf("     💾 正在保存系统数据到本地文件...\n");
    
    // 这里未来调用 data_io.c 里的函数，把当前双向链表的数据覆盖写入 txt
    save_all_data();

    printf("     🧹 正在释放链表内存，清空回收站...\n");
    // 销毁所有双向链表，防止内存泄漏 (非常重要，代码检查必看)
    destroy_all_lists(); 

    printf("\n✅ 数据已安全保存，感谢使用！再见！\n");
    printf("======================================================\n");

    system("pause"); // 等待用户按任意键
    return 0;
}

// 病房管理菜单
static void inpatient_menu()
{
    int running = 1;
    int choice;

    while (running)
    {
        system("cls");
        printf("\n======================================================\n"); 
        printf("               🏥 病房管理\n");
        printf("======================================================\n");   
        printf("  [1] 查看全部床位信息\n");
        printf("  [2] 查看空闲床位\n");
        printf("  [3] 查看床位占用情况\n");
        printf("  [4] 根据床位编号查询\n");
        printf("  [5] 住院登记\n");
        printf("  [6] 分配床位\n");
        printf("  [7] 查询住院患者信息\n");
        printf("  [8] 查询患者住院记录\n");
        printf("  [9] 按住院号查\n");
        printf("  [10] 按科室查住院患者\n");
        printf("  [11] 已出院患者\n");
        printf("  [12] 押金充值\n");
        printf("  [13] 日结计费\n");
        printf("  [14] 转床\n");
        printf("  [15] 办理出院（自动释放床位）\n");
        printf("  [16] 床位/押金预警信息\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");   

        choice = get_safe_int("👉 请输入操作编号: ");

        switch (choice)
        {
            case 1:
                show_all_beds();
                system("pause");
                break;
            case 2:
                show_free_beds();
                system("pause");
                break;
            case 3:
                {
                    printf("\n===============================================================\n");
                    printf("                      床位占用情况\n");
                    printf("===============================================================\n");
                    print_col("病房编号", 12);
                    print_col("床位编号", 12);
                    print_col("类型", 10);
                    print_col("患者编号", 13);
                    print_col("患者姓名", 12);
                    print_col("身份证号", 18);
                    printf("\n");
                    printf("---------------------------------------------------------------\n");

                    if (g_ward_list != NULL && g_ward_list->next != NULL)     
                    {
                        WardNode* ward_curr = g_ward_list->next;
                        int has_occupied_beds = 0;
                        while (ward_curr != NULL)
                        {
                            if (ward_curr->is_occupied)
                            {
                                has_occupied_beds = 1;
                                const char* p_name = "未知";
                                const char* p_card = "未知";
                                PatientNode* patient = find_patient_by_id(g_patient_list, ward_curr->patient_id);
                                if (patient)
                                {
                                    char masked_id_card[20];
                                    mask_id_card(patient->id_card, masked_id_card);
                                    p_name = patient->name;
                                    p_card = masked_id_card;
                                }

                                const char* t = ward_curr->ward_type == WARD_TYPE_ICU ? "ICU" :
                                    (ward_curr->ward_type == WARD_TYPE_ISOLATION ? "隔离病房" :
                                    (ward_curr->ward_type == WARD_TYPE_SINGLE ? "单人病房" : "普通病房"));

                                print_col(ward_curr->room_id, 12);
                                print_col(ward_curr->bed_id, 12);
                                print_col(t, 10);
                                print_col(ward_curr->patient_id, 13);
                                print_col(p_name, 12);
                                print_col(p_card, 18);
                                printf("\n");
                            }
                            ward_curr = ward_curr->next;
                        }
                        if (!has_occupied_beds)
                        {
                            printf("  当前暂无已占用床位\n");
                        }
                        printf("---------------------------------------------------------------\n");
                    }
                    else
                    {
                        printf("  当前无床位数据\n");
                        printf("---------------------------------------------------------------\n");
                    }
                    system("pause");
                }
                break;
            case 4:
                {
                    char bed_id[MAX_ID_LEN];
                    printf("\n================ 根据床位编号查询 ===============-\n");
                    get_safe_string("请输入床位编号: ", bed_id, MAX_ID_LEN);
                    
                    if (g_ward_list != NULL && g_ward_list->next != NULL)
                    {
                        WardNode* ward_curr = g_ward_list->next;
                        int found = 0;
                        while (ward_curr != NULL)
                        {
                            if (strcmp(ward_curr->bed_id, bed_id) == 0)
                            {
                                found = 1;
                                printf("\n======================================================\n");
                                printf("                   床位详细信息\n");
                                printf("======================================================\n");
                                printf("病房编号: %s\n", ward_curr->room_id);
                                printf("床位编号: %s\n", ward_curr->bed_id);
                                printf("病房类型: %s\n", ward_curr->ward_type == WARD_TYPE_ICU ? "ICU" : (ward_curr->ward_type == WARD_TYPE_ISOLATION ? "隔离病房" : (ward_curr->ward_type == WARD_TYPE_SINGLE ? "单人病房" : "普通病房")));
                                printf("床位状态: %s\n", ward_curr->is_occupied ? "占用" : "空闲");
                                
                                if (ward_curr->is_occupied)
                                {
                                    printf("患者编号: %s\n", ward_curr->patient_id);
                                    // 查找患者信息
                                    PatientNode* patient = NULL;
                                    if (g_patient_list != NULL && g_patient_list->next != NULL)
                                    {
                                        PatientNode* patient_curr = g_patient_list->next;
                                        while (patient_curr != NULL)
                                        {
                                            if (strcmp(patient_curr->id, ward_curr->patient_id) == 0)
                                            {
                                                patient = patient_curr;
                                                break;
                                            }
                                            patient_curr = patient_curr->next;
                                        }
                                    }
                                    
                                    if (patient != NULL)
                                    {
                                        char masked_id_card[20];
                                        mask_id_card(patient->id_card, masked_id_card);
                                        printf("患者姓名: %s\n", patient->name);
                                        printf("身份证号: %s\n", masked_id_card);
                                        printf("年龄: %d\n", patient->age);
                                        printf("医保类型: %s\n", patient->m_type == 0 ? "无医保" : (patient->m_type == 1 ? "甲类医保" : (patient->m_type == 2 ? "乙类医保" : "未知")));
                                        printf("余额: %.2f\n", patient->balance);
                                        printf("症状: %s\n", patient->symptom);
                                        printf("目标科室: %s\n", patient->target_dept);
                                    }
                                    else
                                    {
                                        printf("患者信息: 未知\n");
                                    }
                                }
                                printf("======================================================\n");
                                break;
                            }
                            ward_curr = ward_curr->next;
                        }
                        
                        if (!found)
                        {
                            printf("未找到床位编号为 %s 的床位\n", bed_id);
                        }
                    }
                    else
                    {
                        printf("当前无床位数据\n");
                    }
                    system("pause");
                }
                break;
            case 5:
                handle_inpatient_register();
                system("pause");
                break;
            case 6:
                handle_bed_assign();
                break;
            case 7:
                show_hospitalized_patients();
                system("pause");
                break;
            case 8:
                handle_inpatient_record_query();
                break;
            case 9:
                handle_inpatient_query_by_id();
                break;
            case 10:
                handle_inpatient_query_by_dept();
                break;
            case 11:
                handle_discharged_patients();
                break;
            case 12:
                handle_deposit_recharge();
                break;

            case 13:
                handle_daily_settlement();
                break;
            case 14:
                handle_transfer_bed();
                break;
            case 15:
                handle_discharge();
                break;
            case 16:
                show_deposit_warnings();
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

// 住院登记处理函数
static void handle_inpatient_register()
{
    char patient_id[MAX_ID_LEN];
    int estimated_days;
    double deposit;
    int condition_level;
    PatientNode* patient = NULL;
    InpatientRecord* inpatient_record = NULL;
    char input_buf[64];
    int i;
    double temp_deposit;
    
    printf("\n================ 住院登记 ===============-\n");
    
enter_patient:
    while (1)
    {
        get_safe_string("请输入患者编号 (输入B返回上一级): ", patient_id, MAX_ID_LEN);
        
        if (my_strcasecmp(patient_id, "b") == 0)
        {
            printf("\n已取消本次住院登记，返回上一级\n");
            return;
        }
        
        if (is_blank_string(patient_id))
        {
            printf("⚠️ 患者编号不能为空，请重新输入\n");
            continue;
        }
        
        patient = find_patient_by_id(g_patient_list, patient_id);
        if (patient == NULL)
        {
            printf("⚠️ 未找到该患者，请重新输入患者编号或先建档\n");
            continue;
        }

        inpatient_record = find_active_inpatient_by_patient_id(patient_id);
        if (inpatient_record != NULL)
        {
            printf("⚠️ 该患者当前已住院，不能重复登记，请重新输入患者编号\n");
            continue;
        }

        if (patient->status != STATUS_NEED_HOSPITALIZE)
        {
            printf("⚠️ 该患者未被医生开具住院通知，不能办理住院登记！\n");
            printf("   请先让医生为患者开具住院通知\n");
            continue;
        }

        break;
    }
    
enter_days:
    while (1)
    {
        get_safe_string("请输入预计住院天数 (输入B回退上一步): ", input_buf, sizeof(input_buf));
        
        if (my_strcasecmp(input_buf, "b") == 0)
            goto enter_patient;
        
        for (i = 0; input_buf[i] != '\0'; i++)
        {
            if (!isdigit((unsigned char)input_buf[i]))
            {
                break;
            }
        }
        if (input_buf[i] != '\0' || i == 0)
        {
            printf("⚠️ 预计住院天数必须是正整数，请重新输入\n");
            continue;
        }
        
        estimated_days = atoi(input_buf);
        if (estimated_days <= 0)
        {
            printf("⚠️ 预计住院天数必须大于0，请重新输入\n");
            continue;
        }
        
        break;
    }
    
enter_deposit:
    while (1)
    {
        get_safe_string("请输入初始押金金额 (输入B回退上一步): ", input_buf, sizeof(input_buf));
        
        if (my_strcasecmp(input_buf, "b") == 0)
            goto enter_days;
        
        char* endptr;
        temp_deposit = strtod(input_buf, &endptr);
        if (*endptr != '\0' || endptr == input_buf)
        {
            printf("⚠️ 初始押金必须是合法数字，请重新输入\n");
            continue;
        }
        if (temp_deposit <= 0)
        {
            printf("⚠️ 初始押金必须大于0，请重新输入\n");
            continue;
        }
        
        deposit = temp_deposit;
        break;
    }
    
enter_condition:
    while (1)
    {
        get_safe_string("请输入病情级别 (1=普通, 2=重症, 输入B回退上一步): ", input_buf, sizeof(input_buf));
        
        if (my_strcasecmp(input_buf, "b") == 0)
            goto enter_deposit;
        
        for (i = 0; input_buf[i] != '\0'; i++)
        {
            if (!isdigit((unsigned char)input_buf[i]))
            {
                break;
            }
        }
        if (input_buf[i] != '\0' || i == 0)
        {
            printf("⚠️ 病情级别必须是1或2，请重新输入\n");
            continue;
        }
        
        condition_level = atoi(input_buf);
        if (condition_level != 1 && condition_level != 2)
        {
            printf("⚠️ 病情级别必须是1或2，请重新输入\n");
            continue;
        }
        
        break;
    }
    
    if (patient->balance < deposit)
    {
        printf("\n⚠️ 患者账户余额不足！\n");
        printf("   当前余额: %.2f 元\n", patient->balance);
        printf("   需要押金: %.2f 元\n", deposit);
        printf("   差额: %.2f 元\n", deposit - patient->balance);
        printf("   输入 B 重新输入押金\n");
        get_safe_string("按 B 键返回: ", input_buf, sizeof(input_buf));
        goto enter_deposit;
    }
    
    // 显示住院登记摘要
    printf("\n======================================================\n");
    printf("                   住院登记摘要\n");
    printf("======================================================\n");
    printf("患者编号：%s\n", patient_id);
    printf("患者姓名：%s\n", patient->name);
    printf("预计住院天数：%d 天\n", estimated_days);
    printf("初始押金：%.2f 元\n", deposit);
    printf("病情级别：%s\n", condition_level == 1 ? "普通" : "重症");
    printf("推荐病房类型：%s\n", condition_level == 2 ? "ICU" : "普通病房");
    printf("患者当前余额：%.2f 元\n", patient->balance);
    printf("扣除押金后余额：%.2f 元\n", patient->balance - deposit);
    printf("======================================================\n");
    
    // 确认提交
    get_safe_string("是否确认提交住院登记？(Y=确认, B=返回修改): ", input_buf, sizeof(input_buf));
    if (my_strcasecmp(input_buf, "y") != 0 && my_strcasecmp(input_buf, "yes") != 0)
    {
        goto enter_condition;
    }
    
    // 扣除押金
    patient->balance -= deposit;
    
    // 调用住院登记函数
    if (register_inpatient(patient_id, estimated_days, deposit, condition_level))
    {
        printf("\n✅ 住院登记成功！\n");
        
        // 询问是否立即分配床位
        get_safe_string("是否立即为患者分配床位？(Y=确认, B=不分配): ", input_buf, sizeof(input_buf));
        if (my_strcasecmp(input_buf, "y") == 0 || my_strcasecmp(input_buf, "yes") == 0)
        {
            show_free_beds();
            
            char bed_id[MAX_ID_LEN];
            while (1)
            {
                get_safe_string("请输入床位编号 (输入B跳过): ", bed_id, MAX_ID_LEN);
                if (my_strcasecmp(bed_id, "b") == 0) break;
                if (is_blank_string(bed_id))
                {
                    printf("⚠️ 床位编号不能为空，请重新输入\n");
                    continue;
                }
                if (assign_bed_to_patient(patient_id, bed_id))
                    printf("\n✅ 床位分配成功！\n");
                else
                    continue;
                break;
            }
        }
    }
    else
    {
        // 登记失败，回滚押金
        patient->balance += deposit;
        printf("\n❌ 住院登记失败，押金已退回\n");
    }
}

// 分配床位处理函数
static void handle_bed_assign()
{
    char patient_id[MAX_ID_LEN];
    char bed_id[MAX_ID_LEN];
    PatientNode* patient = NULL;
    InpatientRecord* inpatient_record = NULL;
    
    printf("\n================ 分配床位 ===============-\n");
    
enter_patient_bed:
    while (1)
    {
        get_safe_string("请输入患者编号 (输入B返回上一级): ", patient_id, MAX_ID_LEN);
        
        if (my_strcasecmp(patient_id, "b") == 0)
        {
            printf("\n已取消本次床位分配，返回上一级\n");
            system("pause");
            return;
        }
        
        if (is_blank_string(patient_id))
        {
            printf("⚠️ 患者编号不能为空，请重新输入\n");
            continue;
        }
        
        patient = find_patient_by_id(g_patient_list, patient_id);
        if (patient == NULL)
        {
            printf("⚠️ 未找到该患者，请重新输入患者编号\n");
            continue;
        }
        
        inpatient_record = find_active_inpatient_by_patient_id(patient_id);
        if (inpatient_record == NULL)
        {
            printf("⚠️ 该患者未住院，请先办理住院登记\n");
            continue;
        }
        
        if (patient_has_bed(patient_id))
        {
            printf("⚠️ 该患者已分配床位，不能重复分配\n");
            continue;
        }
        
        break;
    }
    
    printf("\n📋 空闲床位列表：\n");
    show_free_beds();
    
    while (1)
    {
        get_safe_string("请输入床位编号 (输入B回退上一步): ", bed_id, MAX_ID_LEN);
        
        if (my_strcasecmp(bed_id, "b") == 0)
            goto enter_patient_bed;
        
        if (is_blank_string(bed_id))
        {
            printf("⚠️ 床位编号不能为空，请重新输入\n");
            continue;
        }
        
        WardNode* target_bed = find_bed_by_id(bed_id);
        if (target_bed == NULL)
        {
            printf("⚠️ 床位不存在，请重新输入\n");
            continue;
        }
        if (target_bed->is_occupied)
        {
            printf("⚠️ 该床位已被占用，请重新输入\n");
            continue;
        }
        
        if (assign_bed_to_patient(patient_id, bed_id))
        {
            printf("\n✅ 床位分配成功！\n");
            break;
        }
        printf("\n❌ 床位分配失败，请重新输入\n");
    }
    
    system("pause");
}

// 查询住院记录处理函数
static void handle_inpatient_record_query()
{
    char patient_id[MAX_ID_LEN];
    
    printf("\n================ 查询住院记录 ===============-\n");
    
    // 输入患者编号（不能为空、必须存在；输入Q取消）
    while (1)
    {
        get_safe_string("请输入患者编号 (输入Q返回上一级): ", patient_id, MAX_ID_LEN);
        
        // 检查是否取消
        if (my_strcasecmp(patient_id, "q") == 0)
        {
            printf("\n已取消本次查询，返回上一级\n");
            system("pause");
            return;
        }
        
        // 检查患者编号是否为空
        if (is_blank_string(patient_id))
        {
            printf("⚠️ 患者编号不能为空，请重新输入\n");
            continue;
        }
        
        // 检查患者是否存在
        if (find_patient_by_id(g_patient_list, patient_id) == NULL)
        {
            printf("⚠️ 未找到该患者，请重新输入患者编号\n");
            continue;
        }
        
        // 所有校验通过
        break;
    }
    
    // 调用查询函数
    show_inpatient_record_by_patient_id(patient_id);
    
    system("pause");
}

static void handle_inpatient_query_by_id()
{
    char query_str[MAX_ID_LEN];

    printf("\n================ 按住院号查询 ===============-\n");

    while (1)
    {
        get_safe_string("请输入住院号（输入B返回上一级）: ", query_str, MAX_ID_LEN);
        if (my_strcasecmp(query_str, "b") == 0) return;
        if (is_blank_string(query_str))
        {
            printf("⚠️ 住院号不能为空，请重新输入\n");
            continue;
        }
        if (strncmp(query_str, "I-", 2) != 0)
        {
            printf("⚠️ 住院号格式应为 I-xxx，请重新输入\n");
            continue;
        }
        break;
    }

    show_inpatient_record_by_inpatient_id(query_str);
    system("pause");
}

static void handle_inpatient_query_by_dept()
{
    char dept[MAX_NAME_LEN];

    printf("\n================ 按科室查询住院患者 ===============-\n");
    printf("可用科室：内科、外科、综合重症、感染科、综合、妇产科 等\n");

    while (1)
    {
        get_safe_string("请输入科室（输入B返回上一级）: ", dept, MAX_NAME_LEN);
        if (my_strcasecmp(dept, "b") == 0) return;
        if (is_blank_string(dept))
        {
            printf("⚠️ 科室名称不能为空，请重新输入\n");
            continue;
        }
        break;
    }

    show_hospitalized_patients_by_dept(dept);
    system("pause");
}

static void handle_discharged_patients()
{
    show_discharged_patients();
    system("pause");
}

// 押金充值处理函数
static void handle_deposit_recharge()
{
    char patient_id[MAX_ID_LEN];
    double amount;
    char input_buf[64];
    
    printf("\n================ 押金充值 ===============-\n");
    
    // 输入患者编号（不能为空、必须存在、必须已住院；输入Q取消）
    while (1)
    {
        get_safe_string("请输入患者编号 (输入Q返回上一级): ", patient_id, MAX_ID_LEN);
        
        // 检查是否取消
        if (my_strcasecmp(patient_id, "q") == 0)
        {
            printf("\n已取消本次押金充值，返回上一级\n");
            system("pause");
            return;
        }
        
        // 检查患者编号是否为空
        if (is_blank_string(patient_id))
        {
            printf("⚠️ 患者编号不能为空，请重新输入\n");
            continue;
        }
        
        // 检查患者是否存在
        if (find_patient_by_id(g_patient_list, patient_id) == NULL)
        {
            printf("⚠️ 未找到该患者，请重新输入患者编号\n");
            continue;
        }
        
        // 检查患者是否已住院
        if (find_active_inpatient_by_patient_id(patient_id) == NULL)
        {
            printf("⚠️ 该患者未住院，不能进行押金充值\n");
            continue;
        }
        
        // 所有校验通过
        break;
    }
    
    // 输入充值金额（必须大于0；输入Q取消）
    while (1)
    {
        printf("请输入充值金额 (输入Q返回上一级): ");
        get_safe_string("", input_buf, sizeof(input_buf));
        
        // 检查是否取消
        if (my_strcasecmp(input_buf, "q") == 0)
        {
            printf("\n已取消本次押金充值，返回上一级\n");
            system("pause");
            return;
        }
        
        // 转换为浮点数并验证
        amount = atof(input_buf);
        if (amount <= 0)
        {
            printf("⚠️ 充值金额必须大于0，请重新输入\n");
            continue;
        }
        
        break;
    }
    
    // 调用押金充值函数
    if (recharge_inpatient_deposit(patient_id, amount))
    {
        printf("\n✅ 押金充值成功！\n");
    }
    else
    {
        printf("\n❌ 押金充值失败，请检查输入信息\n");
    }
    
    system("pause");
}

// 日结计费处理函数
static void handle_daily_settlement()
{
    char patient_id[MAX_ID_LEN];
    
    printf("\n================ 日结计费 ===============-\n");
    
    // 输入患者编号（不能为空、必须存在、必须已住院；输入Q取消）
    while (1)
    {
        get_safe_string("请输入患者编号 (输入Q返回上一级): ", patient_id, MAX_ID_LEN);
        
        // 检查是否取消
        if (my_strcasecmp(patient_id, "q") == 0)
        {
            printf("\n已取消本次日结计费，返回上一级\n");
            system("pause");
            return;
        }
        
        // 检查患者编号是否为空
        if (is_blank_string(patient_id))
        {
            printf("⚠️ 患者编号不能为空，请重新输入\n");
            continue;
        }
        
        // 检查患者是否存在
        if (find_patient_by_id(g_patient_list, patient_id) == NULL)
        {
            printf("⚠️ 未找到该患者，请重新输入患者编号\n");
            continue;
        }
        
        // 检查患者是否已住院
        if (find_active_inpatient_by_patient_id(patient_id) == NULL)
        {
            printf("⚠️ 该患者未住院，不能进行日结计费\n");
            continue;
        }
        
        // 所有校验通过
        break;
    }
    
    // 调用日结计费函数
    if (daily_settlement(patient_id))
    {
        printf("\n✅ 日结计费成功！\n");
    }
    else
    {
        printf("\n❌ 日结计费失败，请检查输入信息\n");
    }
    
    system("pause");
}

// 转床处理函数
static void handle_transfer_bed()
{
    char patient_id[MAX_ID_LEN];
    char old_bed_id[MAX_ID_LEN];
    char new_bed_id[MAX_ID_LEN];
    
    printf("\n================ 转床 ===============-\n");
    
    // 输入患者编号（不能为空、必须存在、必须已住院且已分配床位；输入Q取消）
    while (1)
    {
        get_safe_string("请输入患者编号 (输入Q返回上一级): ", patient_id, MAX_ID_LEN);
        
        // 检查是否取消
        if (my_strcasecmp(patient_id, "q") == 0)
        {
            printf("\n已取消本次转床操作，返回上一级\n");
            system("pause");
            return;
        }
        
        // 检查患者编号是否为空
        if (is_blank_string(patient_id))
        {
            printf("⚠️ 患者编号不能为空，请重新输入\n");
            continue;
        }
        
        // 检查患者是否存在
        if (find_patient_by_id(g_patient_list, patient_id) == NULL)
        {
            printf("⚠️ 未找到该患者，请重新输入患者编号\n");
            continue;
        }
        
        // 检查患者是否已住院
        if (find_active_inpatient_by_patient_id(patient_id) == NULL)
        {
            printf("⚠️ 该患者未住院，不能进行转床操作\n");
            continue;
        }
        
        // 检查患者是否已分配床位
        if (!patient_has_bed(patient_id))
        {
            printf("⚠️ 该患者未分配床位，不能进行转床操作\n");
            continue;
        }
        
        // 所有校验通过
        break;
    }
    
    // 输入原床位编号（不能为空、必须存在、必须是患者当前占用的床位；输入Q取消）
    while (1)
    {
        get_safe_string("请输入原床位编号 (输入Q返回上一级): ", old_bed_id, MAX_ID_LEN);
        
        // 检查是否取消
        if (my_strcasecmp(old_bed_id, "q") == 0)
        {
            printf("\n已取消本次转床操作，返回上一级\n");
            system("pause");
            return;
        }
        
        // 检查原床位编号是否为空
        if (is_blank_string(old_bed_id))
        {
            printf("⚠️ 原床位编号不能为空，请重新输入\n");
            continue;
        }
        
        // 检查原床位是否存在
        WardNode* old_bed = find_bed_by_id(old_bed_id);
        if (old_bed == NULL)
        {
            printf("⚠️ 原床位不存在，请重新输入\n");
            continue;
        }
        
        // 检查原床位是否被该患者占用
        if (!old_bed->is_occupied || strcmp(old_bed->patient_id, patient_id) != 0)
        {
            printf("⚠️ 原床位不是该患者当前占用的床位，请重新输入\n");
            continue;
        }
        
        // 所有校验通过
        break;
    }
    
    // 显示空闲床位
    printf("\n📋 空闲床位列表：\n");
    show_free_beds();
    
    // 输入新床位编号（不能为空、必须存在、必须空闲；输入Q取消）
    while (1)
    {
        get_safe_string("请输入新床位编号 (输入Q返回上一级): ", new_bed_id, MAX_ID_LEN);
        
        // 检查是否取消
        if (my_strcasecmp(new_bed_id, "q") == 0)
        {
            printf("\n已取消本次转床操作，返回上一级\n");
            system("pause");
            return;
        }
        
        // 检查新床位编号是否为空
        if (is_blank_string(new_bed_id))
        {
            printf("⚠️ 新床位编号不能为空，请重新输入\n");
            continue;
        }
        
        WardNode* new_target = find_bed_by_id(new_bed_id);
        if (new_target == NULL)
        {
            printf("⚠️ 新床位不存在，请重新输入\n");
            continue;
        }
        if (strcmp(new_bed_id, old_bed_id) == 0)
        {
            printf("⚠️ 新床位与原床位相同，无需转床，请重新输入\n");
            continue;
        }
        if (new_target->is_occupied)
        {
            printf("⚠️ 新床位已被占用，请重新输入\n");
            continue;
        }
        
        break;
    }
    
    // 调用转床函数
    if (transfer_bed(patient_id, old_bed_id, new_bed_id))
    {
        printf("\n✅ 转床成功！\n");
    }
    else
    {
        printf("\n❌ 转床失败，请检查输入信息\n");
    }
    
    system("pause");
}

// 办理出院处理函数
static void handle_discharge()
{
    char patient_id[MAX_ID_LEN];
    
    printf("\n================ 办理出院（自动释放床位） ===============-\n");
    
    // 输入患者编号（不能为空、必须存在、必须已住院；输入Q取消）
    while (1)
    {
        get_safe_string("请输入患者编号 (输入Q返回上一级): ", patient_id, MAX_ID_LEN);
        
        // 检查是否取消
        if (my_strcasecmp(patient_id, "q") == 0)
        {
            printf("\n已取消本次出院办理，返回上一级\n");
            system("pause");
            return;
        }
        
        // 检查患者编号是否为空
        if (is_blank_string(patient_id))
        {
            printf("⚠️ 患者编号不能为空，请重新输入\n");
            continue;
        }
        
        // 检查患者是否存在
        if (find_patient_by_id(g_patient_list, patient_id) == NULL)
        {
            printf("⚠️ 未找到该患者，请重新输入患者编号\n");
            continue;
        }
        
        // 检查患者是否已住院
        if (find_active_inpatient_by_patient_id(patient_id) == NULL)
        {
            printf("⚠️ 该患者未住院，不能办理出院\n");
            continue;
        }
        
        // 所有校验通过
        break;
    }
    
    // 调用出院函数
    if (discharge_patient(patient_id))
    {
        printf("\n✅ 出院办理成功！\n");
        printf("📋 床位已自动释放\n");
    }
    else
    {
        printf("\n❌ 出院办理失败，请检查输入信息\n");
    }
    
    system("pause");
}

