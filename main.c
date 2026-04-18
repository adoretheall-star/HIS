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
#include "medicine_service.h"
#include "pharmacy_service.h"
#include "admin_service.h"
#include "inpatient_service.h"

// 函数前置声明
static void quick_register_menu();
static void patient_self_service_menu();
static void patient_archive_menu();

static void handle_patient_register();
static void handle_appointment_register();
static void handle_appointment_query();
static void handle_appointment_cancel();
static void handle_appointment_check_in();
static void handle_bed_assign_for_patient(const char* patient_id);
static void handle_basic_record_query();
static void handle_patient_visit_overview();
static void handle_patient_visit_overview_by_id_card();
static void handle_patient_archive_query_by_id();
static void handle_patient_archive_query_by_id_card();
static void handle_patient_archive_query_by_name();
static void handle_patient_archive_update();
static void handle_deposit_recharge();
static void handle_discharge();

static void internal_login_menu();
static void admin_menu();
static void nurse_menu();
static void doctor_menu();
static void pharmacist_menu();
static void inpatient_menu();
static void handle_view_waiting_patients();
static void handle_doctor_consultation();
static void handle_medicine_basic_info_update();
static void handle_medicine_dispense();
static void handle_medicine_remove();
static void handle_account_register();
static void handle_account_update();
static void handle_scheduling_maintenance();
static void handle_admin_dashboard();
static void admin_statistics_menu();
static void readonly_dashboard_menu();
static void appointment_admin_menu();
static void recycle_bin_menu();

