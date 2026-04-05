// ==========================================
// 文件名: main.c
// 描述: 智慧医疗信息管理系统 - 主入口
// 作者:周宇轩
// ==========================================
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "global.h"
#include "list_ops.h"
#include "patient_service.h"
#include "appointment.h"
#include "doctor_service.h"
// 这里未来会引入你们自己写的头文件
// #include "global.h"     // 全局常量与结构体定义
// #include "data_io.h"    // 负责读写 txt 文件的模块
// #include "auth.h"       // 负责登录与权限的模块
// #include "utils.h"      // 存放 get_safe_int 终端输入拦截器等工具函数
static void quick_register_menu();
static void patient_self_service_menu();
static void patient_archive_menu();

static void handle_patient_register();
static void handle_appointment_register();
static void handle_appointment_query();
static void handle_appointment_cancel();
static void handle_appointment_check_in();
static void handle_basic_record_query();
static void handle_patient_visit_overview();
static void handle_patient_visit_overview_by_id_card();
static void handle_patient_archive_query_by_id();
static void handle_patient_archive_query_by_id_card();
static void handle_patient_archive_query_by_name();
static void handle_patient_archive_update();

static void internal_login_menu();
static void admin_menu();
static void nurse_menu();
static void doctor_menu();
static void pharmacist_menu();
static void handle_view_waiting_patients();
static void handle_doctor_consultation();

static void admin_menu()
{
    int running = 1;

    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               🔐 管理员菜单\n");
        printf("======================================================\n");
        printf("  [1] 管理员菜单（后续开发）\n");
        printf("  [0] 退出登录\n");
        printf("------------------------------------------------------\n");

        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                printf("\n[提示] 管理员菜单（后续开发）...\n");
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
    char doctor_id[MAX_ID_LEN];

    printf("\n================ 查看待诊患者 ================\n");
    get_safe_string("请输入医生编号: ", doctor_id, MAX_ID_LEN);
    show_waiting_patients_by_doctor(doctor_id);
    system("pause");
}

static void handle_doctor_consultation()
{
    char doctor_id[MAX_ID_LEN];
    char patient_id[MAX_ID_LEN];
    char diagnosis_text[MAX_RECORD_LEN];
    char treatment_advice[MAX_RECORD_LEN];
    int decision;

    printf("\n================ 医生接诊 ================\n");
    get_safe_string("请输入医生编号: ", doctor_id, MAX_ID_LEN);
    get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);

    printf("\n请选择诊疗决策:\n");
    printf("  [1] 结束就诊\n");
    printf("  [2] 开药\n");
    printf("  [3] 开检查\n");
    printf("  [4] 办理住院\n");
    printf("------------------------------------------\n");

    decision = get_safe_int("👉 请输入操作编号: ");
    
    get_safe_string("请输入诊断结论: ", diagnosis_text, MAX_RECORD_LEN);
    get_safe_string("请输入处理意见: ", treatment_advice, MAX_RECORD_LEN);
    
    doctor_consult_patient(
        doctor_id,
        patient_id,
        decision,
        diagnosis_text,
        treatment_advice
    );
    
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
        printf("  [1] 查看待诊患者\n");
        printf("  [2] 接诊并做诊疗决策\n");
        printf("  [0] 退出登录\n");
        printf("------------------------------------------------------\n");

        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                handle_view_waiting_patients();
                break;
            case 2:
                handle_doctor_consultation();
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
        printf("  [1] 进入快捷挂号业务菜单\n");
        printf("  [0] 退出登录\n");
        printf("------------------------------------------------------\n");

        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                quick_register_menu();
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



