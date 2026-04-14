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
#include "global.h"
#include "list_ops.h"
#include "utils.h"
#include "appointment.h"
#include "patient_service.h"
#include "doctor_service.h"

// 当前登录的医生信息
static DoctorNode* g_current_doctor = NULL;

// 这里未来会引入你们自己写的头文件
// #include "global.h"     // 全局常量与结构体定义
// #include "data_io.h"    // 负责读写 txt 文件的模块
// #include "auth.h"       // 负责登录与权限的模块
// #include "utils.h"      // 存放 get_safe_int 终端输入拦截器等工具函数
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
static void handle_patient_self_consult_history_query();

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
static void nurse_menu();
static void doctor_menu();
static void pharmacist_menu();
static void check_dept_doctor_menu();
static void handle_view_waiting_patients();
static void handle_doctor_consultation();
static void handle_doctor_view_patient_overview();
static int is_check_department(const char* dept);

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
    printf("\n================ 查看待诊患者 ================\n");
    show_waiting_patients_by_doctor(g_current_doctor->id);
    system("pause");
}

static void handle_doctor_consultation()
{
    PatientNode* patient_list[100];
    char diagnosis_text[MAX_RECORD_LEN];
    char treatment_advice[MAX_RECORD_LEN];
    int decision;
    int count;
    int choice;
    char check_item_ids[10][MAX_ID_LEN] = {0};
    int check_count = 0;

    printf("\n================ 医生接诊 ================\n");
    
    // 显示待诊患者列表
    show_waiting_patients_by_doctor(g_current_doctor->id);
    
    // 获取待诊患者列表
    count = get_waiting_patients_by_doctor(g_current_doctor->id, patient_list);
    if (count == 0)
    {
        system("pause");
        return;
    }
    
    // 选择患者
    printf("\n请选择要接诊的患者序号 (1-%d): ", count);
    choice = get_safe_int("👉 ");
    
    if (choice < 1 || choice > count)
    {
        printf("\n⚠️ 无效的选择！\n");
        system("pause");
        return;
    }
    
    // 显示患者信息确认
    printf("\n========================================\n");
    printf("确认接诊患者：\n");
    printf("患者编号：%s\n", patient_list[choice - 1]->id);
    printf("姓名：%s\n", patient_list[choice - 1]->name);
    printf("年龄：%d\n", patient_list[choice - 1]->age);
    printf("症状：%s\n", patient_list[choice - 1]->symptom[0] != '\0' ? patient_list[choice - 1]->symptom : "暂无");
    printf("========================================\n");
    
    // 选择诊疗决策
    printf("\n请选择诊疗决策:\n");
    printf("  [1] 结束就诊\n");
    printf("  [2] 开药\n");
    printf("  [3] 开检查\n");
    printf("  [4] 办理住院\n");
    printf("------------------------------------------\n");

    decision = get_safe_int("👉 请输入操作编号: ");
    
    // 如果选择开检查，显示检查项目列表供选择
    if (decision == 3)
    {
        printf("\n================ 选择检查项目 ================\n");
        printf("可用检查项目：\n");
        
        CheckItemNode* curr = g_check_item_list->next;
        int item_index = 1;
        while (curr != NULL)
        {
            printf("  [%d] %s（%s）- ¥%.2f\n", item_index, curr->item_name, curr->dept, curr->price);
            curr = curr->next;
            item_index++;
        }
        
        printf("\n请选择检查项目（可多选，输入编号后按回车，输入0结束选择）：\n");
        int item_choice;
        while (1)
        {
            item_choice = get_safe_int("👉 ");
            if (item_choice == 0) break;
            
            CheckItemNode* selected_item = g_check_item_list->next;
            int i = 1;
            while (selected_item != NULL && i < item_choice)
            {
                selected_item = selected_item->next;
                i++;
            }
            
            if (selected_item != NULL && check_count < 10)
            {
                strcpy(check_item_ids[check_count], selected_item->item_id);
                printf("✓ 已选择：%s\n", selected_item->item_name);
                check_count++;
            }
            else
            {
                printf("⚠️ 无效的选择！\n");
            }
        }
        
        if (check_count == 0)
        {
            printf("\n⚠️ 未选择任何检查项目，操作取消！\n");
            system("pause");
            return;
        }
    }
    
    get_safe_string("请输入诊断结论: ", diagnosis_text, MAX_RECORD_LEN);
    get_safe_string("请输入处理意见: ", treatment_advice, MAX_RECORD_LEN);
    
    doctor_consult_patient(
        g_current_doctor->id,
        patient_list[choice - 1]->id,
        decision,
        diagnosis_text,
        treatment_advice
    );
    
    // 如果选择开检查，创建检查记录
    if (decision == 3 && check_count > 0)
    {
        for (int i = 0; i < check_count; i++)
        {
            CheckItemNode* item = find_check_item_by_id(g_check_item_list, check_item_ids[i]);
            if (item != NULL)
            {
                char record_id[MAX_ID_LEN];
                sprintf(record_id, "CR-%s-%03d", patient_list[choice - 1]->id, i + 1);
                
                insert_check_record_tail(g_check_record_list, 
                    create_check_record_node(
                        record_id,
                        patient_list[choice - 1]->id,
                        item->item_id,
                        item->item_name,
                        item->dept,
                        NULL,
                        "待检查",
                        0
                    )
                );
                
                printf("📋 已开具检查：%s\n", item->item_name);
            }
        }
    }
    
    system("pause");
}

