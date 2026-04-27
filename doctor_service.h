// ==========================================
// 文件名: doctor_service.h
// 作用: 医生接诊相关业务层声明
// ==========================================

#ifndef DOCTOR_SERVICE_H
#define DOCTOR_SERVICE_H

#include "global.h"

// 辅助函数：获取患者姓名
const char* get_patient_name_by_id(const char* patient_id);

// 1. 查看指定医生的待诊患者
void show_waiting_patients_by_doctor(const char* doctor_id);

// 获取指定医生的待诊患者列表（返回链表）
PatientPtrNode* get_waiting_patients_by_doctor(const char* doctor_id);

// 获取医生的已处理患者列表（用于显示）
PatientPtrNode* get_processed_patients_by_doctor(const char* doctor_id);

// 辅助函数：释放患者指针链表
void free_patient_ptr_list(PatientPtrNode* head);

// 辅助函数：获取患者指针链表长度
int get_patient_ptr_list_count(PatientPtrNode* head);

// 辅助函数：从患者指针链表中获取第 N 个患者
PatientNode* get_nth_patient_from_ptr_list(PatientPtrNode* head, int index);

// 辅助函数：在患者指针链表中查找患者位置
int find_patient_position_in_ptr_list(PatientPtrNode* head, PatientNode* patient);

// 辅助函数：从患者指针链表中获取指定位置后第 N 个患者
PatientNode* get_patient_after_position_in_ptr_list(PatientPtrNode* head, int position, int offset);

// 2. 医生查看患者接诊前概览
void doctor_view_patient_overview(const char* doctor_id, const char* patient_id);

// 3. 医生接诊并做最小诊疗决策
int doctor_consult_patient(
    const char* doctor_id,
    const char* patient_id,
    int decision,
    const char* diagnosis_text,
    const char* treatment_advice
);

// 4. 医生查看已处理患者
void doctor_view_processed_patients(const char* doctor_id);

// 5. 显示医生信息
void show_doctor_info(DoctorNode* doctor);

// 5. 医生查看已处理患者详情
void doctor_view_processed_patient_detail(const char* doctor_id, const char* patient_id);

// 6. 查看历史接诊记录
void doctor_view_consult_history(const char* doctor_id, const char* patient_id);

// 7. 检查科室医生功能
void show_waiting_checks_by_dept(const char* doctor_id);
int get_waiting_checks_by_dept(const char* doctor_id, CheckRecordNode** check_list);
void show_check_record_detail(CheckRecordNode* record);
int doctor_update_check_result(const char* doctor_id, const char* record_id, const char* result);

#endif