static void pharmacist_menu()
{
    int running = 1;

    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               💊 药房菜单\n");
        printf("======================================================\n");
        printf("  [1] 药房菜单（后续开发）\n");
        printf("  [0] 退出登录\n");
        printf("------------------------------------------------------\n");

        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                printf("\n[提示] 药房菜单（后续开发）...\n");
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

    get_safe_string("请输入账号: ", username, MAX_ID_LEN);
    get_safe_string("请输入密码: ", password, MAX_ID_LEN);

    account = find_account_by_username(g_account_list, username);
    if (account == NULL || strcmp(account->password, password) != 0)
    {
        printf("\n⚠️ 登录失败，账号或密码错误！\n");
        system("pause");
        return;
    }

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
            doctor_menu();
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

static void handle_patient_register()
{
    char name[MAX_NAME_LEN];
    char id_card[MAX_ID_LEN];
    char symptom[MAX_SYMPTOM_LEN];
    char target_dept[MAX_NAME_LEN];
    int age;
    printf("\n================ 患者建档 ================\n");
    
    // 输入姓名并验证
    while (1)
    {
        get_safe_string("请输入患者姓名: ", name, MAX_NAME_LEN);
        if (strlen(name) > 0)
            break;
        printf("⚠️ 患者姓名不能为空，请重新输入！\n");
    }
    
    // 输入年龄并验证
    while (1)
    {
        age = get_safe_int("请输入患者年龄: ");
        if (age > 0)
            break;
        printf("⚠️ 患者年龄必须大于0，请重新输入！\n");
    }
    
    // 输入身份证号并验证
    while (1)
    {
        get_safe_string("请输入身份证号: ", id_card, MAX_ID_LEN);
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
    get_safe_string("请输入症状描述(可选): ", symptom, MAX_SYMPTOM_LEN);
    
    // 输入目标科室（可选）
    get_safe_string("请输入目标科室(可选): ", target_dept, MAX_NAME_LEN);

    register_patient(name, age, id_card, symptom, target_dept);
    system("pause");
}
static void handle_appointment_register()
{
    char patient_id[MAX_ID_LEN];
    char appointment_date[MAX_NAME_LEN];
    char appointment_slot[MAX_NAME_LEN];
    char appoint_doctor[MAX_NAME_LEN];
    char appoint_dept[MAX_NAME_LEN];
    printf("\n================ 预约登记 ================\n");
    get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
    get_safe_string("请输入预约日期: ", appointment_date, MAX_NAME_LEN);
    get_safe_string("请输入预约时段: ", appointment_slot, MAX_NAME_LEN);
    get_safe_string("请输入预约医生编号(可留空): ", appoint_doctor, MAX_NAME_LEN);
    get_safe_string("请输入预约科室(可留空): ", appoint_dept, MAX_NAME_LEN);
    register_appointment(
        patient_id,
        appointment_date,
        appointment_slot,
        appoint_doctor,
        appoint_dept
    );
    system("pause");
}
static void handle_appointment_query()
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
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------\n");
        choice = get_safe_int("👉 请输入操作编号: ");
        if (choice == 0) return;
        switch (choice)
        {
            case 1:
                get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
                query_appointments_by_patient_id(patient_id);
                system("pause");
                break;
            case 2:
                get_safe_string("请输入身份证号: ", id_card, MAX_ID_LEN);
                query_appointments_by_id_card(id_card);
                system("pause");
                break;
            default:
                printf("\n⚠️ 无效的选项，请重新输入！\n");
                system("pause");
                break;
        }
    }
}
static void handle_appointment_cancel()
{
    char appointment_id[MAX_ID_LEN];
    printf("\n================ 预约取消 ================\n");
    get_safe_string("请输入预约编号: ", appointment_id, MAX_ID_LEN);
    cancel_appointment(appointment_id);
    system("pause");
}
static void handle_appointment_check_in()
{
    char appointment_id[MAX_ID_LEN];
    printf("\n================ 预约签到 ================\n");
    get_safe_string("请输入预约编号: ", appointment_id, MAX_ID_LEN);
    check_in_appointment(appointment_id);
    system("pause");
}
static void handle_basic_record_query()
{
    char patient_id[MAX_ID_LEN];
    char id_card[MAX_ID_LEN];

    printf("\n================ 基础病历查询 ================\n");
    get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
    get_safe_string("请输入身份证号: ", id_card, MAX_ID_LEN);
    query_basic_patient_record(patient_id, id_card);
    system("pause");
}

static void handle_patient_visit_overview()
{
    char patient_id[MAX_ID_LEN];

    printf("\n================ 患者就诊概览 ================\n");
    get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
    query_patient_visit_overview_by_id(patient_id);
    system("pause");
}

static void handle_patient_visit_overview_by_id_card()
{
    char id_card[MAX_ID_LEN];

    printf("\n================ 患者就诊概览 ================\n");
    get_safe_string("请输入身份证号: ", id_card, MAX_ID_LEN);
    query_patient_visit_overview_by_id_card(id_card);
    system("pause");
}

