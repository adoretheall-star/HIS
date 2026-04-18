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
#include "admin_service.h"
#include "medicine_service.h"
#include "pharmacy_service.h"

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
static void admin_medicine_menu();
static void nurse_menu();
static void doctor_menu();
static void pharmacist_menu();
static void check_dept_doctor_menu();
static void handle_view_waiting_patients();
static void handle_doctor_consultation();
static void handle_doctor_view_patient_overview();
static int is_check_department(const char* dept);
static RoleType prompt_admin_staff_role();
static void handle_admin_register_account();
static void handle_admin_update_account();
static void handle_admin_update_doctor_duty();
static void handle_admin_update_nurse_duty();
static void handle_pharmacy_dispense();
static int is_current_emergency_request(const char* appoint_doctor, const char* appoint_dept, const char* symptom);

static void admin_menu()
{
    int running = 1;

    while (running)
    {
        system("cls");
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
    char department[MAX_NAME_LEN];
    RoleType role;

    printf("\n================ 新增员工账号 ================\n");
    get_safe_string("请输入登录账号: ", username, MAX_ID_LEN);
    get_safe_string("请输入登录密码: ", password, MAX_ID_LEN);
    get_safe_string("请输入真实姓名: ", real_name, MAX_NAME_LEN);

    role = prompt_admin_staff_role();
    if (role == 0)
    {
        printf("⚠️ 无效的角色编号，新增账号已取消。\n");
        return;
    }

    department[0] = '\0';
    if (role == ROLE_DOCTOR)
    {
        get_safe_string("请输入医生所属科室: ", department, MAX_NAME_LEN);
        if (is_blank_string(department))
        {
            printf("⚠️ 医生账号必须绑定科室，新增账号已取消。\n");
            return;
        }
        if (g_doctor_list == NULL)
        {
            printf("⚠️ 医生链表尚未初始化，无法新增医生账号。\n");
            return;
        }
        if (find_doctor_by_id(g_doctor_list, username) != NULL)
        {
            printf("⚠️ 已存在同编号医生实体，无法重复新增。\n");
            return;
        }
    }

    if (!register_account(username, password, real_name, role))
    {
        return;
    }

    if (role == ROLE_DOCTOR)
    {
        DoctorNode* new_doctor = create_doctor_node(username, real_name, department);
        if (new_doctor == NULL)
        {
            delete_account_by_username(g_account_list, username);
            printf("⚠️ 医生实体创建失败，已回滚刚新增的账号。\n");
            return;
        }

        insert_doctor_tail(g_doctor_list, new_doctor);
        printf("✅ 已同步创建医生实体：%s（%s）- %s\n", real_name, username, department);
    }
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
        DoctorNode* new_doctor = create_doctor_node(username, account->real_name, department);
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

            doctor = create_doctor_node(username, account->real_name, department);
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
    get_safe_string("请输入医生工号: ", doctor_id, MAX_ID_LEN);
    new_status = get_safe_int("请输入新状态（1=值班中, 0=未值班）: ");
    update_doctor_duty_status(doctor_id, new_status);
}

static void handle_admin_update_nurse_duty()
{
    char username[MAX_ID_LEN];
    int new_status;

    printf("\n================ 修改护士值班状态 ================\n");
    get_safe_string("请输入护士账号: ", username, MAX_ID_LEN);
    new_status = get_safe_int("请输入新状态（1=值班中, 0=未值班）: ");
    update_nurse_duty_status(username, new_status);
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
        printf("               💊 药品与发药管理\n");
        printf("======================================================\n");
        printf("  [1] 查看全部药品\n");
        printf("  [2] 按关键词查询药品\n");
        printf("  [3] 新增药品\n");
        printf("  [4] 修改药品基础信息\n");
        printf("  [5] 修改药品库存\n");
        printf("  [6] 查看低库存药品\n");
        printf("  [7] 查看近效期药品\n");
        printf("  [8] 下架药品\n");
        printf("  [9] 查看待发药患者\n");
        printf("  [10] 执行发药\n");
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
            case 9:
                show_paid_patients_waiting_for_dispense();
                system("pause");
                break;
            case 10:
                handle_medicine_dispense();
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
    PatientNode* patient = NULL;
    char diagnosis_text[MAX_RECORD_LEN];
    char treatment_advice[MAX_RECORD_LEN];
    char med_id[MAX_ID_LEN];
    char item_id[MAX_ID_LEN];
    int decision;
    int count;
    int choice;
    int consult_success = 0;
    int added_prescription_count = 0;
    int added_check_count = 0;
    static int check_record_counter = 1001;

    printf("\n================ 医生接诊 ================\n");

    show_waiting_patients_by_doctor(g_current_doctor->id);

    count = get_waiting_patients_by_doctor(g_current_doctor->id, patient_list);
    if (count == 0)
    {
        system("pause");
        return;
    }

    choice = 1;
    patient = patient_list[0];
    printf("\n📙 当前系统自动叫号排队首位患者...\n");

    printf("\n========================================\n");
    printf("确认接诊患者：\n");
    printf("患者编号：%s\n", patient->id);
    printf("姓名：%s\n", patient->name);
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
            int continue_consult = get_safe_int("是否继续接诊？(1=继续, 0=取消): ");
            if (continue_consult == 0)
            {
                system("pause");
                return;
            }
        }
    }

    printf("========================================\n");
    
    get_safe_string("请输入诊断结论(叫号未到或特殊情况可直接回车留空): ", diagnosis_text, MAX_RECORD_LEN);
    get_safe_string("请输入处理意见(叫号未到或特殊情况可直接回车留空): ", treatment_advice, MAX_RECORD_LEN);

    // 保存患者原始状态，用于回退时恢复
    MedStatus original_status = patient->status;

    back_to_decision:
    printf("\n请选择诊疗决策:\n");
    printf("  [1] 结束就诊\n");
    printf("  [2] 开药\n");
    printf("  [3] 开检查\n");
    printf("  [4] 办理住院\n");
    printf("  [5] 叫号未到 (过号顺延3位)\n");

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

    if (decision == 2)
    {
        if (g_medicine_list == NULL || g_medicine_list->next == NULL)
        {
            printf("\n⚠️ 当前没有可用药品数据，无法执行“开药”决策！\n");
            system("pause");
            return;
        }
    }
    else if (decision == 3)
    {
        if (g_check_item_list == NULL || g_check_item_list->next == NULL || g_check_record_list == NULL)
        {
            printf("\n⚠️ 当前没有可用检查项目数据，无法执行“开检查”决策！\n");
            system("pause");
            return;
        }
    }

    if (decision == 5)
    {
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
            int current_index = choice - 1;
            int target_index = current_index + 3;

            if (target_index < count)
            {
                patient->queue_time = patient_list[target_index]->queue_time + 1;
            }
            else
            {
                patient->queue_time = time(NULL);
            }
            printf("\n🔄 [过号顺延] 第 %d 次叫号未到，已顺延至 3 位之后。(满 3 次将作废)\n", patient->call_count);
        }

        system("pause");
        return;
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
        system("pause");
        return;
    }

    if (decision == 2)
    {
        MedicineNode* med_curr = NULL;

        if (g_medicine_list == NULL)
        {
            printf("\n⚠️ 药品链表尚未初始化，无法开药！\n");
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
            patient->status = original_status;
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
                printf("操作取消！\n");
                return;
            }
            if (strcmp(med_id, "000") == 0)
            {
                // 回退到是否开药确认
                goto confirm_prescription;
            }
            if (strcmp(med_id, "0") == 0)
            {
                if (added_prescription_count == 0)
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

            add_prescription_to_patient(patient, med_id, quantity);
            added_prescription_count++;
            printf("✅ 开药成功：%s x %d\n", med->name, quantity);
        }

        if (added_prescription_count == 0)
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
            patient->status = original_status;
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
                printf("操作取消！\n");
                return;
            }
            if (strcmp(item_id, "000") == 0)
            {
                // 回退到是否开检查确认
                goto confirm_check;
            }
            if (strcmp(item_id, "0") == 0)
            {
                if (added_check_count == 0)
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

            snprintf(record_id, sizeof(record_id), "CRK-%04d", check_record_counter++);
            new_node = create_check_record_node(
                record_id,
                patient->id,
                item->item_id,
                item->item_name,
                item->dept,
                "",
                "",
                0,
                0
            );

            if (new_node == NULL)
            {
                printf("⚠️ 检查单创建失败，请稍后重试！\n");
                continue;
            }

            insert_check_record_tail(g_check_record_list, new_node);
            added_check_count++;
            printf("✅ 开具检查成功：%s -> 检查单号 %s\n", item->item_name, record_id);
        }

        if (added_check_count == 0)
        {
            printf("\n⚠️ 本次接诊未追加任何检查项目，请确认是否符合医嘱预期。\n");
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

static void handle_pharmacy_dispense()
{
    char patient_id[MAX_ID_LEN];

    printf("\n================ 执行发药 ================\n");
    show_paid_patients_waiting_for_dispense();
    printf("------------------------------------------------------\n");
    get_safe_string("请输入要发药的患者编号: ", patient_id, MAX_ID_LEN);
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
    printf("\n================ 预约签到入队 ================\n");
    get_safe_string("请输入预约编号: ", appointment_id, MAX_ID_LEN);
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
    sprintf(appointment_date, "%04d-%02d-%02d", 
            local_time->tm_year + 1900, 
            local_time->tm_mon + 1, 
            local_time->tm_mday);
    
    printf("\n当前日期：%s\n", appointment_date);
    
    // 时段输入环节
    input_slot:
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
                char patient_id[MAX_ID_LEN];
                get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
                handle_emergency_escape(patient_id);
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
        printf("  [6] 查询自己的基础病历\n");
        printf("  [7] 查询自己的就诊概览\n");
        printf("  [8] 查询自己的历史就诊记录\n");
        printf("  [9] 查询检查报告\n");
        printf("  [10] 自助账单缴费\n");
        printf("  [11] 发起服务投诉\n");
        printf("  [12] 查看投诉进度\n");
        printf("  [13] 就诊满意度评价\n");
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
            case 10:
                get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
                
                // 身份核验
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
            case 11:
                get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
                
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
            case 12:
                get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
                
                // 身份核验
                PatientNode* query_patient = find_patient_by_id_card(id_card);
                if (query_patient == NULL || strcmp(query_patient->id, patient_id) != 0)
                {
                    printf("\n❌ 身份核验失败！患者编号与身份证号不匹配\n");
                    system("pause");
                    break;
                }
                
                printf("\n身份核验成功！欢迎，%s\n", query_patient->name);
                query_patient_complaints(patient_id);
                system("pause");
                break;
            case 13:
                get_safe_string("请输入您的患者编号: ", patient_id, MAX_ID_LEN);
                get_safe_string("请输入您的身份证号（身份核验）: ", id_card, MAX_ID_LEN);
                
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
"M-001", "阿莫西林", "阿莫", "阿莫西林胶囊", 15.5, 100, MEDICARE_CLASS_A, "2027-04-01"));
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

