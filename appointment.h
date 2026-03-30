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
// 2. 按患者编号查询预约
void query_appointments_by_patient_id(const char* patient_id);
// 3. 按身份证号查询预约
void query_appointments_by_id_card(const char* id_card);
// 4. 预约取消
int cancel_appointment(const char* appointment_id);
// 5. 预约签到转挂号
int check_in_appointment(const char* appointment_id);
#endif