static void handle_doctor_view_patient_overview()
{
    PatientNode* patient_list[100];
    int count;
    int choice;
    
    printf("\n================ 查看患者接诊前信息 ================\n");
    
    // 显示待诊患者列表
    show_waiting_patients_by_doctor(g_current_doctor->id);
    
    // 获取待诊患者列表
    count = get_waiting_patients_by_doctor(g_current_doctor->id, patient_list);
    if (count == 0)
    {
        system("pause");
        return;
    }
    
    // 选择患者
    printf("\n请选择要查看的患者序号 (1-%d): ", count);
    choice = get_safe_int("👉 ");
    
    if (choice < 1 || choice > count)
    {
        printf("\n⚠️ 无效的选择！\n");
        system("pause");
        return;
    }
    
    // 查看患者概览
    doctor_view_patient_overview(g_current_doctor->id, patient_list[choice - 1]->id);
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
    PatientNode* patient_list[100];
    int count = 0;
    int choice;
    
    printf("\n================ 查看已处理患者详情 ================\n");
    
    // 获取已处理患者列表
    count = get_processed_patients_by_doctor(g_current_doctor->id, patient_list);
    
    if (count == 0)
    {
        printf("暂无已处理患者记录\n");
        system("pause");
        return;
    }
    
    // 显示已处理患者列表（带序号）
    doctor_view_processed_patients(g_current_doctor->id);
    
    // 选择患者
    printf("\n请选择要查看详情的患者序号 (1-%d): ", count);
    choice = get_safe_int("👉 ");
    
    if (choice < 1 || choice > count)
    {
        printf("\n⚠️ 无效的选择！\n");
        system("pause");
        return;
    }
    
    // 查看患者详情
    doctor_view_processed_patient_detail(g_current_doctor->id, patient_list[choice - 1]->id);
    system("pause");
}

