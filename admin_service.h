#ifndef ADMIN_SERVICE_H
#define ADMIN_SERVICE_H

#include "global.h"

// 查看所有员工账号
void show_all_accounts(void);

// 注册新员工账号
int register_account(const char* username, const char* password, const char* real_name, const char* gender, RoleType role);

// 更新员工资料
int update_account_basic_info(
    const char* username,
    const char* new_real_name,
    const char* new_password,
    RoleType new_role
);

// 查看所有医生及其值班状态
void show_all_doctors_with_duty_status(void);

// 更新医生值班状态
int update_doctor_duty_status(const char* doctor_id, int new_status);

// 显示管理员统计面板
void show_admin_dashboard(void);

// 显示资源预警查看
void show_resource_warnings(void);

// 显示系统预警查看（整合所有7种预警类型）
void show_system_alerts(void);

// 预警管理相关函数
void admin_alert_menu(void);
void show_event_alerts(void);
void show_medicine_alerts(void);
void show_inpatient_alerts(void);
void show_bed_resource_alerts(void);

// 显示负载监控查看
void show_load_monitoring(void);

// 显示公共状态统计摘要
void show_public_status_statistics(void);

// 显示所有护士值班状态
void show_all_nurses_with_duty_status(void);

// 更新护士值班状态
int update_nurse_duty_status(const char* username, int new_status);

// 显示传染病异常提醒
void show_infectious_disease_alerts(void);

// 日志相关函数
void init_log_list(void);
void add_log(const char* operation, const char* target, const char* description);
void show_logs(void);

// 投诉管理相关函数
void admin_complaint_menu(void);
void show_all_complaints(void);
void handle_complaint_response(void);
void query_patient_complaints_by_id(void);
void query_complaint_by_id(void);

// 评价管理相关函数
void admin_evaluation_menu(void);
void show_all_evaluations(void);
void show_evaluation_statistics(void);

// 药品管理相关处理函数
void handle_medicine_register(void);
void handle_medicine_basic_info_update(void);
void handle_medicine_stock_update(void);
void handle_expiring_medicine_check(void);
void handle_medicine_remove(void);

// 检查项目管理
void show_all_check_items(void);
void search_check_item_by_keyword(const char* keyword);
void handle_check_item_register(void);
void handle_check_item_update(void);
void handle_check_item_remove(void);

#endif // ADMIN_SERVICE_H