// 预约辅助管理菜单
static void appointment_admin_menu()
{
    int running = 1;
    int choice;

    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               📅 预约辅助管理\n");
        printf("======================================================\n");
        printf("  [1] 辅助预约登记\n");
        printf("  [2] 辅助预约查询\n");
        printf("  [3] 辅助预约取消\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");

        choice = get_safe_int("👉 请输入操作编号: ");

        switch (choice)
        {
            case 1:
                handle_appointment_register();
                break;
            case 2:
                handle_appointment_query();
                break;
            case 3:
                handle_appointment_cancel();
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

// 操作追踪 / 日志查看菜单（已合并到管理员主菜单 case 9，直接调用 show_logs()）

static void handle_expiring_medicine_check()
{
    char today_str[MAX_DATE_LEN];
    int days_threshold;

    printf("\n================ 近效期药品巡检 ================\n");

    get_safe_string("请输入当前日期 (YYYY-MM-DD): ", today_str, MAX_DATE_LEN);
    if (is_blank_string(today_str))
    {
        printf("⚠️ 日期不能为空或全为空格，操作取消！\n");
        system("pause");
        return;
    }

    days_threshold = get_safe_int("请输入近效期天数阈值: ");
    if (days_threshold <= 0)
    {
        printf("⚠️ 近效期天数阈值必须大于 0，操作取消！\n");
        system("pause");
        return;
    }

    show_expiring_medicines(today_str, days_threshold);

    system("pause");
}



static void handle_medicine_register()
{
    char name[MAX_MED_NAME_LEN];
    char generic_name[MAX_GENERIC_NAME_LEN];
    char alias[MAX_ALIAS_LEN];
    double price;
    int stock;
    int medicare_choice;
    MedicareType medicare_type = MEDICARE_NONE;
    char expiry_date[MAX_DATE_LEN];

    printf("\n================ 新增药品 ================\n");

    get_safe_string("请输入商品名: ", name, MAX_MED_NAME_LEN);
    if (is_blank_string(name))
    {
        printf("⚠️ 商品名不能为空或全为空格，操作取消！\n");
        system("pause");
        return;
    }

    get_safe_string("请输入通用名: ", generic_name, MAX_GENERIC_NAME_LEN);
    if (is_blank_string(generic_name))
    {
        printf("⚠️ 通用名不能为空或全为空格，操作取消！\n");
        system("pause");
        return;
    }

    get_safe_string("请输入别名 (直接回车表示无别名): ", alias, MAX_ALIAS_LEN);

    while (1)
    {
        price = get_safe_double("请输入单价: ");
        if (price <= 0)
        {
            printf("⚠️ 单价必须大于0，请重新输入！\n");
        }
        else
        {
            break;
        }
    }

    stock = get_safe_int("请输入库存: ");
    if (stock < 0)
    {
        printf("⚠️ 库存不能为负数，操作取消！\n");
        system("pause");
        return;
    }

    while (1)
    {
        medicare_choice = get_safe_int("请输入医保类型 (0: 非医保, 1: 甲类医保, 2: 乙类医保): ");
        if (medicare_choice == 0)
        {
            medicare_type = MEDICARE_NONE;
            break;
        }
        else if (medicare_choice == 1)
        {
            medicare_type = MEDICARE_CLASS_A;
            break;
        }
        else if (medicare_choice == 2)
        {
            medicare_type = MEDICARE_CLASS_B;
            break;
        }
        else
        {
            printf("⚠️ 无效的医保类型，请重新输入 (0, 1, 或 2)！\n");
        }
    }

    // 循环输入效期，直到输入合法日期
    while (1)
    {
        get_safe_string("请输入效期 (YYYY-MM-DD): ", expiry_date, MAX_DATE_LEN);
        if (is_blank_string(expiry_date))
        {
            printf("⚠️ 效期不能为空或全为空格，请重新输入！\n");
        }
        else if (!is_valid_date_string(expiry_date))
        {
            printf("⚠️ 效期格式不正确或日期无效，请使用 YYYY-MM-DD 格式，并确保日期合法。请重新输入！\n");
        }
        else
        {
            break; // 日期合法，跳出循环
        }
    }

    register_medicine(name, alias, generic_name, price, stock, medicare_type, expiry_date);

    system("pause");
}

static void handle_medicine_stock_update()
{
    char med_id[MAX_ID_LEN];
    int new_stock;

    printf("\n================ 修改药品库存 ================\n");

    get_safe_string("请输入药品编号: ", med_id, MAX_ID_LEN);
    if (is_blank_string(med_id))
    {
        printf("⚠️ 药品编号不能为空或全为空格，操作取消！\n");
        system("pause");
        return;
    }

    new_stock = get_safe_int("请输入新库存: ");
    if (new_stock < 0)
    {
        printf("⚠️ 新库存不能为负数，操作取消！\n");
        system("pause");
        return;
    }

    update_medicine_stock(med_id, new_stock);

    system("pause");
}



static void admin_menu()
{
    int running = 1;
    int choice;

    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               🔐 管理员菜单\n");
        printf("======================================================\n");
        printf("  [1] 查看员工列表\n");
        printf("  [2] 新增员工账号\n");
        printf("  [3] 修改员工资料与角色\n");
        printf("  [4] 动态排班维护\n");
        printf("  [5] 管理统计与运行监控\n");
        printf("  [6] 回收站查看 / 恢复入口\n");
        printf("  [7] 预约辅助管理\n");
        printf("  [8] 病房管理\n");
        printf("  [9] 操作追踪 / 日志查看\n");
        printf("  [0] 退出登录\n");
        printf("------------------------------------------------------\n");

        choice = get_safe_int("👉 请输入操作编号: ");

        switch (choice)
        {
            case 1:
                show_all_accounts();
                system("pause");
                break;
            case 2:
                handle_account_register();
                break;
            case 3:
                handle_account_update();
                break;
            case 4:
                handle_scheduling_maintenance();
                break;
            case 5:
                        admin_statistics_menu();
                        break;
            case 6:
                recycle_bin_menu();
                break;
            case 7:
                appointment_admin_menu();
                break;
            case 8:
                inpatient_menu();
                break;
            case 9:
                show_logs();
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

// 护士住院与床位办理菜单
// 处理住院登记
static void handle_inpatient_register()
{
    char patient_id[MAX_ID_LEN];
    int estimated_days;
    double deposit;
    int condition_level;
    PatientNode* patient = NULL;
    InpatientRecord* inpatient_record = NULL;
    char input_buf[64];
    
    printf("\n================ 住院登记 ===============-\n");
    
    // 输入患者编号（不能为空、必须存在、不能已住院；输入Q取消）
    while (1)
    {
        get_safe_string("请输入患者编号 (输入Q返回上一级): ", patient_id, MAX_ID_LEN);
        
        // 检查是否取消
        if (strcmpi(patient_id, "q") == 0)
        {
            printf("\n已取消本次住院登记，返回上一级\n");
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
        patient = find_patient_by_id(g_patient_list, patient_id);
        if (patient == NULL)
        {
            printf("⚠️ 未找到该患者，请重新输入患者编号或先建档\n");
            continue;
        }
        
        // 检查患者是否已住院
        inpatient_record = find_active_inpatient_by_patient_id(patient_id);
        if (inpatient_record != NULL)
        {
            printf("⚠️ 该患者当前已住院，不能重复登记，请重新输入患者编号\n");
            continue;
        }
        
        // 所有校验通过
        break;
    }
    
    // 输入预计住院天数（必须大于0；输入Q取消）
    while (1)
    {
        get_safe_string("请输入预计住院天数 (输入Q返回上一级): ", input_buf, sizeof(input_buf));
        
        // 检查是否取消
        if (strcmpi(input_buf, "q") == 0)
        {
            printf("\n已取消本次住院登记，返回上一级\n");
            system("pause");
            return;
        }
        
        // 转换为数字
        if (sscanf(input_buf, "%d", &estimated_days) != 1)
        {
            printf("⚠️ 请输入有效数字，请重新输入\n");
            continue;
        }
        
        if (estimated_days > 0)
        {
            break;
        }
        printf("⚠️ 预计住院天数必须大于 0，请重新输入\n");
    }
    
    // 输入初始押金（必须大于0；输入Q取消）
    while (1)
    {
        get_safe_string("请输入初始押金 (输入Q返回上一级): ", input_buf, sizeof(input_buf));
        
        // 检查是否取消
        if (strcmpi(input_buf, "q") == 0)
        {
            printf("\n已取消本次住院登记，返回上一级\n");
            system("pause");
            return;
        }
        
        // 转换为数字
        if (sscanf(input_buf, "%lf", &deposit) != 1)
        {
            printf("⚠️ 请输入有效数字，请重新输入\n");
            continue;
        }
        
        if (deposit > 0)
        {
            break;
        }
        printf("⚠️ 初始押金必须大于 0，请重新输入\n");
    }
    
    // 输入病情级别（只能是1或2；输入Q取消）
    while (1)
    {
        get_safe_string("请输入病情级别 (1=普通/2=重症，输入Q返回上一级): ", input_buf, sizeof(input_buf));
        
        // 检查是否取消
        if (strcmpi(input_buf, "q") == 0)
        {
            printf("\n已取消本次住院登记，返回上一级\n");
            system("pause");
            return;
        }
        
        // 转换为数字
        if (sscanf(input_buf, "%d", &condition_level) != 1)
        {
            printf("⚠️ 请输入有效数字，请重新输入\n");
            continue;
        }
        
        if (condition_level == 1 || condition_level == 2)
        {
            break;
        }
        printf("⚠️ 病情级别输入错误，请重新输入 1 或 2\n");
    }

    // 显示住院登记摘要并确认
    printf("\n======================================================\n");
    printf("                     住院登记摘要\n");
    printf("======================================================\n");
    printf("患者编号：%s\n", patient_id);
    printf("患者姓名：%s\n", patient->name);
    printf("预计住院天数：%d 天\n", estimated_days);
    printf("初始押金：%.2f 元\n", deposit);
    printf("病情级别：%s\n", condition_level == 2 ? "重症" : "普通");
    printf("推荐病房类型：%s\n", condition_level == 2 ? "ICU" : "普通病房");
    printf("======================================================\n");

    // 确认是否提交住院登记
    char confirm;
    while (1)
    {
        confirm = get_single_char("是否确认提交本次住院登记？(Y/N): ");
        if (confirm == 'Y' || confirm == 'y')
        {
            break;
        }
        else if (confirm == 'N' || confirm == 'n')
        {
            printf("\n已取消本次住院登记\n");
            system("pause");
            return;
        }
        else
        {
            printf("⚠️ 输入无效，请输入 Y 或 N！\n");
        }
    }

    // 先调用业务层完成真正的住院登记
    if (!register_inpatient(patient_id, estimated_days, deposit, condition_level))
    {
        // 登记失败，直接返回
        system("pause");
        return;
    }
    
    // 住院登记成功后询问是否立即分配床位
    printf("\n");
    
    // 询问是否立即分配床位
    while (1)
    {
        confirm = get_single_char("是否现在立即分配床位？(Y/N): ");
        if (confirm == 'Y' || confirm == 'y' || confirm == 'N' || confirm == 'n')
        {
            break;
        }
        printf("⚠️ 输入无效，请输入 Y 或 N！\n");
    }
    
    if (confirm == 'Y' || confirm == 'y')
    {
        // 直接进入分配床位流程
        handle_bed_assign_for_patient(patient_id);
    }
    else
    {
        printf("\n已完成住院登记，可稍后在病房管理中单独分配床位\n");
        system("pause");
    }
}

// 处理床位分配
static void handle_bed_assign()
{
    char patient_id[MAX_ID_LEN];
    PatientNode* patient = NULL;
    InpatientRecord* inpatient_record = NULL;
    
    printf("\n================ 分配床位 ===============-\n");
    
    // 输入患者编号（支持Q/q取消）
    while (1)
    {
        get_safe_string("请输入患者编号 (输入Q返回上一级): ", patient_id, MAX_ID_LEN);
        
        // 检查是否取消
        if (strcmpi(patient_id, "q") == 0)
        {
            printf("\n已取消本次床位分配，返回上一级\n");
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
        patient = find_patient_by_id(g_patient_list, patient_id);
        if (patient == NULL)
        {
            printf("⚠️ 未找到该患者，请重新输入患者编号\n");
            continue;
        }
        
        // 检查患者是否已住院
        inpatient_record = find_active_inpatient_by_patient_id(patient_id);
        if (inpatient_record == NULL)
        {
            printf("⚠️ 该患者未住院或已出院，请重新输入患者编号\n");
            continue;
        }
        
        // 检查患者是否已经分配床位
        if (strlen(inpatient_record->bed_id) > 0)
        {
            printf("⚠️ 该患者已分配床位，不能重复分配，请重新输入患者编号\n");
            continue;
        }
        
        // 所有校验通过
        break;
    }
    
    // 进入床位分配流程
    handle_bed_assign_for_patient(patient_id);
}

// 处理床位分配（已知患者编号）
static void handle_bed_assign_for_patient(const char* patient_id)
{
    char bed_id[MAX_ID_LEN];
    PatientNode* patient = NULL;
    InpatientRecord* inpatient_record = NULL;
    WardNode* curr_bed = NULL;
    int has_recommended_beds = 0;
    int has_free_beds = 0;
    
    // 检查患者是否存在
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到该患者！\n");
        system("pause");
        return;
    }
    
    // 检查患者是否已住院
    inpatient_record = find_active_inpatient_by_patient_id(patient_id);
    if (inpatient_record == NULL)
    {
        printf("⚠️ 该患者未住院或已出院！\n");
        system("pause");
        return;
    }
    
    // 显示推荐病房类型
    printf("\n📋 推荐病房类型：%s\n", 
        inpatient_record->recommended_ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
    
    // 展示推荐类型可用床位
    printf("\n💡 推荐类型可用床位：\n");
    if (g_ward_list != NULL && g_ward_list->next != NULL)
    {
        curr_bed = g_ward_list->next;
        while (curr_bed != NULL)
        {
            if (!curr_bed->is_occupied && curr_bed->ward_type == inpatient_record->recommended_ward_type)
            {
                printf("  - 病房 %s / 床位 %s (%s)\n", curr_bed->room_id, curr_bed->bed_id, 
                    curr_bed->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
                has_recommended_beds = 1;
            }
            curr_bed = curr_bed->next;
        }
    }
    
    if (!has_recommended_beds)
    {
        printf("  当前推荐类型暂无空闲床位\n");
    }
    
    // 展示全部空闲床位
    printf("\n🏥 当前全部空闲床位：\n");
    if (g_ward_list != NULL && g_ward_list->next != NULL)
    {
        curr_bed = g_ward_list->next;
        while (curr_bed != NULL)
        {
            if (!curr_bed->is_occupied)
            {
                printf("  - 病房 %s / 床位 %s (%s)\n", curr_bed->room_id, curr_bed->bed_id, 
                    curr_bed->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
                has_free_beds = 1;
            }
            curr_bed = curr_bed->next;
        }
    }
    
    if (!has_free_beds)
    {
        printf("  当前无空闲床位\n");
        system("pause");
        return;
    }
    
    // 输入床位编号（支持Q/q取消，即时校验）
    while (1)
    {
        get_safe_string("\n请输入床位编号 (输入Q返回上一级): ", bed_id, MAX_ID_LEN);
        
        // 检查是否取消
        if (strcmpi(bed_id, "q") == 0)
        {
            printf("\n已取消本次床位分配，返回上一级\n");
            system("pause");
            return;
        }
        
        // 检查床位编号是否为空
        if (is_blank_string(bed_id))
        {
            printf("⚠️ 床位编号不能为空，请重新输入\n");
            continue;
        }
        
        // 检查床位是否存在
        WardNode* check_bed = find_bed_by_id(bed_id);
        if (check_bed == NULL)
        {
            printf("⚠️ 未找到该床位，请重新输入床位编号\n");
            continue;
        }
        
        // 检查床位是否空闲
        if (check_bed->is_occupied)
        {
            printf("⚠️ 该床位已被占用，请重新输入床位编号\n");
            continue;
        }
        
        // 所有校验通过
        break;
    }

    // 查找目标床位信息用于显示摘要
    WardNode* target_bed = find_bed_by_id(bed_id);

    // 显示床位分配摘要并确认
    printf("\n======================================================\n");
    printf("                     床位分配摘要\n");
    printf("======================================================\n");
    printf("患者编号：%s\n", patient_id);
    printf("患者姓名：%s\n", patient->name);
    printf("推荐病房类型：%s\n", inpatient_record->recommended_ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
    printf("------------------------------------------------------\n");
    printf("目标床位：\n");
    if (target_bed != NULL)
    {
        printf("  病房编号：%s\n", target_bed->room_id);
        printf("  床位编号：%s\n", target_bed->bed_id);
        printf("  床位类型：%s\n", target_bed->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
    }
    else
    {
        printf("  床位编号：%s\n", bed_id);
    }
    printf("======================================================\n");

    // 确认是否分配床位
    char confirm;
    while (1)
    {
        confirm = get_single_char("是否确认分配该床位？(Y/N): ");
        if (confirm == 'Y' || confirm == 'y')
        {
            break;
        }
        else if (confirm == 'N' || confirm == 'n')
        {
            printf("\n已取消本次床位分配\n");
            system("pause");
            return;
        }
        else
        {
            printf("⚠️ 输入无效，请输入 Y 或 N！\n");
        }
    }

    // 分配床位
    assign_bed_to_patient(patient_id, bed_id);
    system("pause");
}

// 处理住院记录查询
static void handle_inpatient_record_query()
{
    char patient_id[MAX_ID_LEN];
    
    printf("\n================ 查询住院记录 ===============-\n");
    get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
    show_inpatient_record_by_patient_id(patient_id);
    system("pause");
}

// 处理日结计费（带预览和确认）
static void handle_daily_settlement()
{
    char patient_id[MAX_ID_LEN];
    PatientNode* patient = NULL;
    InpatientRecord* inpatient_record = NULL;
    double daily_rate = 0.0;
    double current_deposit = 0.0;
    double after_deduction = 0.0;
    int confirm = 0;
    
    printf("\n================ 日结计费 ===============-\n");
    get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);
    
    // 输入校验
    if (is_blank_string(patient_id))
    {
        printf("⚠️ 患者编号不能为空！\n");
        system("pause");
        return;
    }
    
    // 检查患者是否存在
    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("⚠️ 未找到该患者！\n");
        system("pause");
        return;
    }
    
    // 检查患者是否已住院
    inpatient_record = find_active_inpatient_by_patient_id(patient_id);
    if (inpatient_record == NULL)
    {
        printf("⚠️ 该患者未住院，无法执行日结计费！\n");
        system("pause");
        return;
    }
    
    // 检查患者是否已分配床位
    if (strlen(inpatient_record->bed_id) == 0)
    {
        printf("⚠️ 该患者未分配床位，无法执行日结计费！\n");
        system("pause");
        return;
    }
    
    // 计算当日费用
    daily_rate = inpatient_record->ward_type == WARD_TYPE_ICU ? 500.0 : 120.0;
    
    // 检查押金是否足够
    current_deposit = inpatient_record->deposit_balance;
    if (current_deposit < daily_rate)
    {
        printf("⚠️ 押金不足，无法执行日结计费！\n");
        printf("当前押金余额：%.2f 元，当日费用：%.2f 元\n", current_deposit, daily_rate);
        system("pause");
        return;
    }
    
    // 计算扣费后余额
    after_deduction = current_deposit - daily_rate;
    
    // 显示日结计费预览
    printf("\n================ 日结计费预览 ================\n");
    printf("患者编号：%s\n", patient_id);
    printf("患者姓名：%s\n", patient->name);
    printf("床位编号：%s\n", inpatient_record->bed_id);
    printf("病房类型：%s\n", inpatient_record->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
    printf("当前已结算住院天数：%d 天\n", inpatient_record->days_stayed);
    printf("本次结算后累计住院天数：%d 天\n", inpatient_record->days_stayed + 1);
    printf("本次应扣：%.2f 元\n", daily_rate);
    printf("当前押金：%.2f 元\n", current_deposit);
    printf("扣费后余额：%.2f 元\n", after_deduction);
    
    // 确认是否扣费
    while (1)
    {
        confirm = get_single_char("是否确认扣费？(Y/N): ");
        if (confirm == 'Y' || confirm == 'y' || confirm == 'N' || confirm == 'n')
        {
            break;
        }
        printf("⚠️ 输入无效，请输入 Y 或 N！\n");
    }
    
    // 执行扣费
    if (confirm == 'Y' || confirm == 'y')
    {
        daily_settlement(patient_id);
    }
    else
    {
        printf("已取消日结计费\n");
    }
    
    system("pause");
}

// 处理转床
static void handle_transfer_bed()
{
    char patient_id[MAX_ID_LEN];
    char new_bed_id[MAX_ID_LEN];
    char confirm;
    PatientNode* patient = NULL;
    InpatientRecord* inpatient_record = NULL;
    WardNode* old_bed = NULL;
    WardNode* new_bed = NULL;
    WardNode* curr_bed = NULL;
    int has_free_beds = 0;
    
    printf("\n================ 转床 ===============-\n");
    
    // 输入患者编号（支持Q/q取消）
    while (1)
    {
        get_safe_string("请输入患者编号 (输入Q返回上一级): ", patient_id, MAX_ID_LEN);
        
        // 检查是否取消
        if (strcmpi(patient_id, "q") == 0)
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
        patient = find_patient_by_id(g_patient_list, patient_id);
        if (patient == NULL)
        {
            printf("⚠️ 未找到该患者，请重新输入患者编号\n");
            continue;
        }
        
        // 检查患者是否已住院
        inpatient_record = find_active_inpatient_by_patient_id(patient_id);
        if (inpatient_record == NULL)
        {
            printf("⚠️ 该患者未住院或已出院，请重新输入患者编号\n");
            continue;
        }
        
        // 检查患者是否已经分配床位
        if (strlen(inpatient_record->bed_id) == 0)
        {
            printf("⚠️ 该患者当前未分配床位，无法转床\n");
            system("pause");
            return;
        }
        
        // 所有校验通过
        break;
    }
    
    // 显示当前原床位信息（表格化展示）
    old_bed = find_bed_by_id(inpatient_record->bed_id);
    printf("\n======================================================\n");
    printf("                     当前原床位信息\n");
    printf("======================================================\n");
    printf("病房编号       床位编号       病房类型\n");
    printf("------------------------------------------------------\n");
    if (old_bed != NULL)
    {
        printf("%-14s %-14s %s\n", old_bed->room_id, old_bed->bed_id, 
            old_bed->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
    }
    printf("======================================================\n");
    
    // 展示当前可转入床位（排除原床位）- 表格化展示
    printf("\n======================================================\n");
    printf("              当前可转入床位（已排除原床位）\n");
    printf("======================================================\n");
    printf("病房编号       床位编号       病房类型\n");
    printf("------------------------------------------------------\n");
    if (g_ward_list != NULL && g_ward_list->next != NULL)
    {
        curr_bed = g_ward_list->next;
        while (curr_bed != NULL)
        {
            // 只显示空闲且不是原床位的床位
            if (!curr_bed->is_occupied && strcmp(curr_bed->bed_id, inpatient_record->bed_id) != 0)
            {
                printf("%-14s %-14s %s\n", curr_bed->room_id, curr_bed->bed_id, 
                    curr_bed->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
                has_free_beds = 1;
            }
            curr_bed = curr_bed->next;
        }
    }
    printf("======================================================\n");
    
    if (!has_free_beds)
    {
        printf("当前无空闲床位可转入\n");
        system("pause");
        return;
    }
    
    // 输入新床位编号（支持Q/q取消，即时校验）
    while (1)
    {
        get_safe_string("\n请输入新床位编号 (输入Q返回上一级): ", new_bed_id, MAX_ID_LEN);
        
        // 检查是否取消
        if (strcmpi(new_bed_id, "q") == 0)
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
        
        // 检查新床位是否存在
        WardNode* check_bed = find_bed_by_id(new_bed_id);
        if (check_bed == NULL)
        {
            printf("⚠️ 未找到该床位，请重新输入新床位编号\n");
            continue;
        }
        
        // 检查新床位是否空闲
        if (check_bed->is_occupied)
        {
            printf("⚠️ 该床位已被占用，请重新输入新床位编号\n");
            continue;
        }
        
        // 检查新床位是否与原床位相同
        if (strcmp(new_bed_id, inpatient_record->bed_id) == 0)
        {
            printf("⚠️ 新床位不能与原床位相同，请重新输入新床位编号\n");
            continue;
        }
        
        // 所有校验通过
        break;
    }
    
    // 查找新床位信息
    new_bed = find_bed_by_id(new_bed_id);
    
    // 显示转床操作摘要并确认
    printf("\n======================================================\n");
    printf("                     转床操作摘要\n");
    printf("======================================================\n");
    printf("患者编号：%s\n", patient_id);
    printf("患者姓名：%s\n", patient->name);
    printf("------------------------------------------------------\n");
    printf("当前原床位：\n");
    if (old_bed != NULL)
    {
        printf("  病房编号：%s\n", old_bed->room_id);
        printf("  床位编号：%s\n", old_bed->bed_id);
        printf("  病房类型：%s\n", old_bed->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
    }
    printf("------------------------------------------------------\n");
    printf("目标新床位：\n");
    if (new_bed != NULL)
    {
        printf("  病房编号：%s\n", new_bed->room_id);
        printf("  床位编号：%s\n", new_bed->bed_id);
        printf("  病房类型：%s\n", new_bed->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
    }
    printf("======================================================\n");
    
    // 确认是否转床
    while (1)
    {
        confirm = get_single_char("是否确认转床？(Y/N): ");
        if (confirm == 'Y' || confirm == 'y')
        {
            break;
        }
        else if (confirm == 'N' || confirm == 'n')
        {
            printf("\n已取消本次转床操作\n");
            system("pause");
            return;
        }
        else
        {
            printf("⚠️ 输入无效，请输入 Y 或 N！\n");
        }
    }
    
    // 执行转床
    transfer_bed(patient_id, inpatient_record->bed_id, new_bed_id);
    system("pause");
}

// 处理押金充值
static void handle_deposit_recharge()
{
    char patient_id[MAX_ID_LEN];
    double amount = 0.0;
    PatientNode* patient = NULL;
    InpatientRecord* inpatient_record = NULL;
    char input_buf[64];

    printf("\n================ 押金充值 ===============-\n");

    // 输入患者编号（支持Q/q取消）
    while (1)
    {
        get_safe_string("请输入患者编号 (输入Q返回上一级): ", patient_id, MAX_ID_LEN);

        // 检查是否取消
        if (strcmpi(patient_id, "q") == 0)
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
        patient = find_patient_by_id(g_patient_list, patient_id);
        if (patient == NULL)
        {
            printf("⚠️ 未找到该患者，请重新输入患者编号\n");
            continue;
        }

        // 检查患者是否已住院
        inpatient_record = find_active_inpatient_by_patient_id(patient_id);
        if (inpatient_record == NULL)
        {
            printf("⚠️ 该患者未住院或已出院，无法充值押金\n");
            continue;
        }

        // 所有校验通过
        break;
    }

    // 输入充值金额（必须大于0；输入Q取消）
    while (1)
    {
        get_safe_string("请输入充值金额 (输入Q返回上一级): ", input_buf, sizeof(input_buf));

        // 检查是否取消
        if (strcmpi(input_buf, "q") == 0)
        {
            printf("\n已取消本次押金充值，返回上一级\n");
            system("pause");
            return;
        }

        // 转换为数字
        if (sscanf(input_buf, "%lf", &amount) != 1)
        {
            printf("⚠️ 请输入有效数字，请重新输入\n");
            continue;
        }

        if (amount > 0)
        {
            break;
        }
        printf("⚠️ 充值金额必须大于 0，请重新输入\n");
    }

    // 显示押金充值摘要并确认
    printf("\n======================================================\n");
    printf("                     押金充值摘要\n");
    printf("======================================================\n");
    printf("患者编号：%s\n", patient_id);
    printf("患者姓名：%s\n", patient->name);
    printf("充值金额：%.2f 元\n", amount);
    printf("当前押金余额：%.2f 元\n", inpatient_record->deposit_balance);
    printf("充值后押金余额：%.2f 元\n", inpatient_record->deposit_balance + amount);
    printf("======================================================\n");

    // 确认是否充值
    char confirm;
    while (1)
    {
        confirm = get_single_char("是否确认充值？(Y/N): ");
        if (confirm == 'Y' || confirm == 'y')
        {
            break;
        }
        else if (confirm == 'N' || confirm == 'n')
        {
            printf("\n已取消本次押金充值\n");
            system("pause");
            return;
        }
        else
        {
            printf("⚠️ 输入无效，请输入 Y 或 N！\n");
        }
    }

    // 执行押金充值
    recharge_inpatient_deposit(patient_id, amount);
    system("pause");
}

// 处理办理出院
static void handle_discharge()
{
    char patient_id[MAX_ID_LEN];
    PatientNode* patient = NULL;
    InpatientRecord* inpatient_record = NULL;
    WardNode* bed = NULL;

    printf("\n================ 办理出院（自动释放床位） ===============-\n");

    // 输入患者编号（支持Q/q取消）
    while (1)
    {
        get_safe_string("请输入患者编号 (输入Q返回上一级): ", patient_id, MAX_ID_LEN);

        // 检查是否取消
        if (strcmpi(patient_id, "q") == 0)
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
        patient = find_patient_by_id(g_patient_list, patient_id);
        if (patient == NULL)
        {
            printf("⚠️ 未找到该患者，请重新输入患者编号\n");
            continue;
        }

        // 检查患者是否已住院
        inpatient_record = find_active_inpatient_by_patient_id(patient_id);
        if (inpatient_record == NULL)
        {
            printf("⚠️ 该患者未住院或已出院，无法办理出院\n");
            continue;
        }

        // 所有校验通过
        break;
    }

    // 查找当前床位信息
    if (strlen(inpatient_record->bed_id) > 0)
    {
        bed = find_bed_by_id(inpatient_record->bed_id);
    }

    // 显示出院办理摘要并确认
    printf("\n======================================================\n");
    printf("                     出院办理摘要\n");
    printf("======================================================\n");
    printf("患者编号：%s\n", patient_id);
    printf("患者姓名：%s\n", patient->name);
    printf("------------------------------------------------------\n");
    printf("当前床位：\n");
    if (bed != NULL)
    {
        printf("  病房编号：%s\n", bed->room_id);
        printf("  床位编号：%s\n", bed->bed_id);
        printf("  病房类型：%s\n", bed->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房");
    }
    else
    {
        printf("  未分配床位\n");
    }
    printf("------------------------------------------------------\n");
    printf("当前押金余额：%.2f 元\n", inpatient_record->deposit_balance);
    printf("已住天数：%d 天\n", inpatient_record->days_stayed);
    printf("======================================================\n");
    printf("⚠️ 提示：办理出院后将自动释放床位\n");
    printf("======================================================\n");

    // 确认是否办理出院
    char confirm;
    while (1)
    {
        confirm = get_single_char("是否确认办理出院？(Y/N): ");
        if (confirm == 'Y' || confirm == 'y')
        {
            break;
        }
        else if (confirm == 'N' || confirm == 'n')
        {
            printf("\n已取消本次出院办理\n");
            system("pause");
            return;
        }
        else
        {
            printf("⚠️ 输入无效，请输入 Y 或 N！\n");
        }
    }

    // 执行出院办理
    discharge_patient(patient_id);
    system("pause");
}

static void nurse_inpatient_menu()
{
    int running = 1;
    int choice;

    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("          🏥 住院与床位办理（护士版）\n");
        printf("======================================================\n");
        printf("  [1] 住院登记\n");
        printf("  [2] 分配床位\n");
        printf("  [3] 查询住院患者信息\n");
        printf("  [4] 查询患者住院记录\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");

        choice = get_safe_int("👉 请输入操作编号: ");

        switch (choice)
        {
            case 1:
                handle_inpatient_register();
                break;
            case 2:
                handle_bed_assign();
                break;
            case 3:
                show_hospitalized_patients();
                system("pause");
                break;
            case 4:
                handle_inpatient_record_query();
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
        printf("  [2] 住院与床位办理\n");
        printf("  [0] 退出登录\n");
        printf("------------------------------------------------------\n");

        switch (get_safe_int("👉 请输入操作编号: "))
        {
            case 1:
                quick_register_menu();
                break;
            case 2:
                nurse_inpatient_menu();
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
    int choice;
    int threshold;
    char keyword[MAX_MED_NAME_LEN];


    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               💊 药房菜单\n");
        printf("======================================================\n");
        printf("  [1] 查看全部药品\n");
        printf("  [2] 查询药品\n");
        printf("  [3] 低库存预警\n");
        printf("  [4] 近效期巡检\n");
        printf("  [5] 新增药品\n");
        printf("  [6] 修改药品库存\n");
        printf("  [7] 修改药品基础信息\n");
        printf("  [8] 查看待发药患者\n"); // 新增
        printf("  [9] 为患者发药\n");     // 新增
        printf("  [10] 下架药品\n");     // 新增
        printf("  [0] 退出登录\n");
        printf("------------------------------------------------------\n");

        choice = get_safe_int("👉 请输入操作编号: ");

        switch (choice)
        {
            case 1:
                show_all_medicines();
                system("pause");
                break;
            case 2:
                get_safe_string("请输入药品关键字: ", keyword, MAX_MED_NAME_LEN);
                search_medicine_by_keyword(keyword);
                system("pause");
                break;
            case 3:
                threshold = get_safe_int("请输入低库存阈值: ");
                if (threshold <= 0)
                {
                    printf("⚠️ 低库存阈值必须大于 0，操作取消！\n");
                    system("pause");
                    break;
                }
                show_low_stock_medicines(threshold);
                system("pause");
                break;
            case 4:
                handle_expiring_medicine_check();
                break;
            case 5:
                handle_medicine_register();
                break;
            case 6:
                handle_medicine_stock_update();
                break;
            case 7:
                handle_medicine_basic_info_update();
                break;
            case 8: // 新增
                show_paid_patients_waiting_for_dispense();
                system("pause"); // 添加 pause
                break;
            case 9: // 新增
                handle_medicine_dispense();
                break;
            case 10: // 新增
                handle_medicine_remove();
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
    // 记录登录日志
    add_log("员工登录", account->username, account->real_name);
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
    // 声明变量
    PatientNode* test_patient_paid = NULL;
    PatientNode* test_patient_fail = NULL;
    PatientNode* search_result = NULL;
    int running = 1; // 控制系统运行状态的标志位
    int choice = 0;

    // ---------------------------------------------------
    // 第一阶段：系统初始化 (加载数据)
    // ---------------------------------------------------
    printf("======================================================\n");
    printf("     🚀 正在启动智慧医疗信息管理系统 (HIS) ...\n");
    printf("======================================================\n");

    // 这里未来调用 dat-io.c 里的函数，把 txt 数据全部读进双向链表
    // printf("-> 正在加载患者数据...\n");
    // init_patient_list();
    // printf("-> 正在加载医生与科室数据...\n");
    // init_doctor_list();
    // printf("-> 正在加载药品库存数据...\n");
    // init_medicine_list();

    printf("\n✅ 系统初始化完成，数据加载成功！\n");
    // 模拟一个加载停顿，让控制台看起来很高级 (Windows下用Sleep, Linux下用sleep)
    // Sleep(1000);

    // ---------------------------------------------------
    // 第二阶段：主事件循环 (防崩溃核心)

    // ---------------------------------------------------
    printf("⚙️ 正在点火启动底层双向链表引擎...\n");

    // 1. 初始化所有链表头节点
    g_patient_list = init_patient_list();
    g_appointment_list = init_appointment_list();
    g_doctor_list = init_doctor_list();
    g_medicine_list = init_medicine_list();
    g_ward_list = init_ward_list();
    g_account_list = init_account_list();
    g_inpatient_list = create_inpatient_record_head();
    
    // 初始化日志列表
    init_log_list();

    // 2. 注入一组极端测试数据
    // 注入药品数据 (需先注入，因为患者处方会引用药品)
    insert_medicine_tail(g_medicine_list, create_medicine_node(
        "M-001", "感冒灵", "Ganmaoling", "感冒灵", 15.5, 100 // 充足库存
        , MEDICARE_CLASS_A, "2027-12-31"));
    insert_medicine_tail(g_medicine_list, create_medicine_node(
        "M-002", "止痛片", "Zhitongpian", "布洛芬", 8.0, 1 // 少量库存，用于演示库存不足发药失败
        , MEDICARE_NONE, "2026-11-15"));

    // 注入患者数据 - P-001 (成功发药测试患者)
    test_patient_paid = create_patient_node(
        "P-001", "张三", 19, "110101199001011234"
    );
    insert_patient_tail(g_patient_list, test_patient_paid);

    // 设置 P-001 为已缴费状态
    test_patient_paid->status = STATUS_PAID;
    
    // 补充症状：发热、咳嗽
    strncpy(test_patient_paid->symptom, "发热、咳嗽", MAX_SYMPTOM_LEN - 1);
    test_patient_paid->symptom[MAX_SYMPTOM_LEN - 1] = '\0';

    // 为 P-001 添加处方项 (使用真实接口)
    add_prescription_to_patient(test_patient_paid, "M-001", 2); // 2份感冒灵 (库存充足)
    add_prescription_to_patient(test_patient_paid, "M-002", 1); // 1份止痛片 (库存刚好)

    // 注入患者数据 - P-002 (库存不足测试患者)
    test_patient_fail = create_patient_node(
        "P-002", "李四", 25, "110101199501011234"
    );
    insert_patient_tail(g_patient_list, test_patient_fail);

    // 设置 P-002 为已缴费状态
    test_patient_fail->status = STATUS_PAID;
    
    // 补充症状：腹泻
    strncpy(test_patient_fail->symptom, "腹泻", MAX_SYMPTOM_LEN - 1);
    test_patient_fail->symptom[MAX_SYMPTOM_LEN - 1] = '\0';

    // 为 P-002 添加处方项 (使用真实接口)
    add_prescription_to_patient(test_patient_fail, "M-002", 2); // 2份止痛片 (库存不足)

    // 注入医生数据
    insert_doctor_tail(g_doctor_list, create_doctor_node(
        "D-001", "李大夫", "外科"
    ));
    insert_appointment_tail(g_appointment_list, create_appointment_node(
"A-001", "P-001", "2026-04-01", "上午", "D-001", "外科", RESERVED
));
    insert_ward_tail(g_ward_list, create_ward_node(
"101", "W-101", WARD_TYPE_GENERAL
));
    insert_ward_tail(g_ward_list, create_ward_node(
"ICU-01", "W-102", WARD_TYPE_ICU
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
    printf(" -> 测试患者1: %s (P-001，用于成功发药演示)\n"
, g_patient_list->next->name);
    printf(" -> 测试患者2: %s (P-002，用于库存不足失败演示)\n"
, g_patient_list->next->next->name);
    printf(" -> 测试医生: %s (%s)\n"
, g_doctor_list->next->name, g_doctor_list->next->department);
    printf(" -> 测试药品: %s (库存:%d)\n"
, g_medicine_list->next->name, g_medicine_list->next->stock);
    printf(" -> 测试药品: %s (库存:%d)\n"
, g_medicine_list->next->next->name, g_medicine_list->next->next->stock);
    printf(" -> 测试预约: %s (%s %s)\n"
, g_appointment_list->next->appointment_id, g_appointment_list->next->appointment_date, g_appointment_list->next->appointment_slot);
    printf(" -> 测试床位: %s\n"
, g_ward_list->next->bed_id);
    printf(" -> 测试超管: %s (权限级别:%d)\n\n"
, g_account_list->next->real_name, g_account_list->next->role);
    printf("📋 发药测试说明:\n");
    printf("   - P-001: 已缴费，处方包含 2 份感冒灵和 1 份止痛片\n");
    printf("   - P-002: 已缴费，处方包含 2 份止痛片（库存不足）\n");
    printf("   - M-001: 感冒灵，库存 100（充足）\n");
    printf("   - M-002: 止痛片，库存 1（不足）\n\n");
// 测试查找引擎
    search_result = find_patient_by_id(g_patient_list, "P-001");
    if (search_result != NULL) {
        printf("🔍 雷达搜索成功！找到目标：患者姓名 [%s]，当前余额 [%.2f]\n\n", search_result->name, search_result->balance);
    } else {
        printf("❌ 雷达搜索失败，查无此人！\n\n");
    }
    // 🚀 时停魔法：按任意键后才清屏进入菜单！
    system(
"pause"
);

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

        choice = get_safe_int("👉 请输入操作编号: ");

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
                readonly_dashboard_menu();
                break;
            case 0:
                // 准备退出主循环
                printf("\n正在准备退出系统...\n");
                running = 0; 
                break;
            default:
                printf("\n⚠️ 无效的选项，请重新输入！\n");
                system("pause");
                break;
        }
    }

    // ---------------------------------------------------
    // 第三阶段：系统善后 (保存数据、释放内存)
    // ---------------------------------------------------
    // 这里未来调用 dat-io.c 里的函数，把双向链表数据全部写回 txt
    // save_medicine_list();
    printf("     🧹 正在释放链表内存，清空回收站...\n");
    // free_all_lists(); // 尚未实现

    printf("\n✅ 数据已安全保存，感谢使用！再见！\n");
    system("pause");
    return 0;
}
static void handle_medicine_basic_info_update()
{
    // !!! 注意: 本函数中的所有 get_safe_string 调用都依赖于其允许用户输入空字符串 (直接回车) 来实现 "不修改" 的交互语义。
    // !!! 请勿在不理解此依赖的情况下修改 get_safe_string 的行为。

    // 变量声明区域
    char med_id[MAX_ID_LEN];
    MedicineNode* target = NULL;

    char new_name_buf[MAX_MED_NAME_LEN];
    const char* final_new_name = NULL;

    char new_generic_name_buf[MAX_GENERIC_NAME_LEN];
    const char* final_new_generic_name = NULL;

    char new_alias_buf[MAX_ALIAS_LEN];
    const char* final_new_alias = NULL;

    char new_price_str[MAX_MED_NAME_LEN]; // 重新使用 MAX_MED_NAME_LEN 作为通用输入缓冲区
    double new_price_double = -1.0; // -1.0 表示不修改

    char new_expiry_date_buf[MAX_DATE_LEN];
    const char* final_new_expiry_date = NULL; // NULL 表示不修改

    system("cls");
    printf("======================================================\n");
    printf("             💊 修改药品基础信息\n");
    printf("======================================================\n");

    // 第一步：获取药品ID并查找
    get_safe_string("👉 请输入要修改的药品ID: ", med_id, MAX_ID_LEN);

    // 校验药品ID是否为空或全空格
    if (is_blank_string(med_id))
    {
        printf("提示：药品编号不能为空。\n");
        system("pause");
        return;
    }

    target = find_medicine_by_id(g_medicine_list, med_id);
    if (target == NULL)
    {
        printf("提示：未找到编号为 \"%s\" 的药品，修改失败。\n", med_id);
        system("pause");
        return;
    }

    printf("\n当前药品信息：\n");
    printf("药品编号：%s\n", target->id);
    printf("商品名：%s\n", target->name);
    printf("通用名：%s\n", target->generic_name);
    printf("别名：%s\n", target->alias[0] == '\0' ? "无" : target->alias);
    printf("单价：%.2f\n", target->price);
    printf("效期：%s\n", target->expiry_date);
    printf("------------------------------------------------------\n");

    // 第三步：逐项获取新的药品信息（交互语义不变）

    // 获取新商品名
    while (1)
    {
        get_safe_string("请输入新商品名（直接回车表示不修改）: ", new_name_buf, MAX_MED_NAME_LEN);
        if (strlen(new_name_buf) == 0)
        {
            final_new_name = NULL; // 表示不修改
            break;
        }
        if (is_blank_string(new_name_buf))
        {
            printf("⚠️ 非法输入：商品名不能全为空格，请重新输入。\n");
        }
        else
        {
            final_new_name = new_name_buf;
            break;
        }
    }

    // 获取新通用名
    while (1)
    {
        get_safe_string("请输入新通用名（直接回车表示不修改）: ", new_generic_name_buf, MAX_GENERIC_NAME_LEN);
        if (strlen(new_generic_name_buf) == 0)
        {
            final_new_generic_name = NULL; // 表示不修改
            break;
        }
        if (is_blank_string(new_generic_name_buf))
        {
            printf("⚠️ 非法输入：通用名不能全为空格，请重新输入。\n");
        }
        else
        {
            final_new_generic_name = new_generic_name_buf;
            break;
        }
    }

    // 获取新别名
    while (1)
    {
        get_safe_string("请输入新别名（直接回车表示不修改，输入 # 表示清空别名）: ", new_alias_buf, MAX_ALIAS_LEN);
        if (strlen(new_alias_buf) == 0)
        {
            final_new_alias = NULL; // 表示不修改
            break;
        }
        if (strcmp(new_alias_buf, "#") == 0)
        {
            final_new_alias = ""; // 表示清空别名
            break;
        }
        final_new_alias = new_alias_buf;
        break;
    }

    // 获取新单价
    while (1)
    {
        get_safe_string("请输入新单价（直接回车表示不修改）: ", new_price_str, MAX_MED_NAME_LEN);
        if (strlen(new_price_str) == 0)
        {
            new_price_double = -1.0; // 表示不修改
            break;
        }
        if (sscanf(new_price_str, "%lf", &new_price_double) != 1 || new_price_double <= 0)
        {
            printf("⚠️ 非法输入：单价必须是大于 0 的数字，请重新输入。\n");
            new_price_double = -1.0; // 重置，确保下次循环能继续判断
        }
        else
        {
            break;
        }
    }

    // 获取新效期
    while (1)
    {
        get_safe_string("请输入新效期 (YYYY-MM-DD，直接回车表示不修改): ", new_expiry_date_buf, MAX_DATE_LEN);
        if (strlen(new_expiry_date_buf) == 0)
        {
            final_new_expiry_date = NULL; // 表示不修改
            break;
        }
        if (!is_valid_date_string(new_expiry_date_buf))
        {
            printf("⚠️ 非法输入：效期格式不正确或日期无效，请使用 YYYY-MM-DD 格式，并确保日期合法。\n");
        }
        else
        {
            final_new_expiry_date = new_expiry_date_buf;
            break;
        }
    }

    // 第四步：调用业务层函数进行更新
    update_medicine_basic_info(
        med_id,
        final_new_name,
        final_new_alias,
        final_new_generic_name,
        new_price_double,
        final_new_expiry_date
    );
    system("pause");
}

// 新增的菜单层处理函数
static void handle_medicine_dispense()
{
    char patient_id[MAX_ID_LEN];
    char confirm[10];
    PatientNode* patient = NULL;
    const char* status_text = NULL;
    PrescriptionNode* curr_script = NULL;
    MedicineNode* med = NULL;

    printf("\n================ 为患者发药 ================\n");
    get_safe_string("请输入患者编号: ", patient_id, MAX_ID_LEN);

    if (is_blank_string(patient_id))
    {
        printf("提示：患者编号不能为空。\n");
        system("pause");
        return;
    }

    patient = find_patient_by_id(g_patient_list, patient_id);
    if (patient == NULL)
    {
        printf("提示：未找到该患者。\n");
        system("pause");
        return;
    }

    // 检查患者是否符合发药条件
    if (patient->status != STATUS_PAID)
    {
        printf("提示：当前患者不处于待发药状态。\n");
        system("pause");
        return;
    }

    if (patient->script_head == NULL || patient->script_count <= 0)
    {
        printf("提示：当前患者无待发药处方。\n");
        system("pause");
        return;
    }

    printf("\n患者信息确认：\n");
    printf("患者编号：%s\n", patient->id);
    printf("患者姓名：%s\n", patient->name);
    
    switch (patient->status)
    {
        case STATUS_PENDING: status_text = "待诊"; break;
        case STATUS_DIAGNOSED: status_text = "已看诊待缴费"; break;
        case STATUS_PAID: status_text = "已缴费待取药"; break;
        case STATUS_COMPLETED: status_text = "就诊结束"; break;
        default: status_text = "未知状态"; break;
    }
    printf("当前状态：%s\n", status_text);

    // 显示待发药明细
    printf("\n待发药明细：\n");
    printf("------------------------------------------------------\n");
    curr_script = patient->script_head;
    while (curr_script != NULL)
    {
        med = find_medicine_by_id(g_medicine_list, curr_script->med_id);
        if (med != NULL)
        {
            printf("药品编号：%s\n", curr_script->med_id);
            printf("商品名：%s\n", med->name);
            printf("处方数量：%d\n", curr_script->quantity);
        }
        else
        {
            printf("药品编号：%s\n", curr_script->med_id);
            printf("商品名：[未知]\n");
            printf("处方数量：%d\n", curr_script->quantity);
        }
        printf("------------------------------------------------------\n");
        curr_script = curr_script->next;
    }

    while (1)
    {
        get_safe_string("是否确认发药？(Y/N): ", confirm, 10);
        if (strcmp(confirm, "Y") == 0 || strcmp(confirm, "y") == 0)
        {
            dispense_medicine_for_patient(patient_id);
            system("pause");
            return;
        }
        else if (strcmp(confirm, "N") == 0 || strcmp(confirm, "n") == 0)
        {
            printf("已取消发药操作。\n");
            system("pause");
            return;
        }
        else
        {
            printf("输入无效，请输入 Y 或 N。\n");
        }
    }
}

static void handle_medicine_remove()
{
    char med_id[MAX_ID_LEN];
    char confirm[10];
    MedicineNode* medicine = NULL;

    printf("\n================ 下架药品 ================\n");
    get_safe_string("请输入药品编号: ", med_id, MAX_ID_LEN);

    if (is_blank_string(med_id))
    {
        printf("提示：药品编号不能为空。\n");
        system("pause");
        return;
    }

    medicine = find_medicine_by_id(g_medicine_list, med_id);
    if (medicine == NULL)
    {
        printf("提示：未找到该药品。\n");
        system("pause");
        return;
    }

    printf("\n药品信息确认：\n");
    printf("------------------------------------------------------\n");
    printf("药品编号：%s\n", medicine->id);
    printf("商品名：%s\n", medicine->name);
    printf("通用名：%s\n", medicine->generic_name);
    printf("别名：%s\n", medicine->alias[0] == '\0' ? "无" : medicine->alias);
    printf("单价：%.2f\n", medicine->price);
    printf("库存：%d\n", medicine->stock);
    printf("医保类型：%s\n", 
        medicine->m_type == MEDICARE_NONE ? "非医保" : 
        medicine->m_type == MEDICARE_CLASS_A ? "甲类医保" : "乙类医保");
    printf("效期：%s\n", medicine->expiry_date);
    printf("------------------------------------------------------\n");

    while (1)
    {
        get_safe_string("是否确认下架该药品？(Y/N): ", confirm, 10);
        if (strcmp(confirm, "Y") == 0 || strcmp(confirm, "y") == 0)
        {
            remove_medicine(med_id);
            system("pause");
            return;
        }
        else if (strcmp(confirm, "N") == 0 || strcmp(confirm, "n") == 0)
        {
            printf("已取消下架操作。\n");
            system("pause");
            return;
        }
        else
        {
            printf("输入无效，请输入 Y 或 N。\n");
        }
    }
}

static void handle_account_register()
{
    char username[MAX_ID_LEN];
    char password[MAX_ID_LEN];
    char real_name[MAX_NAME_LEN];
    int role_choice;
    RoleType role;

    printf("\n================ 新增员工账号 ===============\n");
    get_safe_string("请输入登录账号: ", username, MAX_ID_LEN);
    if (is_blank_string(username))
    {
        printf("⚠️ 账号不能为空或全为空格，操作取消！\n");
        system("pause");
        return;
    }

    get_safe_string("请输入密码: ", password, MAX_ID_LEN);
    if (is_blank_string(password))
    {
        printf("⚠️ 密码不能为空或全为空格，操作取消！\n");
        system("pause");
        return;
    }

    get_safe_string("请输入真实姓名: ", real_name, MAX_NAME_LEN);
    if (is_blank_string(real_name))
    {
        printf("⚠️ 真实姓名不能为空或全为空格，操作取消！\n");
        system("pause");
        return;
    }

    while (1)
    {
        role_choice = get_safe_int("请输入角色编号 (0: 管理员, 1: 护士, 2: 医生, 3: 药师, -1: 取消): ");
        if (role_choice == -1)
        {
            printf("已取消新增员工操作\n");
            system("pause");
            return;
        }
        else if (role_choice == 0)
        {
            role = ROLE_ADMIN;
            break;
        }
        else if (role_choice == 1)
        {
            role = ROLE_NURSE;
            break;
        }
        else if (role_choice == 2)
        {
            role = ROLE_DOCTOR;
            break;
        }
        else if (role_choice == 3)
        {
            role = ROLE_PHARMACIST;
            break;
        }
        else
        {
            printf("⚠️ 角色输入无效，请重新输入！\n");
        }
    }

    register_account(username, password, real_name, role);
    system("pause");
}

static void handle_account_update()
{
    char username[MAX_ID_LEN];
    char new_real_name[MAX_NAME_LEN];
    char new_password[MAX_ID_LEN];
    int role_choice;
    RoleType new_role = 0;

    printf("\n================ 修改员工资料 ===============\n");
    get_safe_string("请输入要修改的员工账号: ", username, MAX_ID_LEN);
    if (is_blank_string(username))
    {
        printf("⚠️ 账号不能为空或全为空格，操作取消！\n");
        system("pause");
        return;
    }

    get_safe_string("请输入新真实姓名 (直接回车表示不修改): ", new_real_name, MAX_NAME_LEN);
    get_safe_string("请输入新密码 (直接回车表示不修改): ", new_password, MAX_ID_LEN);

    role_choice = get_safe_int("请输入新角色 (0: 管理员, 1: 护士, 2: 医生, 3: 药师, -1: 不修改): ");
    if (role_choice == -1)
    {
        new_role = 0;
    }
    else if (role_choice == 0)
    {
        new_role = ROLE_ADMIN;
    }
    else if (role_choice == 1)
    {
        new_role = ROLE_NURSE;
    }
    else if (role_choice == 2)
    {
        new_role = ROLE_DOCTOR;
    }
    else if (role_choice == 3)
    {
        new_role = ROLE_PHARMACIST;
    }
    else
    {
        printf("⚠️ 角色输入无效，将不修改角色。\n");
        new_role = 0;
    }

    update_account_basic_info(
        username,
        is_blank_string(new_real_name) ? NULL : new_real_name,
        is_blank_string(new_password) ? NULL : new_password,
        new_role
    );
    system("pause");
}

static void handle_scheduling_maintenance()
{
    int running = 1;
    int choice;

    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               👨⚕️ 动态排班维护\n");
        printf("======================================================\n");
        printf("  [1] 查看医生值班状态\n");
        printf("  [2] 修改医生值班状态\n");
        printf("  [3] 查看护士值班状态\n");
        printf("  [4] 修改护士值班状态\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");

        choice = get_safe_int("👉 请输入操作编号: ");

        switch (choice)
        {
            case 1:
                show_all_doctors_with_duty_status();
                system("pause");
                break;
            case 2:
            {
                char doctor_id[MAX_ID_LEN];
                int new_status;

                show_all_doctors_with_duty_status();
                get_safe_string("请输入要修改值班状态的医生工号 (输入 0 取消): ", doctor_id, MAX_ID_LEN);

                if (strcmp(doctor_id, "0") == 0)
                {
                    break;
                }

                new_status = get_safe_int("请输入新的值班状态 (1-值班中, 0-未值班): ");

                if (update_doctor_duty_status(doctor_id, new_status))
                {
                    printf("\n✅ 医生值班状态修改成功！\n");
                }
                system("pause");
                break;
            }
            case 3:
                show_all_nurses_with_duty_status();
                system("pause");
                break;
            case 4:
            {
                char nurse_username[MAX_ID_LEN];
                int new_status;

                show_all_nurses_with_duty_status();
                get_safe_string("请输入要修改值班状态的护士工号 (输入 0 取消): ", nurse_username, MAX_ID_LEN);

                if (strcmp(nurse_username, "0") == 0)
                {
                    break;
                }

                new_status = get_safe_int("请输入新的值班状态 (1-值班中, 0-未值班): ");

                if (update_nurse_duty_status(nurse_username, new_status))
                {
                    printf("\n✅ 护士值班状态修改成功！\n");
                }
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

static void handle_admin_dashboard()
{
    show_admin_dashboard();
    system("pause");
}

static void admin_statistics_menu()
{
    int running = 1;
    int choice;

    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               🔧 统计与预警菜单\n");
        printf("======================================================\n");
        printf("  [1] 查看统计面板\n");
        printf("  [2] 查看资源预警\n");
        printf("  [3] 查看负载监控\n");
        printf("  [4] 传染病异常提醒\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");

        choice = get_safe_int("👉 请输入操作编号: ");

        switch (choice)
        {
            case 1:
                show_admin_dashboard();
                system("pause");
                break;
            case 2:
                show_resource_warnings();
                system("pause");
                break;
            case 3:
                show_load_monitoring();
                system("pause");
                break;
            case 4:
                show_infectious_disease_alerts();
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

// 显示负载监控摘要（只读版）
static void show_load_monitoring_summary()
{
    int doctor_count = 0;
    int pending_count = 0;
    int completed_count = 0;
    int dept_count = 0;
    int i;
    int found;

    DoctorNode* doctor_curr = NULL;
    PatientNode* patient_curr = NULL;
    
    // 用于存储科室信息的临时数组
    char departments[50][MAX_NAME_LEN];
    int dept_queue_lengths[50];

    // 初始化科室数组
    for (i = 0; i < 50; i++)
    {
        departments[i][0] = '\0';
        dept_queue_lengths[i] = 0;
    }

    // 统计医生总数和各科室排队人数
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
                    dept_queue_lengths[i] += doctor_curr->queue_length;
                    found = 1;
                    break;
                }
            }
            if (!found && dept_count < 50)
            {
                strcpy(departments[dept_count], doctor_curr->department);
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
                case STATUS_COMPLETED:
                    completed_count++;
                    break;
                default:
                    break;
            }
            patient_curr = patient_curr->next;
        }
    }

    // 显示负载摘要
    printf("\n======================================================\n");
    printf("                  负载监控摘要\n");
    printf("======================================================\n");
    printf("【总体负载】\n");
    printf("------------------------------------------------------\n");
    printf("医生总数：%d\n", doctor_count);
    printf("总待诊患者数：%d\n", pending_count);
    printf("总已完成患者数：%d\n", completed_count);
    printf("======================================================\n");

    // 显示各科室负载
    printf("【科室负载】\n");
    printf("------------------------------------------------------\n");
    printf("科室名称            总排队人数\n");
    printf("------------------------------------------------------\n");
    if (dept_count > 0)
    {
        for (i = 0; i < dept_count; i++)
        {
            printf("%-18s %-16d\n", 
                departments[i], 
                dept_queue_lengths[i]);
        }
    }
    else
    {
        printf("当前无科室数据\n");
    }
    printf("------------------------------------------------------\n");

    printf("======================================================\n");
    printf("提示：此面板数据实时更新，仅供参考。\n");
    printf("======================================================\n");
}

// 只读大屏菜单
static void readonly_dashboard_menu()
{
    int running = 1;
    int choice;

    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               📊 医疗大屏\n");
        printf("======================================================\n");
        printf("  [1] 查看状态统计\n");
        printf("  [2] 查看负载摘要\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");

        choice = get_safe_int("👉 请输入操作编号: ");

        switch (choice)
        {
            case 1:
                show_public_status_statistics();
                system("pause");
                break;
            case 2:
                show_load_monitoring_summary();
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

static void recycle_bin_menu()
{
    int running = 1;
    int choice;

    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("                 🗑️  回收站管理\n");
        printf("======================================================\n");
        printf("  [1] 查看药品回收站\n");
        printf("  [2] 恢复药品\n");
        printf("  [0] 返回上一级\n");
        printf("------------------------------------------------------\n");

        choice = get_safe_int("👉 请输入操作编号: ");

        switch (choice)
        {
            case 1:
                printf("\n当前回收站功能尚未完成，后续接入。\n");
                system("pause");
                break;
            case 2:
                printf("\n当前回收站功能尚未完成，后续接入。\n");
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

// 病房管理菜单
static void inpatient_menu()
{
    int running = 1;
    int choice;
    char patient_id[MAX_ID_LEN];
    char bed_id[MAX_ID_LEN];
    char old_bed_id[MAX_ID_LEN];
    char new_bed_id[MAX_ID_LEN];
    int estimated_days;
    double deposit;
    double amount;

    while (running)
    {
        system("cls");
        printf("\n======================================================\n");
        printf("               🏥 病房管理\n");
        printf("======================================================\n");
        printf("  [1] 查看全部床位信息\n");
        printf("  [2] 查看空闲床位\n");
        printf("  [3] 查看床位占用状态\n");
        printf("  [4] 住院登记\n");
        printf("  [5] 分配床位\n");
        printf("  [6] 查询住院患者信息\n");
        printf("  [7] 查询患者住院记录\n");
        printf("  [8] 押金充值\n");
        printf("  [9] 日结计费\n");
        printf("  [10] 转床\n");
        printf("  [11] 办理出院（自动释放床位）\n");
        printf("  [12] 床位/押金预警信息\n");
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
                    WardNode* ward_curr = NULL;
                    printf("\n================ 床位占用状态 ===============-\n");
                    printf("------------------------------------------------------\n");
                    printf("病房编号       床位编号       类型         当前患者编号\n");
                    printf("------------------------------------------------------\n");

                    if (g_ward_list != NULL && g_ward_list->next != NULL)
                    {
                        ward_curr = g_ward_list->next;
                        int has_occupied_beds = 0;
                        while (ward_curr != NULL)
                        {
                            if (ward_curr->is_occupied)
                            {
                                has_occupied_beds = 1;
                                printf("%-13s %-13s %-12s %s\n",
                                    ward_curr->room_id,
                                    ward_curr->bed_id,
                                    ward_curr->ward_type == WARD_TYPE_ICU ? "ICU" : "普通病房",
                                    ward_curr->patient_id);
                                printf("------------------------------------------------------\n");
                            }
                            ward_curr = ward_curr->next;
                        }
                        if (!has_occupied_beds)
                        {
                            printf("当前暂无已占用床位\n");
                            printf("------------------------------------------------------\n");
                        }
                    }
                    else
                    {
                        printf("当前无床位数据\n");
                        printf("------------------------------------------------------\n");
                    }
                    system("pause");
                }
                break;
            case 4:
                handle_inpatient_register();
                break;
            case 5:
                handle_bed_assign();
                break;
            case 6:
                show_hospitalized_patients();
                system("pause");
                break;
            case 7:
                handle_inpatient_record_query();
                break;
            case 8:
                handle_deposit_recharge();
                break;

            case 9:
                handle_daily_settlement();
                break;
            case 10:
                handle_transfer_bed();
                break;
            case 11:
                handle_discharge();
                break;
            case 12:
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