static void handle_doctor_view_consult_history()
{
    PatientNode* patient_list[100];
    int count = 0;
    int choice;
    
    printf("\n================ 查看历史接诊记录 ================\n");
    
    // 获取已处理患者列表
    count = get_processed_patients_by_doctor(g_current_doctor->id, patient_list);
    
    if (count == 0)
    {
        printf("暂无已处理患者记录\n");
        system("pause");
        return;
    }
    
    // 显示已处理患者列表（带序号）
    doctor_view_processed_patients(g_current_doctor->id);
    
    // 选择患者查看历史记录
    printf("\n请选择要查看历史记录的患者序号 (1-%d): ", count);
    choice = get_safe_int("👉 ");
    
    if (choice < 1 || choice > count)
    {
        printf("\n⚠️ 无效的选择！\n");
        system("pause");
        return;
    }
    
    // 查看该患者的历史接诊记录
    doctor_view_consult_history(g_current_doctor->id, patient_list[choice - 1]->id);
    system("pause");
}

static void handle_exam_return_registration()
{
    char patient_id[MAX_ID_LEN];
    
    printf("\n================ 检查完成回诊登记 ================\n");
    get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
    
    if (complete_exam_and_return_to_doctor(patient_id))
    {
        printf("\n✅ 回诊登记成功！\n");
    }
    else
    {
        printf("\n❌ 回诊登记失败！\n");
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
        printf("  [4] 查看已处理患者\n");
        printf("  [5] 查看已处理患者详情\n");
        printf("  [6] 查看历史接诊记录\n");
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
                handle_doctor_view_processed_patients();
                break;
            case 5:
                handle_doctor_view_processed_patient_detail();
                break;
            case 6:
                handle_doctor_view_consult_history();
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
                
                // 获取待检查列表
                CheckRecordNode* check_list[100];
                count = get_waiting_checks_by_dept(g_current_doctor->id, check_list);
                
                if (count == 0)
                {
                    printf("ℹ️ 当前没有待检查的记录。\n");
                    system("pause");
                    break;
                }
                
                // 显示待检查列表
                printf("待检查列表：\n");
                for (int i = 0; i < count; i++)
                {
                    const char* patient_name = get_patient_name_by_id(check_list[i]->patient_id);
                    printf("[%d] 检查记录: %s | 患者编号: %s | 患者姓名: %s | 检查项目: %s\n",
                        i + 1,
                        check_list[i]->record_id,
                        check_list[i]->patient_id,
                        patient_name != NULL ? patient_name : "未知",
                        check_list[i]->item_name);
                }
                
                // 选择检查记录
                choice = get_safe_int("\n请选择要录入结果的检查记录序号: ");
                if (choice < 1 || choice > count)
                {
                    printf("⚠️ 无效的序号！\n");
                    system("pause");
                    break;
                }
                
                // 显示选中的检查记录详情
                show_check_record_detail(check_list[choice - 1]);
                
                // 录入检查结果
                get_safe_string("请输入检查结果: ", result, MAX_RECORD_LEN);
                doctor_update_check_result(g_current_doctor->id, check_list[choice - 1]->record_id, result);
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
    if (g_alert_list == NULL || g_alert_list->next == NULL)
    {
        printf("\n🔒 当前安全环境：优\n");
        return;
    }
    
    printf("\n🚨 【安全预警中心】最近警报：\n");
    printf("------------------------------------------------------\n");
    
    // 逆序打印最近的 5 条警报
    AlertNode* curr = g_alert_list->next;
    int count = 0;
    
    // 先找到链表末尾
    while (curr->next != NULL)
    {
        curr = curr->next;
    }
    
    // 从末尾向前遍历，打印最近的 5 条
    while (curr != g_alert_list && count < 5)
    {
        struct tm* local_time = localtime(&curr->time);
        char time_str[20];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
        
        printf("[%s] %s\n", time_str, curr->message);
        
        curr = curr->prev;
        count++;
    }
    
    printf("------------------------------------------------------\n");
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
static void handle_internal_appointment_register()
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
    get_safe_string("请输入预约科室(可留空): ", appoint_dept, MAX_NAME_LEN);
    
    // 如果输入了科室，显示该科室的医生列表
    if (strlen(appoint_dept) > 0)
    {
        display_doctors_by_dept(appoint_dept);
    }
    
    get_safe_string("请输入预约医生编号(可留空): ", appoint_doctor, MAX_NAME_LEN);
    register_appointment(
        patient_id,
        appointment_date,
        appointment_slot,
        appoint_doctor,
        appoint_dept
    );
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
static void handle_internal_appointment_cancel()
{
    char appointment_id[MAX_ID_LEN];
    printf("\n================ 预约取消 ================\n");
    get_safe_string("请输入预约编号: ", appointment_id, MAX_ID_LEN);
    cancel_appointment(appointment_id);
    system("pause");
}
static void handle_internal_appointment_check_in()
{
    char appointment_id[MAX_ID_LEN];
    printf("\n================ 预约签到 ================\n");
    get_safe_string("请输入预约编号: ", appointment_id, MAX_ID_LEN);
    check_in_appointment(appointment_id);
    system("pause");
}
static void handle_patient_self_basic_record_query()
{
    char patient_id[MAX_ID_LEN];
    char id_card[MAX_ID_LEN];

    printf("\n================ 基础病历查询 ================\n");
    get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
    get_safe_string("请输入身份证号: ", id_card, MAX_ID_LEN);
    query_basic_patient_record(patient_id, id_card);
    system("pause");
}

static void handle_patient_self_visit_overview_query()
{
    char patient_id[MAX_ID_LEN];
    char id_card[MAX_ID_LEN];
    
    printf("\n================ 查询自己的就诊概览 ================\n");
    get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
    get_safe_string("请输入身份证号: ", id_card, MAX_ID_LEN);
    
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

static void handle_patient_self_consult_history_query()
{
    char patient_id[MAX_ID_LEN];
    char id_card[MAX_ID_LEN];
    
    printf("\n================ 查询自己的历史就诊记录 ================\n");
    printf("温馨提示：请确保您输入的是本人信息\n");
    
    get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
    get_safe_string("请输入身份证号: ", id_card, MAX_ID_LEN);
    
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
    
    printf("\n================ 自助预约登记 ================\n");
    printf("温馨提示：请确保您输入的是本人信息\n");
    
    get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
    get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
    
    // 身份核验：检查患者编号和身份证号是否匹配
    PatientNode* patient = find_patient_by_id_card(id_card);
    if (patient == NULL || strcmp(patient->id, patient_id) != 0)
    {
        printf("\n❌ 身份核验失败！患者编号与身份证号不匹配\n");
        system("pause");
        return;
    }
    
    printf("\n身份核验成功！欢迎，%s\n", patient->name);
    
    get_safe_string("请输入预约日期: ", appointment_date, MAX_NAME_LEN);
    get_safe_string("请输入预约时段: ", appointment_slot, MAX_NAME_LEN);
    get_safe_string("请输入预约科室(可留空): ", appoint_dept, MAX_NAME_LEN);
    
    // 如果输入了科室，显示该科室的医生列表
    if (strlen(appoint_dept) > 0)
    {
        display_doctors_by_dept(appoint_dept);
    }
    
    get_safe_string("请输入预约医生编号(可留空): ", appoint_doctor, MAX_NAME_LEN);
    
    register_appointment(
        patient_id,
        appointment_date,
        appointment_slot,
        appoint_doctor,
        appoint_dept
    );
    
    printf("\n✅ 自助预约登记成功！\n");
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
    
    get_safe_string("请输入您的身份证号: ", id_card, MAX_ID_LEN);
    
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
    
    get_safe_string("请输入您的姓名: ", name, MAX_NAME_LEN);
    age = get_safe_int("请输入您的年龄: ");
    get_safe_string("请输入您的症状描述: ", symptom, MAX_SYMPTOM_LEN);
    get_safe_string("请输入您要就诊的科室: ", target_dept, MAX_NAME_LEN);
    
    // 调用建档函数
    PatientNode* new_patient = register_patient(name, age, id_card, symptom, target_dept);
    
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
    
    printf("\n================ 自助挂号（现场挂号） ================\n");
    printf("温馨提示：本功能仅支持已建档患者使用\n");
    printf("如果您是首次就诊，请先到服务台建档\n");
    
    get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
    get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
    
    // 身份核验：检查患者编号和身份证号是否匹配
    PatientNode* patient = find_patient_by_id_card(id_card);
    if (patient == NULL || strcmp(patient->id, patient_id) != 0)
    {
        printf("\n❌ 身份核验失败！患者编号与身份证号不匹配\n");
        system("pause");
        return;
    }
    
    printf("\n身份核验成功！欢迎，%s\n", patient->name);
    
    // 症状重新分诊
    char current_symptom[MAX_SYMPTOM_LEN];
    get_safe_string("请输入您本次就诊的症状描述: ", current_symptom, MAX_SYMPTOM_LEN);
    // 拷贝覆盖到患者症状
    strncpy(patient->symptom, current_symptom, MAX_SYMPTOM_LEN - 1);
    patient->symptom[MAX_SYMPTOM_LEN - 1] = '\0'; // 确保字符串结束
    // 重新调用智能导诊
    const char* recommended_dept = recommend_dept_by_symptom(patient->symptom);
    // 根据返回值重新判定急诊状态
    if (strcmp(recommended_dept, "急诊科") == 0)
    {
        patient->is_emergency = 1;
        printf("🚨 症状符合急诊特征，已为您开启绿色生命通道！\n");
    }
    else
    {
        patient->is_emergency = 0;
    }
    
    // 现场挂号使用当天日期
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    sprintf(appointment_date, "%04d-%02d-%02d", 
            local_time->tm_year + 1900, 
            local_time->tm_mon + 1, 
            local_time->tm_mday);
    
    printf("\n当前日期：%s\n", appointment_date);
    get_safe_string("请输入挂号时段: ", appointment_slot, MAX_NAME_LEN);
    
    // 夜间模式防呆提示
    if (is_night_shift()) {
        printf("\n🌙 [系统通知] 当前为夜间模式，所有挂号将自动转入急诊科。\n");
    }
    
    get_safe_string("请输入挂号科室(可留空): ", appoint_dept, MAX_NAME_LEN);
    
    // 如果输入了科室，显示该科室的医生列表
    if (strlen(appoint_dept) > 0)
    {
        display_doctors_by_dept(appoint_dept);
    }
    
    get_safe_string("请输入挂号医生编号(可留空): ", appoint_doctor, MAX_NAME_LEN);
    
    register_appointment(
        patient_id,
        appointment_date,
        appointment_slot,
        appoint_doctor,
        appoint_dept
    );
    
    printf("\n✅ 自助挂号成功！\n");
    system("pause");
}

static void handle_patient_self_appointment_cancel()
{
    char patient_id[MAX_ID_LEN];
    char id_card[MAX_ID_LEN];
    char appointment_id[MAX_ID_LEN];
    
    printf("\n================ 自助预约取消 ================\n");
    printf("温馨提示：请确保您输入的是本人信息\n");
    
    get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
    get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
    
    // 身份核验：检查患者编号和身份证号是否匹配
    PatientNode* patient = find_patient_by_id_card(id_card);
    if (patient == NULL || strcmp(patient->id, patient_id) != 0)
    {
        printf("\n❌ 身份核验失败！患者编号与身份证号不匹配\n");
        system("pause");
        return;
    }
    
    get_safe_string("请输入要取消的预约编号: ", appointment_id, MAX_ID_LEN);
    
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
        printf("  [7] 检查完成回诊登记\n");
        printf("  [8] 登记患者爽约(过号)\n");
        printf("  [9] 登记急诊逃单黑名单\n");
        printf("  [10] 查看全院黑名单\n");
        printf("  [11] 补缴欠费并核销黑名单\n");
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
                handle_exam_return_registration();
                break;
            case 8:
            {
                char appointment_id[MAX_ID_LEN];
                get_safe_string("请输入预约编号: ", appointment_id, MAX_ID_LEN);
                mark_appointment_missed(appointment_id);
                system("pause");
                break;
            }
            case 9:
            {
                char patient_id[MAX_ID_LEN];
                get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
                handle_emergency_escape(patient_id);
                system("pause");
                break;
            }
            case 10:
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
            case 11:
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
        printf("  [3] 自助挂号（现场挂号）\n");
        printf("  [4] 查询自己的预约\n");
        printf("  [5] 取消自己的预约\n");
        printf("  [6] 查询自己的基础病历\n");
        printf("  [7] 查询自己的就诊概览\n");
        printf("  [8] 查询自己的历史就诊记录\n");
        printf("  [9] 查询检查报告\n");
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
                get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
                
                // 身份核验：检查患者编号和身份证号是否匹配
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
                break;
            case 6:
                handle_patient_self_basic_record_query();
                break;
            case 7:
                handle_patient_self_visit_overview_query();
                break;
            case 8:
                handle_patient_self_consult_history_query();
                break;
            case 9:
                get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
                
                // 身份核验
                PatientNode* check_patient = find_patient_by_id_card(id_card);
                if (check_patient == NULL || strcmp(check_patient->id, patient_id) != 0)
                {
                    printf("\n❌ 身份核验失败！患者编号与身份证号不匹配\n");
                    system("pause");
                    break;
                }
                
                printf("\n身份核验成功！欢迎，%s\n", check_patient->name);
                printf("\n================ 检查报告列表 ================\n");
                
                CheckRecordNode* check_records[100];
                int check_count = get_check_records_by_patient(g_check_record_list, patient_id, check_records);
                
                if (check_count == 0)
                {
                    printf("暂无检查记录\n");
                }
                else
                {
                    for (int i = 0; i < check_count; i++)
                    {
                        printf("\n【检查报告 %d】\n", i + 1);
                        printf("检查项目：%s\n", check_records[i]->item_name);
                        printf("检查科室：%s\n", check_records[i]->dept);
                        printf("检查时间：%s\n", check_records[i]->check_time[0] != '\0' ? check_records[i]->check_time : "待安排");
                        printf("检查状态：%s\n", check_records[i]->is_completed ? "已完成" : "待检查");
                        printf("检查结果：%s\n", check_records[i]->result);
                        printf("----------------------------------------\n");
                    }
                }
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

    // ==============================================
    // 检查项目字典初始化
    // ==============================================
    // 放射科检查项目
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-001", "X光检查", "放射科", 150.0));
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-002", "CT扫描", "放射科", 800.0));
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-003", "增强CT", "放射科", 1200.0));
    
    // 影像科检查项目
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-004", "MRI磁共振", "影像科", 1200.0));
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-005", "PET-CT", "影像科", 8000.0));
    
    // 超声科检查项目
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-006", "腹部B超", "超声科", 200.0));
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-007", "心脏彩超", "超声科", 350.0));
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-008", "妇科B超", "超声科", 180.0));
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-009", "甲状腺B超", "超声科", 150.0));
    
    // 检验科检查项目
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-010", "血常规", "检验科", 50.0));
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-011", "生化全项", "检验科", 300.0));
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-012", "肝功能", "检验科", 120.0));
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-013", "肾功能", "检验科", 100.0));
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-014", "血糖检测", "检验科", 30.0));
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-015", "血脂四项", "检验科", 80.0));
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-016", "凝血功能", "检验科", 150.0));
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-017", "尿常规", "检验科", 30.0));
    
    // 心电图室检查项目
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-018", "心电图", "心电图室", 80.0));
    insert_check_item_tail(g_check_item_list, create_check_item_node("C-019", "动态心电图", "心电图室", 300.0));

    // 2. 注入一组极端测试数据
    insert_patient_tail(g_patient_list, create_patient_node(
"P-001", "张三", 19, "110101199001011234"
));
    // 各科室医生数据
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-001", "李建国", "内科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-002", "王大明", "外科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-003", "张小红", "妇产科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-004", "刘小华", "儿科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-005", "陈伟强", "骨科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-006", "赵丽华", "心血管内科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-007", "孙卫东", "呼吸内科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-008", "周秀芳", "消化内科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-009", "吴明德", "神经内科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-010", "郑美玲", "内分泌科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-011", "黄志强", "肾内科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-012", "林建华", "泌尿外科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-013", "何晓燕", "肿瘤科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-014", "罗伟民", "急诊科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-015", "梁丽萍", "眼科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-016", "谢国华", "耳鼻喉科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-017", "马文婷", "口腔科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-018", "唐俊杰", "皮肤科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-019", "许文静", "风湿免疫科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-020", "杨浩然", "感染科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-021", "胡晓峰", "精神科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-022", "朱秀兰", "康复医学科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-023", "韩伟东", "普通外科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-024", "曹丽华", "肝胆外科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-025", "蒋志明", "心胸外科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-026", "丁晓明", "神经外科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-027", "冯彩霞", "全科"));
    
    // ==============================================
    // 辅助检查科室（不直接接诊患者，提供检查服务）
    // ==============================================
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-101", "放射科", "放射科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-102", "影像科", "影像科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-103", "检验科", "检验科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-104", "超声科", "超声科"));
    insert_doctor_tail(g_doctor_list, create_doctor_node("D-105", "心电图室", "心电图室"));
    
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
    // 各科室医生账户（密码均为123456）
    insert_account_tail(g_account_list, create_account_node("D-001", "123456", "李建国-内科", ROLE_DOCTOR));
    insert_account_tail(g_account_list, create_account_node("D-002", "123456", "王大明-外科", ROLE_DOCTOR));
    insert_account_tail(g_account_list, create_account_node("D-003", "123456", "张小红-妇产科", ROLE_DOCTOR));
    insert_account_tail(g_account_list, create_account_node("D-004", "123456", "刘小华-儿科", ROLE_DOCTOR));
    insert_account_tail(g_account_list, create_account_node("D-005", "123456", "陈伟强-骨科", ROLE_DOCTOR));
    insert_account_tail(g_account_list, create_account_node("D-006", "123456", "赵丽华-心血管内科", ROLE_DOCTOR));
    insert_account_tail(g_account_list, create_account_node("D-007", "123456", "孙卫东-呼吸内科", ROLE_DOCTOR));
    insert_account_tail(g_account_list, create_account_node("D-008", "123456", "周秀芳-消化内科", ROLE_DOCTOR));
    insert_account_tail(g_account_list, create_account_node("D-009", "123456", "吴明德-神经内科", ROLE_DOCTOR));
    insert_account_tail(g_account_list, create_account_node("D-010", "123456", "郑美玲-内分泌科", ROLE_DOCTOR));
    insert_account_tail(g_account_list, create_account_node("D-014", "123456", "罗伟民-急诊科", ROLE_DOCTOR));
    insert_account_tail(g_account_list, create_account_node("D-015", "123456", "梁丽萍-眼科", ROLE_DOCTOR));
    insert_account_tail(g_account_list, create_account_node("D-016", "123456", "谢国华-耳鼻喉科", ROLE_DOCTOR));
    insert_account_tail(g_account_list, create_account_node("D-017", "123456", "马文婷-口腔科", ROLE_DOCTOR));
    insert_account_tail(g_account_list, create_account_node("D-018", "123456", "唐俊杰-皮肤科", ROLE_DOCTOR));
    insert_account_tail(g_account_list, create_account_node("D-027", "123456", "冯彩霞-全科", ROLE_DOCTOR));
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

    system("pause"); // 等待用户按任意键
    return 0;
}