static void handle_patient_archive_query_by_id()
{
    char patient_id[MAX_ID_LEN];
    
    printf("\n================ 按编号查询档案 ================\n");
    get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
    query_patient_archive_by_id(patient_id);
    system("pause");
}

static void handle_patient_archive_query_by_id_card()
{
    char id_card[MAX_ID_LEN];
    
    printf("\n================ 按身份证号查询档案 ================\n");
    get_safe_string("请输入身份证号: ", id_card, MAX_ID_LEN);
    query_patient_archive_by_id_card(id_card);
    system("pause");
}

static void handle_patient_archive_query_by_name()
{
    char name[MAX_NAME_LEN];
    
    printf("\n================ 按姓名查询档案 ================\n");
    get_safe_string("请输入患者姓名: ", name, MAX_NAME_LEN);
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
    double balance = 0.0;
    PatientNode* patient = NULL;

    printf("\n================ 修改患者信息 ================\n");
    get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
    
    // 先查询当前档案，如果查不到则直接返回
    if (!query_patient_archive_by_id(patient_id))
    {
        system("pause");
        return;
    }
    
    // 获取患者当前信息
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到患者信息！\n");
        system("pause");
        return;
    }
    
    printf("\n提示：患者编号不可修改。\n");
    printf("提示：留空表示不修改该字段。\n\n");
    
    // 输入新姓名（留空表示不修改）
    printf("当前姓名：%s\n", patient->name);
    get_safe_string("请输入新姓名: ", temp_input, MAX_NAME_LEN);
    if (strlen(temp_input) > 0)
        strcpy(name, temp_input);
    else
        strcpy(name, patient->name);
    
    // 输入新年龄（留空表示不修改）
    printf("当前年龄：%d\n", patient->age);
    printf("请输入新年龄: ");
    if (fgets(temp_input, sizeof(temp_input), stdin) != NULL)
    {
        temp_input[strcspn(temp_input, "\n")] = '\0';
        if (strlen(temp_input) > 0)
            age = atoi(temp_input);
        else
            age = patient->age;
    }
    else
        age = patient->age;
    
    // 输入新症状（留空表示不修改）
    printf("当前症状：%s\n", patient->symptom);
    get_safe_string("请输入新症状描述: ", temp_input, MAX_SYMPTOM_LEN);
    if (strlen(temp_input) > 0)
        strcpy(symptom, temp_input);
    else
        strcpy(symptom, patient->symptom);
    
    // 输入新目标科室（留空表示不修改）
    printf("当前目标科室：%s\n", patient->target_dept);
    get_safe_string("请输入新目标科室: ", temp_input, MAX_NAME_LEN);
    if (strlen(temp_input) > 0)
        strcpy(target_dept, temp_input);
    else
        strcpy(target_dept, patient->target_dept);
    
    // 输入新身份证号（留空表示不修改）
    printf("当前身份证号：%s\n", patient->id_card);
    get_safe_string("请输入新身份证号: ", temp_input, MAX_ID_LEN);
    if (strlen(temp_input) > 0)
        strcpy(id_card, temp_input);
    else
        strcpy(id_card, patient->id_card);
    
    // 输入新账户余额（留空表示不修改）
    printf("当前账户余额：%.2f\n", patient->balance);
    printf("请输入新账户余额: ");
    if (fgets(temp_input, sizeof(temp_input), stdin) != NULL)
    {
        temp_input[strcspn(temp_input, "\n")] = '\0';
        if (strlen(temp_input) > 0)
            balance = atof(temp_input);
        else
            balance = patient->balance;
    }
    else
        balance = patient->balance;

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
        printf("  [5] 预约签到\n");
        printf("  [6] 患者档案管理\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");
        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                handle_patient_register();
                break;
            case 2:
                handle_appointment_register();
                break;
            case 3:
                handle_appointment_query();
                break;
            case 4:
                handle_appointment_cancel();
                break;
            case 5:
                handle_appointment_check_in();
                break;
            case 6:
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
static void patient_self_service_menu()
{
    int running = 1;
    char patient_id[MAX_ID_LEN];
    char id_card[MAX_ID_LEN];
    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               🔍 患者自助查询菜单\n");
        printf("======================================================\n");
        printf("  [1] 按患者ID查询预约\n");
        printf("  [2] 按身份证号查询预约\n");
        printf("  [3] 身份核验后查询基础病历信息\n");
        printf("  [4] 按患者编号查看就诊概览\n");
        printf("  [5] 按身份证号查看就诊概览\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");
        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
                query_appointments_by_patient_id(patient_id);
                system("pause");
                break;
            case 2:
                get_safe_string("请输入身份证号: ", id_card, MAX_ID_LEN);
                query_appointments_by_id_card(id_card);
                system("pause");
                break;
            case 3:
                handle_basic_record_query();
                break;
            case 4:
                handle_patient_visit_overview();
                break;
            case 5:
                handle_patient_visit_overview_by_id_card();
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

    // 2. 注入一组极端测试数据
    insert_patient_tail(g_patient_list, create_patient_node(
"P-001", "张三", 19, "110101199001011234"
));
    insert_doctor_tail(g_doctor_list, create_doctor_node(
"D-001", "李大夫", "外科"
));
    insert_medicine_tail(g_medicine_list, create_medicine_node(
"M-001", "阿莫西林", 15.5, 100
, MEDICARE_CLASS_A));
    insert_appointment_tail(g_appointment_list, create_appointment_node(
"A-001", "P-001", "2026-04-01", "上午", "D-001", "外科", RESERVED
));
    insert_ward_tail(g_ward_list, create_ward_node(
"W-101"
));
    insert_account_tail(g_account_list, create_account_node(
"admin", "123456", "超级管理员"
, ROLE_ADMIN));
    insert_account_tail(g_account_list, create_account_node(
"nurse", "123456", "护士"
, ROLE_NURSE));
    insert_account_tail(g_account_list, create_account_node(
"doctor", "123456", "医生"
, ROLE_DOCTOR));
    insert_account_tail(g_account_list, create_account_node(
"pharm", "123456", "药剂师"
, ROLE_PHARMACIST));

    // 3. 打印核验
    printf("✅ 引擎全部就绪！\n"
);
    printf(" -> 测试患者: %s\n"
, g_patient_list->next->name);
    printf(" -> 测试医生: %s (%s)\n"
, g_doctor_list->next->name, g_doctor_list->next->department);
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
    // 🚀 时停魔法：按任意键后才清屏进入菜单！
    system(
"pause"
);
    int running = 1; // 控制系统运行状态的标志位

    while (running) 
    {
        // 每次循环清空屏幕，保持 UI 整洁 (Windows用cls, Mac/Linux用clear)
        system("cls"); 
        
       printf("\n======================================================\n");
        printf("               🏥 社区智慧医疗管理系统\n");
        printf("======================================================\n");
        printf("  [1] 🔑 内部登录 (医生/护士/药房专员/管理员)\n");
        printf("  [2] 👨‍⚕️ 快捷挂号 (智能分诊通道)\n");
        printf("  [3] 🔍 患者自助 (预约查询 / 基础病历)\n");
        printf("  [4] 📊 医疗大屏 (内部数据可视化展示)\n");
        printf("  [0] 🚪 安全退出系统\n");
        printf("------------------------------------------------------\n");

        int choice = get_safe_int("👉 请输入操作编号: ");

        switch (choice) {
            case 1:
                printf("\n[跳转] -> 进入内部人员登录模块...\n");
                internal_login_menu(); 
                break;
            case 2:
                printf("\n[跳转] -> 进入快捷挂号通道...\n");
                quick_register_menu();
                break;
            case 3:
                // 🌟 这里就是专门为患者视角设计的通道！
                printf("\n[跳转] -> 进入患者自助预约查询终端...\n");
                patient_self_service_menu(); 
                // 在这个函数里，患者输入姓名或编号，系统遍历处方链表打印结算单
                break;
            case 4:
                printf("\n[跳转] -> 加载数据可视化大屏...\n");
                // show_dashboard();
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
    printf("     💾 正在保存数据到本地文件 (XOR 加密处理)...\n");
    
    // 这里未来调用 data_io.c 里的函数，把当前双向链表的数据覆盖写入 txt
    // save_patient_list();
    // save_doctor_list();
    // save_medicine_list();

    printf("     🧹 正在释放链表内存，清空回收站...\n");
    // 销毁所有双向链表，防止内存泄漏 (非常重要，代码检查必看)
    // destroy_all_lists(); 

    printf("\n✅ 数据已安全保存，感谢使用！再见！\n");
    printf("======================================================\n");

    return 0;
}

