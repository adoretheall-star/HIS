// ==========================================
// 文件名: appointment.h
// 作用: 预约管理相关业务层声明
// ==========================================
#ifndef APPOINTMENT_H
#define APPOINTMENT_H
#include "global.h"
// 1. 预约登记
AppointmentNode* register_appointment(
    const char* patient_id,
    const char* appointment_date,
    const char* appointment_slot,
    const char* appoint_doctor,
    const char* appoint_dept
);
// 1.5 提前检查重复预约（日期+时段确定后立即调用）
int check_duplicate_appointment_early(
    const char* patient_id,
    const char* appointment_date,
    const char* appointment_slot,
    char* existing_appointment_id,
    size_t id_size
);
// 2. 按患者编号查询预约
void query_appointments_by_patient_id(const char* patient_id);
// 3. 按身份证号查询预约
void query_appointments_by_id_card(const char* id_card);
// 4. 按预约编号查询预约及患者信息
void query_appointment_and_patient(const char* appointment_id);
// 5. 预约取消
int cancel_appointment(const char* appointment_id);
// 5. 预约签到转挂号
int check_in_appointment(const char* appointment_id);

// 6. 现场挂号直接入队
int queue_walk_in_registration(const char* appointment_id);

// 7. 根据科室显示医生列表
int display_doctors_by_dept(const char* dept_name);
const char* get_appointment_display_status(const AppointmentNode* appointment);

// 7.5 科室是否存在（不管是否值班）
int department_exists(const char* dept_name);

// 7.6 科室是否有值班医生
int has_on_duty_doctor_in_department(const char* dept_name);

// 7.7 判断预约是否可取消
int is_appointment_cancelable(const AppointmentNode* appt);

// 7.8 显示患者可取消的预约列表，返回可取消预约的数量
int show_cancelable_appointments_for_patient(const char* patient_id);

// 7.9 在指定科室中查找最佳医生（自动分配用）
// 规则1：优先选择值班中的医生
// 规则2：如果有多名值班医生，选择 queue_length 最小的
// 规则3：如果 queue_length 也相同，选择编号较小的医生
DoctorNode* find_best_doctor_for_department(const char* dept);

// 8. 登记预约爽约
int mark_appointment_missed(const char* appointment_id);
#endif
